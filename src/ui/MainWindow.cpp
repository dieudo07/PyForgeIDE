#include "MainWindow.hpp"
#include <QApplication>
#include <QMenuBar>
#include "MainWindow.hpp"
#include <QApplication>
#include <QPushButton>
#include <QClipboard>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>
#include <QSettings>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QCloseEvent>
#include <QTreeWidget>
#include <QDir>
#include <QTextStream>
#include <QProcess>
#include <QFont>
#include <QSizePolicy>
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHeaderView>
#include <QMenu>

namespace PyForge {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("PyForge IDE 3.0");
    setMinimumSize(1200, 700);
    resize(1600, 950);

    applyGlobalTheme();
    buildUI();
    buildMenuBar();
    buildToolBar();
    buildStatusBar();

    restoreSession();
    if (editorTabs_->count() == 0) newFile();
}

void MainWindow::applyGlobalTheme() {
    const auto& c = Theme::instance().colors;
    qApp->setStyleSheet(QString(R"(
        QMainWindow,QWidget{background:%1;}
        QMenuBar{background:%2;color:%3;border-bottom:1px solid %4;padding:2px 4px;font-size:9pt;}
        QMenuBar::item{padding:4px 10px;border-radius:3px;}
        QMenuBar::item:selected{background:%5;color:%1;}
        QMenu{background:%6;color:%3;border:1px solid %4;border-radius:6px;padding:4px;font-size:9pt;}
        QMenu::item{padding:5px 24px;border-radius:3px;}
        QMenu::item:selected{background:%5;color:%1;}
        QMenu::separator{height:1px;background:%4;margin:4px 12px;}
        QToolBar{background:%2;border-bottom:1px solid %4;spacing:2px;padding:3px 6px;}
        QToolButton{background:transparent;color:%3;border:none;border-radius:4px;padding:4px 8px;font-size:9pt;}
        QToolButton:hover{background:%7;}
        QToolButton:pressed{background:%5;color:%1;}
        QSplitter::handle{background:%4;}
        QSplitter::handle:horizontal{width:1px;}
        QSplitter::handle:vertical{height:1px;}
        QLineEdit{background:%6;color:%3;border:1px solid %4;border-radius:4px;padding:4px 8px;}
        QLineEdit:focus{border-color:%5;}
        QScrollBar:vertical{background:%1;width:10px;border:none;}
        QScrollBar::handle:vertical{background:%4;border-radius:5px;min-height:24px;}
        QScrollBar::handle:vertical:hover{background:%9;}
        QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}
        QScrollBar:horizontal{background:%1;height:10px;border:none;}
        QScrollBar::handle:horizontal{background:%4;border-radius:5px;min-width:24px;}
        QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal{width:0;}
        QLabel{color:%3;}
        QPushButton{background:%6;color:%3;border:1px solid %4;border-radius:4px;padding:5px 12px;}
        QPushButton:hover{background:%7;}
        QToolTip{background:%6;color:%3;border:1px solid %5;border-radius:4px;padding:4px 8px;}
    )")
    .arg(c.bg_editor.name())      // %1
    .arg(c.bg_toolbar.name())     // %2
    .arg(c.text_primary.name())   // %3
    .arg(c.bg_hover.name())       // %4
    .arg(c.accent.name())         // %5
    .arg(c.bg_dropdown.name())    // %6
    .arg(c.bg_hover.name())       // %7
    .arg(c.bg_tab.name())         // %8
    .arg(c.text_secondary.name()) // %9
    );
}

void MainWindow::buildUI() {
    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    mainSplitter_->setHandleWidth(1);
    setCentralWidget(mainSplitter_);

    // ═══ SIDEBAR AVEC EXPLORATEUR ═══
    auto* sidebarWidget = new QWidget();
    sidebarWidget->setFixedWidth(280);
    auto* sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);

    // Header sidebar
    auto* sideHeader = new QWidget();
    sideHeader->setFixedHeight(35);
    sideHeader->setStyleSheet("background: #010409; border-bottom: 1px solid #30363d;");
    auto* sideHeaderLay = new QHBoxLayout(sideHeader);
    sideHeaderLay->setContentsMargins(10, 0, 5, 0);

    auto* explorerLabel = new QLabel("EXPLORATEUR");
    explorerLabel->setStyleSheet(
        "color: #7d8590; font-weight: bold; font-size: 8pt;");
    sideHeaderLay->addWidget(explorerLabel);
    sideHeaderLay->addStretch();

    // Bouton "Ouvrir dossier"
    auto* openFolderBtn = new QPushButton("📁 Ouvrir");
    openFolderBtn->setFixedHeight(24);
    openFolderBtn->setCursor(Qt::PointingHandCursor);
    openFolderBtn->setStyleSheet(
        "QPushButton {"
        "  background: #238636; color: white;"
        "  border: none; border-radius: 3px;"
        "  padding: 2px 10px; font-size: 8pt; font-weight: bold;"
        "}"
        "QPushButton:hover { background: #2ea043; }"
    );
    connect(openFolderBtn, &QPushButton::clicked, this, &MainWindow::openFolder);
    sideHeaderLay->addWidget(openFolderBtn);

    auto* refreshBtn = new QPushButton("⟳");
    refreshBtn->setFixedSize(24, 24);
    refreshBtn->setToolTip("Rafraîchir");
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #7d8590; "
        "  border: none; font-size: 12pt; }"
        "QPushButton:hover { color: #58a6ff; }"
    );
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshFileTree);
    sideHeaderLay->addWidget(refreshBtn);

    sidebarLayout->addWidget(sideHeader);

    // Arbre de fichiers
    fileTree_ = new QTreeWidget();
    fileTree_->setHeaderHidden(true);
    fileTree_->setAnimated(true);
    fileTree_->setIndentation(15);
    fileTree_->setStyleSheet(
        "QTreeWidget {"
        "  background: #161b22;"
        "  color: #e6edf3;"
        "  border: none;"
        "  font-size: 9pt;"
        "  padding: 5px;"
        "  outline: none;"
        "}"
        "QTreeWidget::item {"
        "  padding: 4px 2px;"
        "  border-radius: 3px;"
        "}"
        "QTreeWidget::item:hover { background: #21262d; }"
        "QTreeWidget::item:selected { background: #264f78; color: white; }"
    );

    connect(fileTree_, &QTreeWidget::itemDoubleClicked,
            [this](QTreeWidgetItem* item, int) {
        QString path = item->data(0, Qt::UserRole).toString();
        if (!path.isEmpty() && QFile::exists(path))
            openFileAt(path);
    });

    // Menu contextuel clic droit
    fileTree_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(fileTree_, &QTreeWidget::customContextMenuRequested,
            [this](const QPoint& pos) {
        auto* item = fileTree_->itemAt(pos);
        if (!item) return;
        QString path = item->data(0, Qt::UserRole).toString();

        QMenu menu(this);
        menu.setStyleSheet(
            "QMenu { background: #1c2128; color: #e6edf3; "
            "  border: 1px solid #30363d; border-radius: 6px; padding: 4px; }"
            "QMenu::item { padding: 6px 20px; border-radius: 3px; }"
            "QMenu::item:selected { background: #264f78; }"
        );

        if (!path.isEmpty()) {
            menu.addAction("📂 Ouvrir", [this, path]{ openFileAt(path); });
            menu.addAction("📋 Copier le chemin", [path]{
                QApplication::clipboard()->setText(path);
            });
            menu.addSeparator();
            menu.addAction("🗑 Supprimer", [this, path, item]{
                if (QMessageBox::question(this, "Supprimer",
                    "Supprimer " + QFileInfo(path).fileName() + " ?") == QMessageBox::Yes) {
                    QFile::remove(path);
                    delete item;
                }
            });
        }
        menu.exec(fileTree_->mapToGlobal(pos));
    });

    sidebarLayout->addWidget(fileTree_);

    // Message vide au début
    auto* emptyMsg = new QTreeWidgetItem(fileTree_);
    emptyMsg->setText(0, "Cliquez sur 'Ouvrir' pour");
    emptyMsg->setForeground(0, QColor("#7d8590"));
    auto* emptyMsg2 = new QTreeWidgetItem(fileTree_);
    emptyMsg2->setText(0, "sélectionner un dossier");
    emptyMsg2->setForeground(0, QColor("#7d8590"));

    mainSplitter_->addWidget(sidebarWidget);

    // ═══ CENTRE ═══
    auto* center = new QSplitter(Qt::Vertical);

    editorTabs_ = new QTabWidget();
    editorTabs_->setTabsClosable(true);
    editorTabs_->setMovable(true);
    editorTabs_->setDocumentMode(true);
    editorTabs_->setStyleSheet(
        "QTabWidget::pane { background: #0d1117; border: none; }"
        "QTabBar::tab { background: #161b22; color: #7d8590; "
        "  padding: 8px 16px; border-right: 1px solid #30363d; "
        "  min-width: 100px; font-size: 9pt; }"
        "QTabBar::tab:selected { background: #0d1117; color: #e6edf3; "
        "  border-top: 2px solid #58a6ff; }"
        "QTabBar::tab:hover:!selected { background: #21262d; color: #e6edf3; }"
    );
    connect(editorTabs_, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(editorTabs_, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    center->addWidget(editorTabs_);

    // ═══ TERMINAL INTERACTIF ═══
    bottomPanel_ = new QTabWidget();
    bottomPanel_->setTabPosition(QTabWidget::North);
    bottomPanel_->setMinimumHeight(180);
    bottomPanel_->setMaximumHeight(300);
    bottomPanel_->setStyleSheet(
        "QTabWidget { background: #0d1117; }"
        "QTabWidget::pane { background: #0d1117; "
        "  border-top: 1px solid #30363d; border-left: none; "
        "  border-right: none; border-bottom: none; }"
        "QTabBar { background: #161b22; }"
        "QTabBar::tab { background: #161b22; color: #7d8590; "
        "  padding: 6px 20px; border: none; border-right: 1px solid #30363d; "
        "  font-size: 9pt; font-weight: bold; }"
        "QTabBar::tab:selected { background: #0d1117; color: #58a6ff; "
        "  border-top: 2px solid #58a6ff; }"
    );

    terminal_ = new Terminal(this);

    auto* problems = new QTextEdit();
    problems->setReadOnly(true);
    problems->setStyleSheet(
        "QTextEdit { background: #0d1117; color: #e6edf3; border: none; padding: 10px; }"
    );
    problems->append("<span style='color: #3fb950;'>[OK] Aucun probleme detecte</span>");

    auto* output = new QTextEdit();
    output->setReadOnly(true);
    output->setStyleSheet(
        "QTextEdit { background: #0d1117; color: #e6edf3; border: none; padding: 10px; }"
    );

    bottomPanel_->addTab(terminal_, "TERMINAL");
    bottomPanel_->addTab(problems, "PROBLEMES");
    bottomPanel_->addTab(output, "SORTIE");

    center->addWidget(bottomPanel_);
    center->setStretchFactor(0, 3);
    center->setStretchFactor(1, 1);

    mainSplitter_->addWidget(center);
    mainSplitter_->setStretchFactor(0, 0);
    mainSplitter_->setStretchFactor(1, 1);
}

void MainWindow::populateTree(QTreeWidgetItem* parent, const QString& path) {
    QDir d(path);
    auto entries = d.entryInfoList(
        QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
        QDir::DirsFirst | QDir::Name);

    for (const auto& entry : entries) {
        if (entry.fileName().startsWith('.') ||
            entry.fileName() == "__pycache__" ||
            entry.fileName() == "node_modules" ||
            entry.fileName() == "build") continue;

        auto* item = new QTreeWidgetItem(parent);

        if (entry.isDir()) {
            item->setText(0, "📁 " + entry.fileName());
            item->setForeground(0, QColor("#58a6ff"));
            populateTree(item, entry.filePath());
        } else {
            QString icon;
            QString ext = entry.suffix().toLower();
            if (ext == "py") icon = "🐍";
            else if (ext == "md") icon = "📝";
            else if (ext == "json") icon = "📋";
            else if (ext == "txt") icon = "📄";
            else if (ext == "html" || ext == "htm") icon = "🌐";
            else if (ext == "css") icon = "🎨";
            else if (ext == "js") icon = "⚡";
            else if (ext == "cpp" || ext == "hpp" || ext == "h" || ext == "c") icon = "⚙";
            else if (ext == "exe") icon = "🚀";
            else if (ext == "png" || ext == "jpg" || ext == "ico") icon = "🖼";
            else icon = "📄";

            item->setText(0, icon + " " + entry.fileName());
            item->setData(0, Qt::UserRole, entry.filePath());
            item->setForeground(0, QColor("#e6edf3"));
        }
    }
}

void MainWindow::openFolder() {
    const QString dir = QFileDialog::getExistingDirectory(this, "Ouvrir dossier", lastDir_);
    if (dir.isEmpty()) return;
    lastDir_ = dir;
    currentFolder_ = dir;
    refreshFileTree();
    statusMsg_->setText("  📁 " + QDir(dir).dirName());

    // Aussi changer le dossier du terminal
    if (terminal_) terminal_->setWorkingDirectory(dir);
}

void MainWindow::refreshFileTree() {
    if (currentFolder_.isEmpty()) return;
    fileTree_->clear();
    auto* root = new QTreeWidgetItem(fileTree_);
    root->setText(0, "📁 " + QDir(currentFolder_).dirName());
    root->setForeground(0, QColor("#ffa657"));
    QFont f = root->font(0); f.setBold(true);
    root->setFont(0, f);
    populateTree(root, currentFolder_);
    root->setExpanded(true);
}

void MainWindow::buildMenuBar() {
    auto* mb = menuBar();

    auto* fileMenu = mb->addMenu("Fichier");
    fileMenu->addAction("Nouveau", this, &MainWindow::newFile, QKeySequence::New);
    fileMenu->addAction("Ouvrir...", this, &MainWindow::openFile, QKeySequence::Open);
    fileMenu->addAction("📁 Ouvrir dossier...", this, &MainWindow::openFolder,
                       QKeySequence(Qt::CTRL|Qt::Key_K, Qt::CTRL|Qt::Key_O));
    fileMenu->addSeparator();
    fileMenu->addAction("Sauvegarder", this, &MainWindow::saveFile, QKeySequence::Save);
    fileMenu->addAction("Sauvegarder sous...", this, &MainWindow::saveFileAs, QKeySequence::SaveAs);
    fileMenu->addAction("Tout sauvegarder", this, &MainWindow::saveAll);
    fileMenu->addSeparator();
    fileMenu->addAction("Fermer", this, &MainWindow::closeCurrentTab, QKeySequence::Close);
    fileMenu->addSeparator();
    fileMenu->addAction("Quitter", qApp, &QApplication::quit, QKeySequence::Quit);

    auto* editMenu = mb->addMenu("Edition");
    editMenu->addAction("Annuler", this, &MainWindow::undo, QKeySequence::Undo);
    editMenu->addAction("Retablir", this, &MainWindow::redo, QKeySequence::Redo);
    editMenu->addSeparator();
    editMenu->addAction("Couper", this, &MainWindow::cut, QKeySequence::Cut);
    editMenu->addAction("Copier", this, &MainWindow::copy, QKeySequence::Copy);
    editMenu->addAction("Coller", this, &MainWindow::paste, QKeySequence::Paste);
    editMenu->addAction("Tout selectionner", this, &MainWindow::selectAll, QKeySequence::SelectAll);
    editMenu->addSeparator();
    editMenu->addAction("Commenter", this, &MainWindow::toggleComment, QKeySequence(Qt::CTRL|Qt::Key_Slash));
    editMenu->addAction("Dupliquer la ligne", this, &MainWindow::duplicateLine, QKeySequence(Qt::CTRL|Qt::Key_D));
    editMenu->addAction("Supprimer la ligne", this, &MainWindow::deleteLine, QKeySequence(Qt::CTRL|Qt::Key_K));
    editMenu->addAction("Monter la ligne", this, &MainWindow::moveLinesUp, QKeySequence(Qt::ALT|Qt::Key_Up));
    editMenu->addAction("Descendre la ligne", this, &MainWindow::moveLinesDown, QKeySequence(Qt::ALT|Qt::Key_Down));
    editMenu->addAction("Indenter", this, &MainWindow::indentLines, QKeySequence(Qt::CTRL|Qt::Key_BracketRight));
    editMenu->addAction("Desindenter", this, &MainWindow::dedentLines, QKeySequence(Qt::CTRL|Qt::Key_BracketLeft));

    auto* viewMenu = mb->addMenu("Affichage");
    viewMenu->addAction("Terminal (Ctrl+`)", this, &MainWindow::toggleTerminal);
    viewMenu->addAction("Aller a la ligne...", this, &MainWindow::gotoLine, QKeySequence(Qt::CTRL|Qt::Key_G));
    viewMenu->addAction("Palette de commandes", this, &MainWindow::commandPalette,
                       QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_P));

    auto* themeMenu = viewMenu->addMenu("Theme");
    themeMenu->addAction("GitHub Dark", [this]{ applyTheme("github"); });
    themeMenu->addAction("One Dark Pro", [this]{ applyTheme("onedark"); });
    themeMenu->addAction("Monokai Pro", [this]{ applyTheme("monokai"); });
    themeMenu->addAction("Dracula", [this]{ applyTheme("dracula"); });

    auto* runMenu = mb->addMenu("Executer");
    runMenu->addAction("Executer (F5)", this, &MainWindow::runFile, Qt::Key_F5);
    runMenu->addAction("Arreter", this, &MainWindow::stopExecution, Qt::Key_F6);
    runMenu->addSeparator();
    runMenu->addAction("Formater (Black)", this, &MainWindow::formatWithBlack,
                      QKeySequence(Qt::SHIFT|Qt::ALT|Qt::Key_F));
    runMenu->addAction("Linter (Ruff)", this, &MainWindow::runRuff);

    auto* helpMenu = mb->addMenu("Aide");
    helpMenu->addAction("Raccourcis clavier", this, &MainWindow::showShortcuts);
    helpMenu->addSeparator();
    helpMenu->addAction("A propos", this, &MainWindow::showAbout);

    new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_QuoteLeft), this,
                  this, &MainWindow::toggleTerminal);
}

void MainWindow::buildToolBar() {
    auto* tb = addToolBar("Principal");
    tb->setMovable(false);

    tb->addAction("📄 Nouveau", this, &MainWindow::newFile);
    tb->addAction("📂 Ouvrir", this, &MainWindow::openFile);
    tb->addAction("📁 Dossier", this, &MainWindow::openFolder);
    tb->addAction("💾 Sauver", this, &MainWindow::saveFile);
    tb->addSeparator();

    auto* runAct = tb->addAction("▶ Executer", this, &MainWindow::runFile);
    if (auto* btn = tb->widgetForAction(runAct))
        btn->setStyleSheet(
            "QToolButton{background:#3fb950;color:#0d1117;font-weight:bold;"
            "border-radius:4px;padding:4px 12px;}"
            "QToolButton:hover{background:#46c85a;}");

    tb->addAction("⏹ Stop", this, &MainWindow::stopExecution);
    tb->addSeparator();
    tb->addAction("🎨 Format", this, &MainWindow::formatWithBlack);
    tb->addSeparator();

    searchBar_ = new QLineEdit();
    searchBar_->setPlaceholderText("🔍 Rechercher...");
    searchBar_->setFixedWidth(240);
    searchBar_->setClearButtonEnabled(true);
    connect(searchBar_, &QLineEdit::returnPressed, this, &MainWindow::globalSearch);
    tb->addWidget(searchBar_);

    auto* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacer);

    lspStatus_ = new QLabel("  Ctrl+Shift+P = Palette  ");
    lspStatus_->setStyleSheet("color:#7d8590;font-size:8pt;");
    tb->addWidget(lspStatus_);
}

void MainWindow::buildStatusBar() {
    auto* sb = statusBar();
    sb->setStyleSheet(R"(
        QStatusBar { background: #161b22; color: #e6edf3;
                     border-top: 1px solid #30363d; padding: 0; }
        QStatusBar::item { border: none; }
    )");

    statusPos_ = new QLabel("  Ligne 1, Col 1  ");
    statusPos_->setStyleSheet("color: #58a6ff; padding: 3px 8px;");
    sb->addWidget(statusPos_);

    auto* sep1 = new QLabel("|");
    sep1->setStyleSheet("color: #30363d; padding: 3px 4px;");
    sb->addPermanentWidget(sep1);

    statusLang_ = new QLabel(" Python ");
    statusLang_->setStyleSheet("color: #3fb950; font-weight: bold; padding: 3px 8px;");
    sb->addPermanentWidget(statusLang_);

    auto* sep2 = new QLabel("|");
    sep2->setStyleSheet("color: #30363d; padding: 3px 4px;");
    sb->addPermanentWidget(sep2);

    statusEncoding_ = new QLabel(" UTF-8 ");
    statusEncoding_->setStyleSheet("color: #d2a8ff; padding: 3px 8px;");
    sb->addPermanentWidget(statusEncoding_);

    auto* sep3 = new QLabel("|");
    sep3->setStyleSheet("color: #30363d; padding: 3px 4px;");
    sb->addPermanentWidget(sep3);

    statusMsg_ = new QLabel("  Pret");
    statusMsg_->setStyleSheet("color: #7d8590; padding: 3px 8px; min-width: 200px;");
    sb->addPermanentWidget(statusMsg_);

    auto* version = new QLabel(" PyForge v3.0 ");
    version->setStyleSheet(
        "color: #58a6ff; font-weight: bold; "
        "background: #0d1117; padding: 3px 12px; "
        "border-left: 2px solid #58a6ff;");
    sb->addPermanentWidget(version);
}

void MainWindow::newFile() {
    auto* e = new EditorWidget(this);
    setupEditor(e);
    const int idx = editorTabs_->addTab(e, "Sans titre");
    editorTabs_->setCurrentIndex(idx);
    e->setFocus();
}

void MainWindow::openFile() {
    const QStringList paths = QFileDialog::getOpenFileNames(this,
        "Ouvrir", lastDir_, "Python (*.py *.pyw *.pyi);;Tous (*)");
    for (const auto& p : paths) {
        openFileAt(p);
        lastDir_ = QFileInfo(p).dir().absolutePath();
    }
}

void MainWindow::openFileAt(const QString& path) {
    for (int i = 0; i < editorTabs_->count(); ++i)
        if (auto* e = qobject_cast<EditorWidget*>(editorTabs_->widget(i)))
            if (e->filePath() == path) { editorTabs_->setCurrentIndex(i); return; }

    auto* e = new EditorWidget(this);
    setupEditor(e);
    if (!e->loadFile(path)) { delete e; return; }

    const int idx = editorTabs_->addTab(e, QFileInfo(path).fileName());
    editorTabs_->setTabToolTip(idx, path);
    editorTabs_->setCurrentIndex(idx);
    e->setFocus();
}

void MainWindow::setupEditor(EditorWidget* e) {
    e->applyTheme(Theme::instance().colors);
    e->applySettings(settings_);

    connect(e, &EditorWidget::cursorPositionInfo,
            [this](int line, int col, int sel) {
        statusPos_->setText(QString("  Ligne %1, Col %2%3  ")
            .arg(line).arg(col)
            .arg(sel > 0 ? QString("  (%1 sel.)").arg(sel) : ""));
    });

    connect(e, &EditorWidget::fileModified, [this, e](bool mod) {
        const int idx = editorTabs_->indexOf(e);
        if (idx < 0) return;
        QString title = editorTabs_->tabText(idx).remove("● ");
        editorTabs_->setTabText(idx, mod ? "● " + title : title);
    });

    connect(e, &EditorWidget::textSaved, [this](const QString& path) {
        statusMsg_->setText("  ✓ " + QFileInfo(path).fileName());
        QTimer::singleShot(2500, [this] { statusMsg_->setText("  Pret"); });
    });
}

void MainWindow::saveFile() {
    if (auto* e = currentEditor()) {
        if (e->filePath().isEmpty()) saveFileAs();
        else e->saveFile();
    }
}

void MainWindow::saveFileAs() {
    if (auto* e = currentEditor()) {
        const QString p = QFileDialog::getSaveFileName(this, "Sauvegarder",
            lastDir_, "Python (*.py);;Tous (*)");
        if (!p.isEmpty()) {
            e->saveFile(p);
            const int idx = editorTabs_->indexOf(e);
            if (idx >= 0) editorTabs_->setTabText(idx, QFileInfo(p).fileName());
        }
    }
}

void MainWindow::saveAll() {
    for (int i = 0; i < editorTabs_->count(); ++i)
        if (auto* e = qobject_cast<EditorWidget*>(editorTabs_->widget(i)))
            if (!e->filePath().isEmpty()) e->saveFile();
}

void MainWindow::closeCurrentTab() { closeTab(editorTabs_->currentIndex()); }

void MainWindow::closeTab(int idx) {
    auto* e = qobject_cast<EditorWidget*>(editorTabs_->widget(idx));
    if (e && e->isModified()) {
        const int r = QMessageBox::question(this, "Sauvegarder?",
            "Sauvegarder les modifications ?",
            QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
        if (r == QMessageBox::Save) saveFile();
        else if (r == QMessageBox::Cancel) return;
    }
    editorTabs_->removeTab(idx);
}

void MainWindow::onTabChanged(int idx) {
    if (auto* e = qobject_cast<EditorWidget*>(editorTabs_->widget(idx))) {
        auto pos = e->currentPosition();
        statusPos_->setText(QString("  Ligne %1, Col %2  ").arg(pos.first).arg(pos.second));
    }
}

void MainWindow::runFile() {
    saveFile();
    auto* e = currentEditor();
    if (!e || e->filePath().isEmpty()) {
        statusMsg_->setText("  ⚠ Sauvegardez d'abord");
        return;
    }

    bottomPanel_->setCurrentIndex(0);
    QString workDir = QFileInfo(e->filePath()).absolutePath();
    terminal_->setWorkingDirectory(workDir);
    QString scriptName = QFileInfo(e->filePath()).fileName();
    terminal_->runCommand(QString("python %1").arg(scriptName));

    statusMsg_->setText("  ▶ Execution en cours...");
}

void MainWindow::stopExecution() {
    statusMsg_->setText("  Arrete");
}

void MainWindow::formatWithBlack() {
    auto* e = currentEditor();
    if (!e || e->filePath().isEmpty()) return;
    saveFile();
    QProcess p;
    p.start("black", {e->filePath(), "--line-length=88"});
    p.waitForFinished(15000);
    if (p.exitCode() == 0) {
        e->loadFile(e->filePath());
        statusMsg_->setText("  ✓ Formate avec Black");
    } else {
        statusMsg_->setText("  ⚠ Erreur Black");
    }
    QTimer::singleShot(3000, [this]{ statusMsg_->setText("  Pret"); });
}

void MainWindow::runRuff() {
    auto* e = currentEditor();
    if (!e || e->filePath().isEmpty()) return;
    bottomPanel_->setCurrentIndex(0);
    terminal_->runCommand(QString("ruff check \"%1\"").arg(e->filePath()));
}

void MainWindow::gotoLine() {
    auto* e = currentEditor();
    if (!e) return;
    bool ok;
    const int line = QInputDialog::getInt(this, "Aller a la ligne",
        "Numero:", 1, 1, e->document()->blockCount(), 1, &ok);
    if (ok) e->gotoLine(line - 1);
}

void MainWindow::toggleTerminal() {
    bottomPanel_->setVisible(!bottomPanel_->isVisible());
}

void MainWindow::clearTerminal() {
    if (terminal_) terminal_->clearScreen();
}

void MainWindow::globalSearch() {
    const QString text = searchBar_->text();
    if (text.isEmpty()) return;
    if (auto* e = currentEditor()) {
        if (e->findNext(text)) statusMsg_->setText("  ✓ " + text);
        else statusMsg_->setText("  ⚠ Non trouve");
    }
}

void MainWindow::commandPalette() {
    QDialog dlg(this);
    dlg.setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    dlg.setFixedSize(600, 400);
    dlg.setStyleSheet("QDialog { background: #1c2128; border: 2px solid #30363d; }");

    QPoint center = geometry().center();
    dlg.move(center.x() - 300, center.y() - 200);

    auto* lay = new QVBoxLayout(&dlg);
    lay->setContentsMargins(2, 2, 2, 2);
    lay->setSpacing(0);

    auto* header = new QLabel("  🎯 PALETTE DE COMMANDES");
    header->setStyleSheet(
        "background: #0d1117; color: #58a6ff; padding: 10px; "
        "font-weight: bold; font-size: 10pt; border-bottom: 1px solid #30363d;");
    lay->addWidget(header);

    auto* search = new QLineEdit();
    search->setPlaceholderText("Tapez une commande...");
    search->setStyleSheet(
        "QLineEdit { background: #0d1117; color: #e6edf3; border: none; "
        "  border-bottom: 1px solid #30363d; padding: 12px; font-size: 11pt; }");
    lay->addWidget(search);

    auto* list = new QListWidget();
    list->setStyleSheet(
        "QListWidget { background: #1c2128; color: #e6edf3; border: none; padding: 5px; }"
        "QListWidget::item { padding: 8px 12px; border-radius: 4px; }"
        "QListWidget::item:selected { background: #264f78; }");

    struct Cmd { QString name, sc, cat; std::function<void()> act; };
    QList<Cmd> commands = {
        {"Nouveau fichier", "Ctrl+N", "Fichier", [this]{newFile();}},
        {"Ouvrir fichier", "Ctrl+O", "Fichier", [this]{openFile();}},
        {"📁 Ouvrir dossier", "", "Fichier", [this]{openFolder();}},
        {"Sauvegarder", "Ctrl+S", "Fichier", [this]{saveFile();}},
        {"Executer", "F5", "Executer", [this]{runFile();}},
        {"Formater (Black)", "Shift+Alt+F", "Format", [this]{formatWithBlack();}},
        {"Commenter", "Ctrl+/", "Edition", [this]{toggleComment();}},
        {"Dupliquer ligne", "Ctrl+D", "Edition", [this]{duplicateLine();}},
        {"Aller a la ligne", "Ctrl+G", "Navig", [this]{gotoLine();}},
        {"Terminal", "Ctrl+`", "Vue", [this]{toggleTerminal();}},
        {"Theme GitHub Dark", "", "Theme", [this]{applyTheme("github");}},
        {"Theme One Dark Pro", "", "Theme", [this]{applyTheme("onedark");}},
        {"Theme Monokai Pro", "", "Theme", [this]{applyTheme("monokai");}},
        {"Theme Dracula", "", "Theme", [this]{applyTheme("dracula");}},
    };

    auto refresh = [&](const QString& f) {
        list->clear();
        for (int i = 0; i < commands.size(); ++i) {
            const auto& c = commands[i];
            if (f.isEmpty() || c.name.contains(f, Qt::CaseInsensitive)) {
                auto* item = new QListWidgetItem(list);
                item->setText(QString("%1     [%2]     %3").arg(c.name).arg(c.sc).arg(c.cat));
                item->setData(Qt::UserRole, i);
            }
        }
        if (list->count() > 0) list->setCurrentRow(0);
    };
    refresh("");
    connect(search, &QLineEdit::textChanged, refresh);

    auto exec = [&]{
        if (!list->currentItem()) return;
        int i = list->currentItem()->data(Qt::UserRole).toInt();
        dlg.accept();
        commands[i].act();
    };
    connect(list, &QListWidget::itemActivated, [&](QListWidgetItem*) { exec(); });
    connect(search, &QLineEdit::returnPressed, exec);

    lay->addWidget(list);
    search->setFocus();
    dlg.exec();
}

void MainWindow::applyTheme(const QString& name) {
    auto& t = Theme::instance();
    if (name == "onedark") t.applyOneDarkPro();
    else if (name == "monokai") t.applyMonokai();
    else if (name == "dracula") t.applyDracula();
    else t.applyGitHubDark();
    applyGlobalTheme();
    for (int i = 0; i < editorTabs_->count(); ++i)
        if (auto* e = qobject_cast<EditorWidget*>(editorTabs_->widget(i)))
            e->applyTheme(t.colors);
    statusMsg_->setText("  Theme: " + t.name);
}

void MainWindow::showShortcuts() {
    QMessageBox::information(this, "Raccourcis",
        "Ctrl+N          Nouveau fichier\n"
        "Ctrl+O          Ouvrir fichier\n"
        "Ctrl+K Ctrl+O   Ouvrir dossier\n"
        "Ctrl+S          Sauvegarder\n"
        "Ctrl+Shift+P    Palette de commandes\n"
        "F5              Executer\n"
        "Ctrl+/          Commenter\n"
        "Ctrl+D          Dupliquer ligne\n"
        "Ctrl+K          Supprimer ligne\n"
        "Alt+Up/Down     Deplacer ligne\n"
        "Ctrl+Space      Auto-completion\n"
        "Ctrl+G          Aller a la ligne\n"
        "Ctrl+`          Terminal\n"
        "Shift+Alt+F     Formater (Black)");
}

void MainWindow::showAbout() {
    QMessageBox::about(this, "PyForge IDE v3.0",
        "PyForge IDE 3.0\n\n"
        "Editeur Python haute performance\n\n"
        "Nouveautes v3.0:\n"
        "  ✓ Terminal interactif\n"
        "  ✓ Auto-completion Python\n"
        "  ✓ Gestionnaire de fichiers\n"
        "  ✓ Logo professionnel\n\n"
        "F5 = Executer | Ctrl+Shift+P = Palette");
}

void MainWindow::undo() { if (auto* e=currentEditor()) e->undo(); }
void MainWindow::redo() { if (auto* e=currentEditor()) e->redo(); }
void MainWindow::cut() { if (auto* e=currentEditor()) e->cut(); }
void MainWindow::copy() { if (auto* e=currentEditor()) e->copy(); }
void MainWindow::paste() { if (auto* e=currentEditor()) e->paste(); }
void MainWindow::selectAll() { if (auto* e=currentEditor()) e->selectAll(); }
void MainWindow::toggleComment() { if (auto* e=currentEditor()) e->toggleComment(); }
void MainWindow::duplicateLine() { if (auto* e=currentEditor()) e->duplicateLine(); }
void MainWindow::moveLinesUp() { if (auto* e=currentEditor()) e->moveLinesUp(); }
void MainWindow::moveLinesDown() { if (auto* e=currentEditor()) e->moveLinesDown(); }
void MainWindow::deleteLine() { if (auto* e=currentEditor()) e->deleteCurrentLine(); }
void MainWindow::indentLines() { if (auto* e=currentEditor()) e->indentSelection(); }
void MainWindow::dedentLines() { if (auto* e=currentEditor()) e->dedentSelection(); }

EditorWidget* MainWindow::currentEditor() const {
    return qobject_cast<EditorWidget*>(editorTabs_->currentWidget());
}

void MainWindow::saveSession() {
    QSettings s("PyForge", "IDE");
    QStringList files;
    for (int i = 0; i < editorTabs_->count(); ++i)
        if (auto* e = qobject_cast<EditorWidget*>(editorTabs_->widget(i)))
            if (!e->filePath().isEmpty()) files << e->filePath();
    s.setValue("session/files", files);
    s.setValue("session/geometry", saveGeometry());
    s.setValue("session/folder", currentFolder_);
}

void MainWindow::restoreSession() {
    QSettings s("PyForge", "IDE");
    const auto files = s.value("session/files").toStringList();
    for (const auto& f : files) if (QFile::exists(f)) openFileAt(f);
    const auto geo = s.value("session/geometry").toByteArray();
    if (!geo.isEmpty()) restoreGeometry(geo);
    const auto folder = s.value("session/folder").toString();
    if (!folder.isEmpty() && QDir(folder).exists()) {
        currentFolder_ = folder;
        refreshFileTree();
    }
}

void MainWindow::closeEvent(QCloseEvent* e) {
    saveSession();
    e->accept();
}

} // namespace PyForge

