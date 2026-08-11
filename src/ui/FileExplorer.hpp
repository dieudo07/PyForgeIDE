#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QDir>

namespace PyForge {

class FileExplorer : public QWidget {
    Q_OBJECT
public:
    explicit FileExplorer(QWidget* parent = nullptr);
    void setRootPath(const QString& path);

signals:
    void fileDoubleClicked(const QString& path);

private:
    QTreeWidget* tree_ = nullptr;
    void populate(QTreeWidgetItem* parent, const QString& path);
};

} // namespace PyForge