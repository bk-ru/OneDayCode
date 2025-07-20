#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QUuid>

struct MessagePacket {
    quint32 messageType;  // Тип сообщения
    QUuid userId;         // Уникальный идентификатор пользователя
    quint32 messageSize{0};  // Размер сообщения (без заголовка)
    QByteArray messageData;  // Данные сообщения
};

class ClientHandler : public QObject
{
    Q_OBJECT
public:
    explicit ClientHandler(QTcpSocket *socket, QObject *parent = nullptr, bool debug = false);
    ~ClientHandler();

public:
    const QUuid& getId() const;
    void setId(const QUuid& id);

public:
    bool isValid() const;
    void sendData(const QByteArray &data);

private:
    bool isConnected();

signals:
    void disconnected(const QUuid& clientId);
    void signalMessageReceived(const QUuid& userId, const QString &message);
    void signalAuthorize(const QUuid& currentId, const QUuid& userId);

private slots:
    void onReadyRead();
    void onDisconnected();

public:
    void sendDontAuthorize();

private:
    MessagePacket m_expectedMsg;
    QByteArray m_buffer{QByteArray()};
    QTcpSocket *m_socket{nullptr};
    QUuid m_id{QUuid::createUuid()};
    bool m_debug{false};
};

#endif // CLIENTHANDLER_H
