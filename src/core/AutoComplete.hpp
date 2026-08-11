#pragma once
#include <QFrame>
#include <QListWidget>
#include <QKeyEvent>

namespace PyForge {

struct CompletionItem {
    QString label;
    QString detail;
    QString insertText;
    int     kind = 0;
};

class AutoComplete : public QFrame {
    Q_OBJECT
public:
    explicit AutoComplete(QWidget* parent = nullptr);
    bool isVisible() const;
    bool handleKeyPress(QKeyEvent* event);
    void showCompletions(const QList<CompletionItem>& items,
                         const QPoint& pos,
                         const QString& prefix);
    void hide();

signals:
    void completionSelected(const CompletionItem& item);

private:
    QListWidget* list_ = nullptr;
    QList<CompletionItem> items_;
};

} // namespace PyForge