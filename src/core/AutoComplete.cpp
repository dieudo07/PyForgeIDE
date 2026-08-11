#include "AutoComplete.hpp"
#include <QVBoxLayout>

namespace PyForge {

AutoComplete::AutoComplete(QWidget* parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setFixedWidth(350);
    setMaximumHeight(250);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);

    list_ = new QListWidget(this);
    layout->addWidget(list_);

    connect(list_, &QListWidget::itemActivated,
            [this](QListWidgetItem* item) {
        const int row = list_->row(item);
        if (row >= 0 && row < items_.size()) {
            emit completionSelected(items_[row]);
            hide();
        }
    });

    QFrame::hide();
}

bool AutoComplete::isVisible() const {
    return QFrame::isVisible() && list_->count() > 0;
}

bool AutoComplete::handleKeyPress(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Down:
        list_->setCurrentRow(
            qMin(list_->currentRow() + 1, list_->count() - 1));
        return true;
    case Qt::Key_Up:
        list_->setCurrentRow(
            qMax(list_->currentRow() - 1, 0));
        return true;
    case Qt::Key_Return:
    case Qt::Key_Tab:
        if (list_->currentItem()) {
            const int row = list_->currentRow();
            if (row >= 0 && row < items_.size()) {
                emit completionSelected(items_[row]);
            }
            hide();
            return true;
        }
        return false;
    case Qt::Key_Escape:
        hide();
        return true;
    default:
        return false;
    }
}

void AutoComplete::showCompletions(const QList<CompletionItem>& items,
                                   const QPoint& pos,
                                   const QString& prefix)
{
    items_ = items;
    list_->clear();

    for (const auto& item : items) {
        if (prefix.isEmpty() ||
            item.label.startsWith(prefix, Qt::CaseInsensitive)) {
            list_->addItem(item.label + "  " + item.detail);
        }
    }

    if (list_->count() == 0) { hide(); return; }

    move(pos);
    QFrame::show();
    list_->setCurrentRow(0);
    raise();
}

void AutoComplete::hide() {
    QFrame::hide();
    list_->clear();
}

} // namespace PyForge
#include "AutoComplete.moc"