#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QUuid>

#include <functional>
#include <unordered_map>

#include "clienthandler.h"


namespace std {
    template<>
    struct hash<QUuid> {
        std::size_t operator()(const QUuid& uuid) const {
            return qHash(uuid); // Используем встроенную Qt-хеш-функцию
        }
    };
}


class Server : public QTcpServer
{
    Q_OBJECT
public:
    static Server* instance(QObject *parent = nullptr, bool debug = false) {
        static Server *instance = nullptr;
        if (instance == nullptr) {
            instance = new Server(parent, debug);
        }
        return instance;
    }

    ~Server() override;

private:
    explicit Server(QObject *parent = nullptr, bool debug = false);
    Q_DISABLE_COPY(Server)

public:
    bool start(const QString &host, quint16 port);
    bool stop();

public:
    bool isRunning();

signals:
    void signalReloadConStatus(const QString& status);

signals:
    void signalClientConnected(const QUuid& clientId);
    void signalClientDisconnected(const QUuid& clientId);
    void signalErrorOccurred(const QString &error);

protected:
    void incomingConnection(qintptr handle) override;

private slots:
    void onClientDisconnected(const QUuid& clientId);
    void onClientMessageReceived(const QUuid& clientId, const QString &message);
    void onClientAuthoriz(const QUuid& currentId, const QUuid& clientId);

private:
    std::unordered_map<QUuid, std::unique_ptr<ClientHandler>> m_clients;
    QHash<qintptr, ClientHandler*> _clients;
    bool m_isRunning{false};
    bool m_debug{true};
};

#endif // SERVER_H
