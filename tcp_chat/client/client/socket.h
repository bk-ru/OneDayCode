#ifndef SOCKET_H
#define SOCKET_H

#include <QTcpSocket>
#include <QObject>
#include <QTimer>
#include <QUuid>

struct MessagePacket {
    quint32 messageType;  // Тип сообщения
    QUuid userId;         // Уникальный идентификатор пользователя
    quint32 messageSize;  // Размер сообщения (без заголовка)
    QByteArray messageData;  // Данные сообщения
};

class Socket : public QObject
{
    Q_OBJECT
public:
    static Socket* instance(QObject *parent = nullptr, bool debug = false);

    ~Socket() override;

private:
    explicit Socket(QObject *parent = nullptr, bool debug = false);
    Q_DISABLE_COPY(Socket)

public:
    void setUserId(const QUuid& id);
public:
    void connectToHost(const QString &host, quint16 port);
    void disconnectFromHost();
    void sendMessage(const QString &message, const QUuid& userId);
    void sendMessage(const QUuid& userId);
    void sendMessage(const QByteArray &data);
    bool isConnected() const;

signals:
    void signalConnected();
    void signalDisconnected();
    void signalErrorOccurred(const QString &error);
    void signalMessageReceived(const QUuid& userId, const QString &message);

signals:
    void signalReloadConStatus(const QString& status);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_socket{nullptr};
    QUuid m_myId{"32513efb-11e7-488d-90ea-9ca6c157b76e"};
    QByteArray m_buffer{QByteArray()};
    MessagePacket m_expectedMsg;
    bool m_debug{false};

};

#endif // SOCKET_H
