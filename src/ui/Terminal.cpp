#include "Terminal.hpp"
#include "Terminal.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QDir>
#include <QScrollBar>
#include <QDir>
#include <QDateTime>

namespace PyForge {

Terminal::Terminal(QWidget* parent) : QWidget(parent) {
    workingDir_ = QDir::currentPath();

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Zone de sortie
    output_ = new QTextEdit(this);
    output_->setReadOnly(true);
    output_->setFont(QFont("Cascadia Code", 10));
    output_->setStyleSheet(
        "QTextEdit {"
        "  background: #0d1117;"
        "  color: #e6edf3;"
        "  border: none;"
        "  padding: 10px;"
        "  font-family: 'Cascadia Code', 'Consolas', monospace;"
        "  font-size: 10pt;"
        "  selection-background-color: #264f78;"
        "}"
    );
    layout->addWidget(output_, 1);

    // Ligne de saisie avec prompt
    auto* inputLine = new QWidget(this);
    inputLine->setStyleSheet(
        "background: #161b22; border-top: 1px solid #30363d;"
    );
    auto* inputLayout = new QHBoxLayout(inputLine);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(0);

    prompt_ = new QLabel();
    prompt_->setStyleSheet(
        "background: #161b22;"
        "color: #3fb950;"
        "font-family: 'Cascadia Code', 'Consolas';"
        "font-size: 10pt;"
        "font-weight: bold;"
        "padding: 6px 10px;"
    );
    updatePromptText();
    inputLayout->addWidget(prompt_);

    input_ = new QLineEdit(this);
    input_->setFont(QFont("Cascadia Code", 10));
    input_->setPlaceholderText("Tapez une commande (ex: dir, python --version, pip list)...");
    input_->setStyleSheet(
        "QLineEdit {"
        "  background: #161b22;"
        "  color: #e6edf3;"
        "  border: none;"
        "  padding: 6px 8px;"
        "  font-family: 'Cascadia Code', 'Consolas', monospace;"
        "  font-size: 10pt;"
        "  selection-background-color: #264f78;"
        "}"
    );
    input_->installEventFilter(this);
    inputLayout->addWidget(input_, 1);

    layout->addWidget(inputLine);

    connect(input_, &QLineEdit::returnPressed,
            this, &Terminal::onReturnPressed);

    // Messages d'accueil
    appendOutput("=====================================", QColor("#58a6ff"));
    appendOutput("   PyForge IDE Terminal v3.0", QColor("#58a6ff"));
    appendOutput("=====================================", QColor("#58a6ff"));
    appendOutput("", QColor("#e6edf3"));
    appendOutput("Bienvenue dans le terminal interactif !", QColor("#7d8590"));
    appendOutput("Astuce: utilisez les fleches Haut/Bas pour l'historique", QColor("#7d8590"));
    appendOutput("", QColor("#e6edf3"));

    input_->setFocus();
}

void Terminal::updatePromptText() {
    QString dir = QDir(workingDir_).dirName();
    if (dir.isEmpty()) dir = workingDir_;
    prompt_->setText(QString("  %1 >").arg(dir));
}

void Terminal::setWorkingDirectory(const QString& dir) {
    workingDir_ = dir;
    updatePromptText();
}

void Terminal::clearScreen() {
    output_->clear();
    appendOutput("Terminal efface", QColor("#7d8590"));
    appendOutput("", QColor("#e6edf3"));
}

void Terminal::runCommand(const QString& cmd) {
    if (cmd.isEmpty()) return;

    // Afficher la commande
    appendOutput(QString("$ %1").arg(cmd), QColor("#58a6ff"));

    // Ajouter à l'historique
    if (history_.isEmpty() || history_.last() != cmd) {
        history_.append(cmd);
        if (history_.size() > 100) history_.removeFirst();
    }
    historyIndex_ = -1;

    // Commandes internes
    if (cmd == "clear" || cmd == "cls") {
        clearScreen();
        return;
    }
    if (cmd == "exit") {
        appendOutput("(Ne pas fermer le terminal integre)", QColor("#d29922"));
        return;
    }
    if (cmd.startsWith("cd ")) {
        QString newDir = cmd.mid(3).trimmed();
        QDir d(workingDir_);
        if (d.cd(newDir)) {
            workingDir_ = d.absolutePath();
            updatePromptText();
            appendOutput(QString("Dossier: %1").arg(workingDir_), QColor("#3fb950"));
        } else {
            appendOutput(QString("Erreur: %1 introuvable").arg(newDir), QColor("#f85149"));
        }
        return;
    }
    if (cmd == "pwd" || cmd == "cd") {
        appendOutput(workingDir_, QColor("#e6edf3"));
        return;
    }

    // Kill le processus précédent s'il existe
    if (process_) {
        process_->kill();
        process_->deleteLater();
        process_ = nullptr;
    }

    process_ = new QProcess(this);
    process_->setWorkingDirectory(workingDir_);
    process_->setProcessChannelMode(QProcess::MergedChannels);

    connect(process_, &QProcess::readyReadStandardOutput,
            this, &Terminal::onProcessOutput);
    connect(process_, &QProcess::readyReadStandardError,
            this, &Terminal::onProcessError);
    connect(process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Terminal::onProcessFinished);

    // Windows utilise cmd /C pour exécuter les commandes
    process_->start("cmd.exe", {"/C", cmd});
}

void Terminal::appendOutput(const QString& text, const QColor& color) {
    QString safe = text.toHtmlEscaped();
    QString html = QString("<span style='color: %1;'>%2</span>")
                   .arg(color.name()).arg(safe);
    output_->append(html);

    // Scroll vers le bas
    QScrollBar* sb = output_->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void Terminal::onReturnPressed() {
    const QString cmd = input_->text().trimmed();
    input_->clear();
    if (!cmd.isEmpty()) runCommand(cmd);
}

void Terminal::onProcessOutput() {
    if (!process_) return;
    const QString text = QString::fromLocal8Bit(
        process_->readAllStandardOutput());
    for (const auto& line : text.split('\n')) {
        QString trimmed = line;
        if (trimmed.endsWith('\r'))
            trimmed.chop(1);
        if (!trimmed.isEmpty())
            appendOutput(trimmed, QColor("#e6edf3"));
    }
}

void Terminal::onProcessError() {
    if (!process_) return;
    const QString text = QString::fromLocal8Bit(
        process_->readAllStandardError());
    for (const auto& line : text.split('\n')) {
        QString trimmed = line;
        if (trimmed.endsWith('\r'))
            trimmed.chop(1);
        if (!trimmed.isEmpty())
            appendOutput(trimmed, QColor("#f85149"));
    }
}

void Terminal::onProcessFinished(int code) {
    if (code != 0) {
        appendOutput(QString("[Code sortie: %1]").arg(code), QColor("#d29922"));
    }
    appendOutput("", QColor("#e6edf3"));
}

bool Terminal::eventFilter(QObject* obj, QEvent* event) {
    if (obj == input_ && event->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Up) {
            if (history_.isEmpty()) return true;
            if (historyIndex_ == -1)
                historyIndex_ = history_.size();
            historyIndex_ = qMax(0, historyIndex_ - 1);
            input_->setText(history_[historyIndex_]);
            return true;
        }
        if (ke->key() == Qt::Key_Down) {
            if (history_.isEmpty() || historyIndex_ == -1) return true;
            historyIndex_ = qMin(history_.size(), historyIndex_ + 1);
            if (historyIndex_ == history_.size()) {
                input_->clear();
                historyIndex_ = -1;
            } else {
                input_->setText(history_[historyIndex_]);
            }
            return true;
        }
        if (ke->key() == Qt::Key_Tab) {
            // Auto-complétion basique
            QString current = input_->text();
            QStringList completions = {
                "python ", "pip install ", "pip list", "pip freeze",
                "dir", "cd ", "cls", "clear", "python --version",
                "python -m ", "git status", "git add .", "git commit -m ",
                "git push", "git pull", "code .", "notepad ",
                "type ", "echo ", "where ", "tasklist", "ipconfig"
            };
            for (const auto& c : completions) {
                if (c.startsWith(current, Qt::CaseInsensitive) && c != current) {
                    input_->setText(c);
                    return true;
                }
            }
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

} // namespace PyForge