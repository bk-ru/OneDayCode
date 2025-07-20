#include "clienthandler.h"
#include <QDataStream>

ClientHandler::ClientHandler(QTcpSocket *socket, QObject *parent, bool debug)
    : QObject(parent), m_socket(socket), m_debug(debug)
{
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::onDisconnected);
}

ClientHandler::~ClientHandler()
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

const QUuid& ClientHandler::getId() const
{
    return m_id;
}

void ClientHandler::setId(const QUuid &id)
{
    m_id = id;
}

bool ClientHandler::isValid() const
{
    return m_socket && m_socket->state() == QTcpSocket::ConnectedState;
}

void ClientHandler::sendData(const QByteArray &data)
{
    if (isValid()) {
        m_socket->write(data);
        if (m_debug)
            qDebug() << "Отправлено клиенту" << m_id << ":" << data;
    }
}

void ClientHandler::sendDontAuthorize()
{
    if (!isConnected())
        return;

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);

    out << quint32(0);
    out << m_expectedMsg.userId;
    out << 0;

    m_socket->write(packet);

    if (m_debug) {
        qDebug() << "Отправлено сообщение авторизации для:" << m_expectedMsg.userId;
    }
}

void ClientHandler::onReadyRead()
{
    m_buffer.append(m_socket->readAll());
    QDataStream in(&m_buffer, QIODevice::ReadOnly);
    qDebug() << "Пришло:" << m_buffer.size() << m_expectedMsg.messageSize;

    while (true) {
        if (m_expectedMsg.messageSize == 0) {
            if (m_buffer.size() < sizeof(quint32) * 2 + sizeof(QUuid))
                return;  // Ждем, пока не придет достаточно данных для заголовка

            in >> m_expectedMsg.messageType >> m_expectedMsg.userId >> m_expectedMsg.messageSize;
        }

        if (m_buffer.size() < sizeof(quint32) * 2 + sizeof(QUuid) + m_expectedMsg.messageSize)
            return;  // Ждем, пока не придет все сообщение

        QByteArray messageData;
        messageData.resize(m_expectedMsg.messageSize);
        in.readRawData(messageData.data(), m_expectedMsg.messageSize);

        switch (m_expectedMsg.messageType) {
            case 0:
                emit signalAuthorize(m_id, m_expectedMsg.userId);
                m_id = m_expectedMsg.userId;
                break;
            case 1:
                emit signalMessageReceived(m_expectedMsg.userId, QString::fromUtf8(messageData));  // Отправляем вместе с UUID
                // Обработка изображения
                break;
            case 2:
                // Обработка файла
                break;
            case 3:
                // Обработка файла
                break;
            default:
                qWarning() << "Неизвестный тип сообщения";
                break;
        }

        m_buffer = m_buffer.mid(sizeof(quint32) * 2 + sizeof(QUuid) + m_expectedMsg.messageSize);
        m_expectedMsg.messageSize = 0;
    }
}

bool ClientHandler::isConnected()
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void ClientHandler::onDisconnected()
{
    emit disconnected(m_id);
}
