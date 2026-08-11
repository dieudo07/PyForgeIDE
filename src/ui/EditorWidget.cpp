#include "EditorWidget.hpp"
#include "GutterWidget.hpp"
#include <QPainter>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTextBlock>
#include <QFileInfo>
#include <QFile>
#include <QListWidget>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <string>

namespace PyForge {

EditorWidget::EditorWidget(QWidget* parent)
    : QPlainTextEdit(parent)
{
    gutter_      = new GutterWidget(this);
    highlighter_ = std::make_unique<SyntaxHighlighter>(document());

    saveTimer_.setSingleShot(true);
    saveTimer_.setInterval(3000);

    setLineWrapMode(QPlainTextEdit::NoWrap);
    setUndoRedoEnabled(true);

    applyTheme(Theme::instance().colors);

    connect(this, &QPlainTextEdit::textChanged,
            this, &EditorWidget::onTextChanged);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &EditorWidget::onCursorMoved);
    connect(document(), &QTextDocument::blockCountChanged,
            this, &EditorWidget::updateGutterWidth);
    connect(&saveTimer_, &QTimer::timeout, [this] {
        if (!filePath_.isEmpty()) saveFile();
    });

    updateGutterWidth();
}

EditorWidget::~EditorWidget() = default;

bool EditorWidget::loadFile(const QString& path) {
    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text)) return false;
    setPlainText(QString::fromUtf8(f.readAll()));
    filePath_ = path;
    modified_ = false;
    document()->setModified(false);
    emit fileModified(false);
    return true;
}

bool EditorWidget::saveFile(const QString& path) {
    const QString p = path.isEmpty() ? filePath_ : path;
    if (p.isEmpty()) return false;
    QFile f(p);
    if (!f.open(QFile::WriteOnly | QFile::Text)) return false;
    f.write(toPlainText().toUtf8());
    filePath_ = p;
    modified_ = false;
    document()->setModified(false);
    emit fileModified(false);
    emit textSaved(p);
    return true;
}

bool EditorWidget::isModified() const { return modified_; }

void EditorWidget::applyTheme(const ColorScheme& c) {
    colors_ = c;
    QPalette p = palette();
    p.setColor(QPalette::Base,            c.bg_editor);
    p.setColor(QPalette::Text,            c.text_primary);
    p.setColor(QPalette::Highlight,       c.selection);
    p.setColor(QPalette::HighlightedText, c.text_primary);
    setPalette(p);
    setCursorWidth(2);
    if (highlighter_) highlighter_->setTheme(c);
    update();
}

void EditorWidget::applySettings(const EditorSettings& s) {
    settings_ = s;
    setLineWrapMode(s.wordWrap
        ? QPlainTextEdit::WidgetWidth
        : QPlainTextEdit::NoWrap);
    update();
}

void EditorWidget::setDiagnostics(const QList<Diagnostic>& d) {
    diagnostics_ = d; update();
}

void EditorWidget::gotoLine(int line, int col) {
    QTextBlock b = document()->findBlockByLineNumber(line);
    if (!b.isValid()) return;
    QTextCursor c(b);
    if (col > 0)
        c.movePosition(QTextCursor::Right,
                       QTextCursor::MoveAnchor, col);
    setTextCursor(c);
    centerCursor();
}

void EditorWidget::toggleComment() {
    QTextCursor cur = textCursor();
    cur.beginEditBlock();
    int s = document()->findBlock(
                cur.selectionStart()).blockNumber();
    int e = document()->findBlock(
                cur.selectionEnd()).blockNumber();

    bool allCommented = true;
    for (int i = s; i <= e; ++i) {
        if (!document()->findBlockByNumber(i)
                .text().trimmed().startsWith('#')) {
            allCommented = false; break;
        }
    }
    for (int i = s; i <= e; ++i) {
        QTextBlock blk = document()->findBlockByNumber(i);
        QTextCursor c(blk);
        if (allCommented) {
            int h = blk.text().indexOf('#');
            if (h >= 0) {
                c.setPosition(blk.position() + h);
                c.deleteChar();
                if (!c.atEnd() &&
                    document()->characterAt(c.position()) == ' ')
                    c.deleteChar();
            }
        } else {
            int in = 0;
            const QString t = blk.text();
            while (in < t.length() && t[in] == ' ') ++in;
            c.setPosition(blk.position() + in);
            c.insertText("# ");
        }
    }
    cur.endEditBlock();
}

void EditorWidget::duplicateLine() {
    QTextCursor c = textCursor();
    c.beginEditBlock();
    c.movePosition(QTextCursor::StartOfBlock);
    c.movePosition(QTextCursor::EndOfBlock,
                   QTextCursor::KeepAnchor);
    const QString t = c.selectedText();
    c.movePosition(QTextCursor::EndOfBlock);
    c.insertText("\n" + t);
    c.endEditBlock();
}

void EditorWidget::moveLinesUp() {
    QTextCursor c = textCursor();
    c.beginEditBlock();
    QTextBlock b = c.block();
    if (b.previous().isValid()) {
        const QString cur  = b.text();
        const QString prev = b.previous().text();
        QTextCursor c1(b.previous());
        c1.movePosition(QTextCursor::EndOfBlock,
                        QTextCursor::KeepAnchor);
        c1.insertText(cur);
        QTextCursor c2(b);
        c2.movePosition(QTextCursor::EndOfBlock,
                        QTextCursor::KeepAnchor);
        c2.insertText(prev);
        QTextCursor nc = textCursor();
        nc.movePosition(QTextCursor::PreviousBlock);
        setTextCursor(nc);
    }
    c.endEditBlock();
}

void EditorWidget::moveLinesDown() {
    QTextCursor c = textCursor();
    c.beginEditBlock();
    QTextBlock b = c.block();
    if (b.next().isValid()) {
        const QString cur  = b.text();
        const QString next = b.next().text();
        QTextCursor c2(b.next());
        c2.movePosition(QTextCursor::EndOfBlock,
                        QTextCursor::KeepAnchor);
        c2.insertText(cur);
        QTextCursor c1(b);
        c1.movePosition(QTextCursor::EndOfBlock,
                        QTextCursor::KeepAnchor);
        c1.insertText(next);
        QTextCursor nc = textCursor();
        nc.movePosition(QTextCursor::NextBlock);
        setTextCursor(nc);
    }
    c.endEditBlock();
}

void EditorWidget::deleteCurrentLine() {
    QTextCursor c = textCursor();
    c.beginEditBlock();
    c.movePosition(QTextCursor::StartOfBlock);
    c.movePosition(QTextCursor::EndOfBlock,
                   QTextCursor::KeepAnchor);
    c.removeSelectedText();
    if (!c.atEnd()) c.deleteChar();
    c.endEditBlock();
    setTextCursor(c);
}

void EditorWidget::indentSelection() {
    QTextCursor c = textCursor();
    c.beginEditBlock();
    int s = document()->findBlock(
                c.selectionStart()).blockNumber();
    int e = document()->findBlock(
                c.selectionEnd()).blockNumber();
    for (int i = s; i <= e; ++i) {
        QTextCursor q(document()->findBlockByNumber(i));
        q.insertText(indentStr());
    }
    c.endEditBlock();
}

void EditorWidget::dedentSelection() {
    QTextCursor c = textCursor();
    c.beginEditBlock();
    int s = document()->findBlock(
                c.selectionStart()).blockNumber();
    int e = document()->findBlock(
                c.selectionEnd()).blockNumber();
    for (int i = s; i <= e; ++i) {
        QTextBlock b = document()->findBlockByNumber(i);
        QTextCursor q(b);
        const QString t = b.text();
        int rm = 0;
        for (int j = 0;
             j < std::min(settings_.tabSize, (int)t.length());
             ++j) {
            if (t[j] == ' ') ++rm;
            else if (t[j] == '\t') { ++rm; break; }
            else break;
        }
        for (int j = 0; j < rm; ++j) q.deleteChar();
    }
    c.endEditBlock();
}

bool EditorWidget::findNext(const QString& text,
                             Qt::CaseSensitivity cs) {
    QTextDocument::FindFlags f;
    if (cs == Qt::CaseSensitive)
        f |= QTextDocument::FindCaseSensitively;
    if (!find(text, f)) {
        moveCursor(QTextCursor::Start);
        find(text, f);
    }
    return true;
}

int EditorWidget::replaceAll(const QString& from,
                              const QString& to) {
    QTextCursor c = textCursor();
    c.beginEditBlock();
    moveCursor(QTextCursor::Start);
    int n = 0;
    while (find(from)) { textCursor().insertText(to); ++n; }
    c.endEditBlock();
    return n;
}

QPair<int,int> EditorWidget::currentPosition() const {
    const QTextCursor c = textCursor();
    return {c.blockNumber() + 1, c.positionInBlock() + 1};
}

int EditorWidget::gutterWidth() const {
    const int digits = std::max(1,
        (int)std::to_string(document()->blockCount()).length());
    return fontMetrics().horizontalAdvance('9') *
           (digits + 2) + 20;
}

QString EditorWidget::wordUnderCursor() const {
    QTextCursor c = textCursor();
    c.select(QTextCursor::WordUnderCursor);
    return c.selectedText();
}

QString EditorWidget::indentStr() const {
    return settings_.useSpaces
        ? QString(settings_.tabSize, ' ') : "\t";
}

// ─── Touches ────────────────────────────────────────────────────

void EditorWidget::keyPressEvent(QKeyEvent* e) {
    // Si le popup d'auto-complétion est visible
    if (completerPopup_ && completerPopup_->isVisible()) {
        if (e->key() == Qt::Key_Escape) {
            completerPopup_->hide();
            return;
        }
        if (e->key() == Qt::Key_Up) {
            int row = completerPopup_->currentRow();
            completerPopup_->setCurrentRow(qMax(0, row - 1));
            return;
        }
        if (e->key() == Qt::Key_Down) {
            int row = completerPopup_->currentRow();
            completerPopup_->setCurrentRow(
                qMin(completerPopup_->count() - 1, row + 1));
            return;
        }
        if (e->key() == Qt::Key_Return ||
            e->key() == Qt::Key_Enter ||
            e->key() == Qt::Key_Tab) {
            if (completerPopup_->currentItem()) {
                emit completerPopup_->itemActivated(
                    completerPopup_->currentItem());
                return;
            }
        }
    }

    // Ctrl+Espace = déclencher auto-complétion
    if (e->key() == Qt::Key_Space &&
        e->modifiers() == Qt::ControlModifier) {
        showAutoComplete();
        return;
    }

    if (handleAutoPair(e))       return;
    if (handleSmartBackspace(e)) return;
    if (e->key() == Qt::Key_Tab) {
        if (handleTab(e)) return;
    }
    if (e->key() == Qt::Key_Return ||
        e->key() == Qt::Key_Enter) {
        if (handleEnter(e)) return;
    }

    const Qt::KeyboardModifiers m = e->modifiers();
    if (m == Qt::ControlModifier) {
        if (e->key() == Qt::Key_D)     { duplicateLine();     return; }
        if (e->key() == Qt::Key_Slash) { toggleComment();     return; }
        if (e->key() == Qt::Key_K)     { deleteCurrentLine(); return; }
    }
    if (m == (Qt::ControlModifier | Qt::ShiftModifier)) {
        if (e->key() == Qt::Key_Up)   { moveLinesUp();   return; }
        if (e->key() == Qt::Key_Down) { moveLinesDown(); return; }
    }
    if (m == Qt::AltModifier) {
        if (e->key() == Qt::Key_Up)   { moveLinesUp();   return; }
        if (e->key() == Qt::Key_Down) { moveLinesDown(); return; }
    }

    QPlainTextEdit::keyPressEvent(e);

    // Mettre à jour l'auto-complétion si popup ouvert
    if (completerPopup_ && completerPopup_->isVisible()) {
        QString word = wordUnderCursor();
        if (word.isEmpty() || word.length() < 2) {
            completerPopup_->hide();
        } else {
            showAutoComplete();
        }
        return;
    }

    // Auto-complétion automatique après 2+ caractères
    if (!e->text().isEmpty() && e->text()[0].isLetter()) {
        QString word = wordUnderCursor();
        if (word.length() >= 2) {
            showAutoComplete();
        }
    }
}
void EditorWidget::paintIndentGuides(QPainter& p) {
    p.setPen(QPen(colors_.indent_guide, 1));
    const QFontMetrics fm(font());
    const int cw = fm.horizontalAdvance(' ');
    const int lh = fm.lineSpacing();

    QTextBlock blk = firstVisibleBlock();
    int top = static_cast<int>(
        blockBoundingGeometry(blk)
        .translated(contentOffset()).top());

    while (blk.isValid() && top <= viewport()->height()) {
        const QString t = blk.text();
        int level = 0;
        for (const QChar& c : t) {
            if (c==' ') ++level; else break;
        }
        for (int i = 1; i <= level/settings_.tabSize; ++i) {
            const int x = gutterWidth() + i*settings_.tabSize*cw;
            p.drawLine(x, top, x, top+lh-1);
        }
        top += static_cast<int>(blockBoundingRect(blk).height());
        blk  = blk.next();
    }
}

void EditorWidget::paintDiagnostics(QPainter& p) {
    const QFontMetrics fm(font());
    for (const auto& d : diagnostics_) {
        QTextBlock blk =
            document()->findBlockByLineNumber(d.line);
        if (!blk.isValid()) continue;
        const QRectF bg =
            blockBoundingGeometry(blk).translated(contentOffset());
        const int y  = static_cast<int>(bg.bottom()) - 2;
        const QString lt = blk.text();
        const int sx =
            gutterWidth() + fm.horizontalAdvance(lt.left(d.col));
        const int ex = d.endCol > 0
            ? gutterWidth()+fm.horizontalAdvance(lt.left(d.endCol))
            : gutterWidth()+fm.horizontalAdvance(lt);
        QColor c;
        switch (d.severity) {
        case 1:  c=colors_.diag_error;   break;
        case 2:  c=colors_.diag_warning; break;
        case 3:  c=colors_.diag_info;    break;
        default: c=colors_.diag_hint;    break;
        }
        p.setPen(QPen(c, 1.5));
        int x=sx; bool up=true;
        while (x<ex) {
            p.drawLine(x,y+(up?0:2),x+3,y+(up?2:0));
            x+=3; up=!up;
        }
    }
}

// ─── Slots ──────────────────────────────────────────────────────

void EditorWidget::onTextChanged() {
    if (!document()->isModified()) return;
    if (!modified_) { modified_=true; emit fileModified(true); }
    saveTimer_.start();
}

void EditorWidget::onCursorMoved() {
    const QTextCursor c = textCursor();
    const int sel = c.hasSelection()
        ? std::abs(c.selectionEnd()-c.selectionStart()) : 0;
    emit cursorPositionInfo(
        c.blockNumber()+1, c.positionInBlock()+1, sel);
    if (settings_.bracketMatch) highlightBrackets();
}

void EditorWidget::updateGutterWidth() {
    const int digits = std::max(1,
        (int)std::to_string(document()->blockCount()).length());
    const int w =
        fontMetrics().horizontalAdvance('9')*(digits+2)+20;
    setViewportMargins(w, 0, 0, 0);
    if (gutter_) gutter_->setFixedWidth(w);
}

void EditorWidget::resizeEvent(QResizeEvent* e) {
    QPlainTextEdit::resizeEvent(e);
    const QRect cr = contentsRect();
    if (gutter_)
        gutter_->setGeometry(cr.left(), cr.top(),
                             gutterWidth(), cr.height());
}

void EditorWidget::highlightBrackets() {
    QList<QTextEdit::ExtraSelection> sels;
    const QTextCursor c = textCursor();
    const int pos = c.position();
    const QString text = toPlainText();
    if (pos<0||pos>=text.length()) {
        setExtraSelections(sels); return;
    }
    static const QString op="([{", cl=")]}";
    const QChar ch=text[pos];
    int idx=op.indexOf(ch), matchPos=-1;
    if (idx>=0) {
        int depth=1, i=pos+1;
        while (i<text.length()&&depth>0) {
            if (text[i]==op[idx])      ++depth;
            else if (text[i]==cl[idx]) --depth;
            ++i;
        }
        if (depth==0) matchPos=i-1;
    } else {
        idx=cl.indexOf(ch);
        if (idx>=0) {
            int depth=1, i=pos-1;
            while (i>=0&&depth>0) {
                if (text[i]==cl[idx])      ++depth;
                else if (text[i]==op[idx]) --depth;
                --i;
            }
            if (depth==0) matchPos=i+1;
        }
    }
    if (matchPos>=0) {
        auto mkSel=[&](int p){
            QTextEdit::ExtraSelection s;
            QTextCharFormat f;
            f.setBackground(colors_.match_bracket.lighter(150));
            f.setForeground(colors_.match_bracket);
            f.setFontWeight(QFont::Bold);
            s.format=f;
            QTextCursor tc=textCursor();
            tc.setPosition(p);
            tc.movePosition(QTextCursor::NextCharacter,
                            QTextCursor::KeepAnchor);
            s.cursor=tc; return s;
        };
        sels<<mkSel(pos)<<mkSel(matchPos);
    }
    setExtraSelections(sels);
}
// ═══════════════════════════════════════════════════════════
// FONCTIONS MANQUANTES - AJOUTÉES POUR LA COMPILATION
// ═══════════════════════════════════════════════════════════

void EditorWidget::paintEvent(QPaintEvent* e) {
    QPainter p(viewport());
    if (settings_.highlightLine)    paintCurrentLine(p);
    if (settings_.showIndentGuides) paintIndentGuides(p);
    QPlainTextEdit::paintEvent(e);
    paintDiagnostics(p);
}

bool EditorWidget::handleAutoPair(QKeyEvent* e) {
    const QString t = e->text();
    if (t.isEmpty()) return false;

    static const QHash<QString, QString> pairs = {
        {"(",")"}, {"[","]"}, {"{","}"},
        {"\"","\""}, {"'","'"}
    };

    if (!pairs.contains(t)) return false;
    if ((t == "\"" || t == "'") && !settings_.autoCloseQuotes)
        return false;
    if (!settings_.autoCloseBraces && t != "\"" && t != "'")
        return false;

    QTextCursor c = textCursor();
    const int pos = c.position();
    const QString doc = toPlainText();

    if (t == pairs[t] && pos < doc.length() && doc[pos] == t[0]) {
        c.movePosition(QTextCursor::NextCharacter);
        setTextCursor(c);
        return true;
    }

    c.insertText(t + pairs[t]);
    c.movePosition(QTextCursor::PreviousCharacter);
    setTextCursor(c);
    return true;
}

bool EditorWidget::handleSmartBackspace(QKeyEvent* e) {
    if (e->key() != Qt::Key_Backspace) return false;
    if (!settings_.autoCloseBraces)    return false;

    QTextCursor c = textCursor();
    if (c.hasSelection()) return false;

    const int pos = c.position();
    const QString doc = toPlainText();
    if (pos == 0 || pos >= doc.length()) return false;

    static const QHash<QChar, QChar> pairs = {
        {'(',')'}, {'[',']'}, {'{','}'},
        {'"','"'}, {'\'','\''}
    };
    if (pairs.contains(doc[pos-1]) && pairs[doc[pos-1]] == doc[pos]) {
        c.deletePreviousChar();
        c.deleteChar();
        setTextCursor(c);
        return true;
    }
    return false;
}

bool EditorWidget::handleTab(QKeyEvent*) {
    QTextCursor c = textCursor();
    if (c.hasSelection()) { indentSelection(); return true; }
    if (settings_.useSpaces) {
        const int col = c.positionInBlock();
        const int sp = settings_.tabSize - (col % settings_.tabSize);
        c.insertText(QString(sp, ' '));
    } else {
        c.insertText("\t");
    }
    return true;
}

bool EditorWidget::handleEnter(QKeyEvent*) {
    if (!settings_.autoIndent) return false;
    QTextCursor c = textCursor();
    const QString line = c.block().text();
    int indent = 0;
    for (const QChar& ch : line) {
        if (ch == ' ') ++indent;
        else if (ch == '\t') indent += settings_.tabSize;
        else break;
    }
    bool inc = line.trimmed().endsWith(':') &&
               !line.trimmed().startsWith('#');
    QString ni = QString(indent, ' ');
    if (inc) ni += indentStr();
    c.insertText("\n" + ni);
    setTextCursor(c);
    return true;
}

void EditorWidget::showAutoComplete() {
    QString prefix = wordUnderCursor();
    if (prefix.length() < 2) {
        if (completerPopup_) completerPopup_->hide();
        return;
    }

    static const QStringList keywords = {
        "and", "as", "assert", "async", "await", "break", "class",
        "continue", "def", "del", "elif", "else", "except", "False",
        "finally", "for", "from", "global", "if", "import", "in",
        "is", "lambda", "None", "nonlocal", "not", "or", "pass",
        "raise", "return", "True", "try", "while", "with", "yield"
    };

    static const QStringList builtins = {
        "abs", "all", "any", "bool", "bytes", "chr", "dict",
        "dir", "enumerate", "eval", "exec", "filter", "float",
        "format", "getattr", "globals", "hasattr", "hash", "help",
        "hex", "id", "input", "int", "isinstance", "issubclass",
        "iter", "len", "list", "locals", "map", "max", "min",
        "next", "object", "oct", "open", "ord", "pow", "print",
        "property", "range", "repr", "reversed", "round", "set",
        "setattr", "sorted", "str", "sum", "super", "tuple", "type",
        "vars", "zip", "self", "cls", "__init__", "__name__"
    };

    QStringList suggestions;
    suggestions += keywords;
    suggestions += builtins;

    QString text = toPlainText();
    QRegularExpression re("\\b[a-zA-Z_][a-zA-Z0-9_]{2,}\\b");
    auto it = re.globalMatch(text);
    QSet<QString> uniqueWords;
    while (it.hasNext()) {
        auto m = it.next();
        QString word = m.captured();
        if (word != prefix) uniqueWords.insert(word);
    }
    suggestions += QStringList(uniqueWords.begin(), uniqueWords.end());

    QStringList filtered;
    for (const auto& s : suggestions) {
        if (s.startsWith(prefix, Qt::CaseInsensitive) && s != prefix) {
            filtered.append(s);
        }
    }

    if (filtered.isEmpty()) {
        if (completerPopup_) completerPopup_->hide();
        return;
    }

    filtered.removeDuplicates();
    std::sort(filtered.begin(), filtered.end());
    if (filtered.size() > 10) filtered = filtered.mid(0, 10);

    if (!completerPopup_) {
        completerPopup_ = new QListWidget(this);
        completerPopup_->setWindowFlags(
            Qt::ToolTip | Qt::FramelessWindowHint);
        completerPopup_->setFocusPolicy(Qt::NoFocus);
        completerPopup_->setAttribute(Qt::WA_ShowWithoutActivating);
        completerPopup_->setStyleSheet(
            "QListWidget {"
            "  background: #1c2128;"
            "  color: #e6edf3;"
            "  border: 1px solid #58a6ff;"
            "  border-radius: 6px;"
            "  padding: 4px;"
            "  font-family: 'Consolas';"
            "  font-size: 10pt;"
            "  outline: none;"
            "}"
            "QListWidget::item { padding: 4px 12px; border-radius: 3px; }"
            "QListWidget::item:selected { background: #264f78; color: white; }"
        );
        completerPopup_->setFixedWidth(300);

        connect(completerPopup_, &QListWidget::itemActivated,
                [this](QListWidgetItem* item) {
            QString word = item->data(Qt::UserRole).toString();
            QTextCursor c = textCursor();
            c.select(QTextCursor::WordUnderCursor);
            c.insertText(word);
            completerPopup_->hide();
            setFocus();
        });
    }

    completerPopup_->clear();
    for (const auto& s : filtered) {
        auto* item = new QListWidgetItem();
        QString icon = "  ";
        QColor color(230, 237, 243);
        if (keywords.contains(s))      { icon = "* "; color = QColor(255, 123, 114); }
        else if (builtins.contains(s)) { icon = "f "; color = QColor(210, 168, 255); }
        else                            { icon = ". "; color = QColor(230, 237, 243); }
        item->setText(icon + s);
        item->setForeground(color);
        item->setData(Qt::UserRole, s);
        completerPopup_->addItem(item);
    }

    completerPopup_->setCurrentRow(0);

    QRect rect = cursorRect();
    QPoint pos = mapToGlobal(QPoint(rect.left() + gutterWidth(), rect.bottom() + 2));
    completerPopup_->move(pos);

    int height = qMin(220, filtered.size() * 28 + 12);
    completerPopup_->setFixedHeight(height);

    if (!completerPopup_->isVisible()) {
        completerPopup_->show();
    }

    setFocus();
}

void EditorWidget::paintCurrentLine(QPainter& p) {
    const QRect cr = cursorRect();
    p.fillRect(QRect(0, cr.top(), viewport()->width(), cr.height()),
               colors_.cursor_line);
}

} // namespace PyForge