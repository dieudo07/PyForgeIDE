#include "LSPClient.hpp"

namespace PyForge {

LSPClient::LSPClient(QObject* parent) : QObject(parent) {}

bool LSPClient::start(const QString& pythonPath) {
    Q_UNUSED(pythonPath)
    emit serverStarted();
    return true;
}

void LSPClient::stop() {
    emit serverStopped();
}

bool LSPClient::isRunning() const {
    return false;
}

} // namespace PyForge
#include "LSPClient.moc"