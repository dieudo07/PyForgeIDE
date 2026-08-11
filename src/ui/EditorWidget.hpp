#include <QListWidget>  // ← Ajouter
#pragma once
#include <QPlainTextEdit>
#include <QTimer>
#include <memory>
#include "../core/SyntaxHighlighter.hpp"
#include "../utils/Theme.hpp"

namespace PyForge {

class GutterWidget;

struct EditorSettings {
    int  tabSize          = 4;
    bool useSpaces        = true;
    bool autoIndent       = true;
    bool autoCloseBraces  = true;
    bool autoCloseQuotes  = true;
    bool highlightLine    = true;
    bool showIndentGuides = true;
    bool bracketMatch     = true;
    bool wordWrap         = false;
};

class EditorWidget : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit EditorWidget(QWidget* parent = nullptr);
    ~EditorWidget() override;

    struct Diagnostic {
        int     line, col, endLine, endCol;
        QString message;
        int     severity;
    };

    // Fichier
    bool    loadFile(const QString& path);
    bool    saveFile(const QString& path = QString());
    bool    isModified() const;
    QString filePath() const { return filePath_; }

    // Éditeur
    int     gutterWidth() const;
    void    applyTheme(const ColorScheme& c);
    void    applySettings(const EditorSettings& s);
    void    setDiagnostics(const QList<Diagnostic>& d);
    void    gotoLine(int line, int col = 0);
    void    toggleComment();
    void    duplicateLine();
    void    moveLinesUp();
    void    moveLinesDown();
    void    deleteCurrentLine();
    void    indentSelection();
    void    dedentSelection();
    bool    findNext(const QString& text,
                     Qt::CaseSensitivity cs = Qt::CaseInsensitive);
    int     replaceAll(const QString& from, const QString& to);
    QPair<int,int> currentPosition() const;

    // Expose les méthodes protected de QPlainTextEdit
    // pour que GutterWidget puisse les utiliser
    using QPlainTextEdit::firstVisibleBlock;
    using QPlainTextEdit::blockBoundingGeometry;
    using QPlainTextEdit::contentOffset;
    using QPlainTextEdit::blockBoundingRect;

signals:
    void fileModified(bool mod);
    void cursorPositionInfo(int line, int col, int sel);
    void completionRequested(const QString& prefix,
                             int line, int col);
    void textSaved(const QString& path);

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void onTextChanged();
    void onCursorMoved();
    void updateGutterWidth();

public:
    void showAutoComplete();  // ← Ajouter

private:
    QString           filePath_;
    bool              modified_ = false;
    EditorSettings    settings_;
    ColorScheme       colors_;
    QList<Diagnostic> diagnostics_;

    std::unique_ptr<SyntaxHighlighter> highlighter_;
    GutterWidget* gutter_    = nullptr;
    QListWidget* completerPopup_ = nullptr; 
    QTimer        saveTimer_;

    bool    handleAutoPair(QKeyEvent* e);
    bool    handleSmartBackspace(QKeyEvent* e);
    bool    handleTab(QKeyEvent* e);
    bool    handleEnter(QKeyEvent* e);

    void    paintCurrentLine(QPainter& p);
    void    paintIndentGuides(QPainter& p);
    void    paintDiagnostics(QPainter& p);
    void    highlightBrackets();
    QString wordUnderCursor() const;
    QString indentStr() const;
};

} // namespace PyForge