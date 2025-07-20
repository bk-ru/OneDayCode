#include "socket.h"

#include <QDebug>
#include <QDataStream>
#include <QHostAddress>
#include <QByteArray>

Socket::Socket(QObject *parent, bool debug)
    : QObject(parent),
      m_socket(new QTcpSocket(this)), m_debug(debug)
{
    connect(m_socket, &QTcpSocket::connected, this, &Socket::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &Socket::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Socket::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &Socket::onErrorOccurred);
}

Socket *Socket::instance(QObject *parent, bool debug) {
    static Socket *instance = nullptr;
    if (instance == nullptr) {
        instance = new Socket(parent, debug);
    }
    return instance;
}

Socket::~Socket()
{
    if (m_socket->isOpen()) {
        m_socket->close();
    }
}

void Socket::connectToHost(const QString &host, quint16 port)
{
    if (m_socket->isOpen()) {
        m_socket->abort();
    }

    m_socket->connectToHost(host, port);

    QTimer::singleShot(100, [this, host, port]() {
        if (isConnected()) {
            QString statusMessage{"Успешное соединение : " + host + ":" + QString::number(port)};
            emit signalReloadConStatus(statusMessage);
        }
    });
}

void Socket::disconnectFromHost()
{
    if (m_socket->isOpen()) {
        m_socket->disconnectFromHost();
    }
}

void Socket::sendMessage(const QString &message, const QUuid& userId)
{
    if (!isConnected()) {
        emit signalErrorOccurred("Невозможно отправить: сокет не подключён.");
        return;
    }

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);

    quint32 messageType = 1;  // Тип сообщения (текстовая строка)
    QByteArray messageData = message.toUtf8();

    out << messageType;
    out << userId;
    out << quint32(messageData.size());
    out << messageData;

    m_socket->write(packet);

    if (m_debug) {
        qDebug() << "Отправлено сообщение: " << message;
    }
}

void Socket::sendMessage(const QUuid &userId)
{
    if (!isConnected()) {
        emit signalErrorOccurred("Невозможно отправить: сокет не подключён.");
        return;
    }

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);

    out << quint32(0); // тип сообщения
    out << userId;
    out << quint32(0); // размер передаваемых данных (не считая заголовка)

    m_socket->write(packet);

    if (m_debug) {
        qDebug() << "Отправлено сообщение авторизации";
    }
}

void Socket::sendMessage(const QByteArray &data)
{
    if (isConnected()) {
        m_socket->write(data);
    }
    else
        emit signalErrorOccurred("Невозможно отправить: сокет не подключён.");

    if (m_debug) {
        qDebug() << "Отправлены данные размером: " << data.size();
    }
}

void Socket::onReadyRead()
{
    m_buffer.append(m_socket->readAll());
    QDataStream in(&m_buffer, QIODevice::ReadOnly);

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
                if (m_debug)
                    qDebug() << "Не удалось авторизоваться";
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

bool Socket::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void Socket::onConnected()
{
    if (m_debug) {
        qDebug() << "Подключено к серверу";
    }
    sendMessage(m_myId);
    emit signalConnected();
}

void Socket::onDisconnected()
{
    if (m_debug) {
        qDebug() << "Отключение от сервера";
    }
    emit signalDisconnected();
    QString statusMessage{"Нет соединения"};
    emit signalReloadConStatus(statusMessage);
}

void Socket::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errorMsg = m_socket->errorString();
    if (m_debug) {
        qWarning() << "Ошибка сокета:" << errorMsg;
    }
    emit signalErrorOccurred(errorMsg);
}

void Socket::setUserId(const QUuid &id)
{
    m_myId = id;
}
