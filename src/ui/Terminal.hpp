#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QProcess>
#include <QKeyEvent>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QColor>

namespace PyForge {

class Terminal : public QWidget {
    Q_OBJECT

public:
    explicit Terminal(QWidget* parent = nullptr);

    void clearScreen();
    void runCommand(const QString& cmd);
    void appendOutput(const QString& text, const QColor& color = Qt::white);
    void setWorkingDirectory(const QString& dir);

private slots:
    void onReturnPressed();
    void onProcessOutput();
    void onProcessError();
    void onProcessFinished(int code);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QTextEdit* output_ = nullptr;
    QLineEdit* input_ = nullptr;
    QLabel* prompt_ = nullptr;
    QProcess* process_ = nullptr;
    QString workingDir_;
    QStringList history_;
    int historyIndex_ = -1;

    void updatePromptText();
};

} // namespace PyForge