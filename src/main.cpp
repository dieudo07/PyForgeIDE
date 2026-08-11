#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QFont>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include "ui/MainWindow.hpp"
#include "utils/Theme.hpp"

int main(int argc, char* argv[]) {
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName("PyForge IDE");
    app.setApplicationVersion("3.0.0");
    app.setOrganizationName("PyForge");
    app.setFont(QFont("Segoe UI", 9));

    // Icône de l'application
    QIcon appIcon;
    appIcon.addFile(":/icons/pyforge_16.png",  QSize(16, 16));
    appIcon.addFile(":/icons/pyforge_32.png",  QSize(32, 32));
    appIcon.addFile(":/icons/pyforge_48.png",  QSize(48, 48));
    appIcon.addFile(":/icons/pyforge_64.png",  QSize(64, 64));
    appIcon.addFile(":/icons/pyforge_128.png", QSize(128, 128));
    appIcon.addFile(":/icons/pyforge_256.png", QSize(256, 256));
    app.setWindowIcon(appIcon);

    // Splash screen amélioré
    QPixmap splash(480, 280);
    splash.fill(QColor("#0d1117"));
    QPainter p(&splash);
    p.setRenderHint(QPainter::Antialiasing);

    // Bordure
    p.setPen(QPen(QColor("#30363d"), 2));
    p.drawRoundedRect(2, 2, 476, 276, 10, 10);

    // Logo
    QPixmap logo(":/icons/pyforge_128.png");
    if (!logo.isNull()) {
        p.drawPixmap(176, 30, 128, 128, logo);
    }

    // Texte
    p.setPen(QColor("#58a6ff"));
    p.setFont(QFont("Segoe UI", 28, QFont::Bold));
    p.drawText(QRect(0, 170, 480, 40), Qt::AlignCenter, "PyForge IDE");

    p.setFont(QFont("Segoe UI", 11));
    p.setPen(QColor("#7d8590"));
    p.drawText(QRect(0, 210, 480, 30), Qt::AlignCenter, "Version 3.0");

    p.setFont(QFont("Segoe UI", 9));
    p.setPen(QColor("#484f58"));
    p.drawText(QRect(0, 245, 480, 25), Qt::AlignCenter,
               "Editeur Python haute performance");

    QSplashScreen sc(splash, Qt::WindowStaysOnTopHint);
    sc.setWindowIcon(appIcon);
    sc.show();
    app.processEvents();

    PyForge::Theme::instance().applyGitHubDark();
    PyForge::MainWindow win;
    win.setWindowIcon(appIcon);

    for (int i = 1; i < argc; ++i) {
        QString path = QString::fromLocal8Bit(argv[i]);
        if (QFile::exists(path)) {
            win.openFileAt(path);
        }
    }

    QTimer::singleShot(1500, &sc, [&sc, &win] {
        sc.finish(&win);
        win.show();
    });

    return app.exec();
}