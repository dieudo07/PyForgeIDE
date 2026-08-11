#pragma once
#include <QObject>
#include <QString>

namespace PyForge {

class LSPClient : public QObject {
    Q_OBJECT
public:
    explicit LSPClient(QObject* parent = nullptr);
    bool start(const QString& pythonPath = "python");
    void stop();
    bool isRunning() const;

signals:
    void serverStarted();
    void serverStopped();
    void serverError(const QString& message);
};

} // namespace PyForge