#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QSplitter>
#include <QTreeWidget>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QProcess>
#include "EditorWidget.hpp"
#include "Terminal.hpp"
#include "../utils/Theme.hpp"

namespace PyForge {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    void openFileAt(const QString& path);

protected:
    void closeEvent(QCloseEvent* e) override;

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void saveAll();
    void closeCurrentTab();
    void closeTab(int idx);
    void onTabChanged(int idx);
    void runFile();
    void stopExecution();
    void formatWithBlack();
    void runRuff();
    void toggleComment();
    void duplicateLine();
    void moveLinesUp();
    void moveLinesDown();
    void deleteLine();
    void indentLines();
    void dedentLines();
    void gotoLine();
    void toggleTerminal();
    void clearTerminal();
    void globalSearch();
    void openFolder();
    void commandPalette();
    void showShortcuts();
    void showAbout();
    void applyTheme(const QString& name);
    void undo();  void redo();
    void cut();   void copy(); void paste();
    void selectAll();
    void refreshFileTree();

private:
    void buildUI();
    void buildMenuBar();
    void buildToolBar();
    void buildStatusBar();
    void applyGlobalTheme();
    void setupEditor(EditorWidget* e);
    void saveSession();
    void restoreSession();
    void populateTree(QTreeWidgetItem* parent, const QString& path);
    EditorWidget* currentEditor() const;

    QSplitter*   mainSplitter_  = nullptr;
    QTabWidget*  sidebar_       = nullptr;
    QTreeWidget* fileTree_      = nullptr;
    QTabWidget*  editorTabs_    = nullptr;
    QTabWidget*  bottomPanel_   = nullptr;
    Terminal*    terminal_      = nullptr;
    QWidget*     welcomeWidget_ = nullptr;
    QLineEdit*   searchBar_     = nullptr;

    QLabel* statusPos_      = nullptr;
    QLabel* statusLang_     = nullptr;
    QLabel* statusEncoding_ = nullptr;
    QLabel* statusMsg_      = nullptr;
    QLabel* lspStatus_      = nullptr;

    QProcess* runProcess_ = nullptr;

    QString lastDir_;
    QString currentFolder_;
    EditorSettings settings_;
};

} // namespace PyForge