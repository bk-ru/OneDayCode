#include "server.h"

#include <QHostAddress>
#include <QDebug>
#include <memory>

Server::Server(QObject *parent, bool debug)
    : QTcpServer(parent), m_debug(debug)
{
}

Server::~Server()
{
    stop();
}

bool Server::start(const QString &host, quint16 port)
{
    if (!listen(QHostAddress(host), port)) {
        emit signalErrorOccurred(errorString());
        m_isRunning = false;
        return false;
    }

    if (m_debug)
        qDebug() << "Сервер запущен на" << host << ":" << port;

    QString statusMessage{"Сервер слушает на : " + host + ":" + QString::number(port)};
    emit signalReloadConStatus(statusMessage);
    m_isRunning = true;
    return true;
}

bool Server::stop()
{
    if (!m_isRunning) {
       if (m_debug)
           qDebug() << "Сервер уже остановлен";
       return false;
    }

    if (this->isListening())
        close();

    m_clients.clear();
    m_isRunning = false;

    if (m_debug)
        qDebug() << "Сервер остановлен";

    QString statusMessage{"Сервер остановлен"};
    emit signalReloadConStatus(statusMessage);

    return true;
}

void Server::incomingConnection(qintptr handle)
{
    if (m_debug)
        qDebug() << "Новое подключение:" << handle;

    QTcpSocket *socket = new QTcpSocket(this);
    if (!socket->setSocketDescriptor(handle)) {
        if (m_debug)
            qDebug() << "Ошибка создания сокета:" << socket->errorString();
        delete socket;
        return;
    }

    std::unique_ptr<ClientHandler> handler(new ClientHandler(socket, this, m_debug));

    QObject::connect(handler.get(), &ClientHandler::disconnected, this, &Server::onClientDisconnected);
    QObject::connect(handler.get(), &ClientHandler::signalMessageReceived, this, &Server::onClientMessageReceived);
    QObject::connect(handler.get(), &ClientHandler::signalAuthorize, this, &Server::onClientAuthoriz);

    m_clients[handler->getId()] = std::move(handler);
}

void Server::onClientDisconnected(const QUuid& clientId)
{
    if (m_debug)
        qDebug() << "Клиент отключен:" << clientId;

    if (m_clients.find(clientId) != m_clients.end()) {
        m_clients.erase(clientId);
    }
}

void Server::onClientMessageReceived(const QUuid& clientId, const QString &message)
{
    if (m_debug)
        qDebug() << "Сообщение от клиента" << clientId << ":" << message;
}

void Server::onClientAuthoriz(const QUuid& currentId, const QUuid &clientId)
{
    ClientHandler* client = qobject_cast<ClientHandler*>(sender());
    if (m_clients.find(clientId) != m_clients.end()) {
        client->sendDontAuthorize();
    }
    else {
        auto it = m_clients.find(currentId);
        std::unique_ptr<ClientHandler> handler = std::move(it->second);
        m_clients.erase(it);

        client->setId(clientId);
        m_clients[clientId] = std::move(handler);
    }
}

bool Server::isRunning()
{
    return m_isRunning;
}
