#include "FileExplorer.hpp"
#include <QFileInfo>

namespace PyForge {

FileExplorer::FileExplorer(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    tree_ = new QTreeWidget(this);
    tree_->setHeaderLabel("Explorateur");
    tree_->setAnimated(true);
    layout->addWidget(tree_);

    connect(tree_, &QTreeWidget::itemDoubleClicked,
            [this](QTreeWidgetItem* item, int) {
        const QString path = item->data(0, Qt::UserRole).toString();
        if (!path.isEmpty()) emit fileDoubleClicked(path);
    });
}

void FileExplorer::setRootPath(const QString& path) {
    tree_->clear();
    auto* root = new QTreeWidgetItem(tree_);
    root->setText(0, QDir(path).dirName());
    populate(root, path);
    root->setExpanded(true);
}

void FileExplorer::populate(QTreeWidgetItem* parent, const QString& path) {
    QDir dir(path);
    const auto entries = dir.entryInfoList(
        QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
        QDir::DirsFirst | QDir::Name);

    for (const auto& entry : entries) {
        auto* item = new QTreeWidgetItem(parent);
        item->setText(0, entry.fileName());
        if (entry.isDir()) {
            populate(item, entry.filePath());
        } else {
            item->setData(0, Qt::UserRole, entry.filePath());
        }
    }
}

} // namespace PyForge
#include "FileExplorer.moc"