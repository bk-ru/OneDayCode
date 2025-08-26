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
    if (m_socket->isOpen())
        m_socket->disconnectFromHost();
}

void Socket::sendMessage(ClientMessageType type, const QUuid& userId, const QByteArray& data)
{
    if (!isConnected()) {
        emit signalErrorOccurred("Сокет не подключен");
        return;
    }

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << static_cast<quint16>(type);
    out << userId;
    out << static_cast<quint32>(data.size());
    out.writeRawData(data.constData(), data.size());

    m_socket->write(packet);

    if (m_debug) {
        qDebug() << "Отправлено сообщение типа" << static_cast<int>(type)
                 << "размером" << packet.size() << "байт";
        qDebug() << "Hex:" << packet.toHex();
    }
}

// Для текстовых сообщений
void Socket::sendTextMessage(const QString& message, const QUuid& userId)
{
    sendMessage(ClientMessageType::TEXT, userId, message.toUtf8());
}

// Для авторизации
void Socket::sendAuthMessage(const QUuid& userId)
{
    sendMessage(ClientMessageType::AUTH, userId);
}

// Для бинарных данных (файлы, изображения)
void Socket::sendBinaryData(ClientMessageType type, const QUuid& userId, const QByteArray& data)
{
    Q_ASSERT(type == ClientMessageType::FILE || type == ClientMessageType::IMAGE);
    sendMessage(type, userId, data);
}

void Socket::onReadyRead()
{
    QByteArray newData = m_socket->readAll();
    m_buffer.append(newData);

    if (m_debug) {
        qDebug() << "=== Socket::onReadyRead ===";
        qDebug() << "Новые данные:" << newData.size() << "байт";
        qDebug() << "Общий буфер:" << m_buffer.size() << "байт";
        qDebug() << "Hex буфера:" << m_buffer.toHex();
    }

    while (true) {
        quint32 expectedHeaderSize = sizeof(quint16) + sizeof(QUuid) + sizeof(quint32);

        if (m_debug) {
            qDebug() << "--- Начало цикла обработки ---";
            qDebug() << "Ожидаемый размер заголовка:" << expectedHeaderSize;
            qDebug() << "Текущий размер буфера:" << m_buffer.size();
        }

        if (m_buffer.size() < expectedHeaderSize) {
            if (m_debug) {
                qDebug() << "Недостаточно данных для заголовка (" << m_buffer.size()
                         << " < " << expectedHeaderSize << "), ждем больше данных";
            }
            break;
        }

        // Создаем новый QDataStream для каждого чтения!
        QDataStream in(&m_buffer, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_5_15); // Добавьте эту строку
        quint16 messageType;
        QUuid userId;
        quint32 messageSize;

        in >> messageType >> userId >> messageSize;

        // Добавьте проверку на разумные значения
        if (messageSize > 10 * 1024 * 1024) { // 10MB максимум
            qWarning() << "Подозрительно большой размер сообщения:" << messageSize;
            m_buffer.clear();
            break;
        }

        m_expectedMsg.messageType = messageType;
        m_expectedMsg.userId = userId;
        m_expectedMsg.messageSize = messageSize;

        if (m_debug) {
            qDebug() << "Разобран заголовок:";
            qDebug() << "  Тип сообщения:" << messageType;
            qDebug() << "  ID пользователя:" << userId.toString();
            qDebug() << "  Размер данных:" << messageSize;
        }

        quint32 fullMessageSize = expectedHeaderSize + m_expectedMsg.messageSize;

        if (m_debug) {
            qDebug() << "Ожидаемый полный размер сообщения:" << fullMessageSize;
            qDebug() << "Текущий размер буфера:" << m_buffer.size();
        }

        if (m_buffer.size() < fullMessageSize) {
            if (m_debug) {
                qDebug() << "Недостаточно данных для полного сообщения (" << m_buffer.size()
                         << " < " << fullMessageSize << "), ждем больше данных";
            }
            break;
        }

        // Извлекаем данные сообщения
        QByteArray messageData = m_buffer.mid(expectedHeaderSize, m_expectedMsg.messageSize);

        if (m_debug) {
            qDebug() << "Извлечены данные сообщения:" << messageData.size() << "байт";
            qDebug() << "Содержимое:" << QString::fromUtf8(messageData);
        }

        switch (m_expectedMsg.messageType) {
            case 0: // AUTH
                if (m_debug)
                    qDebug() << "Обработка AUTH сообщения от" << m_expectedMsg.userId;
                if (m_expectedMsg.userId.isNull()) {
                    if (m_debug)
                        qDebug() << "Не удалось авторизоваться";
                    emit signalErrorOccurred("Авторизация не удалась");
                } else {
                    if (m_debug)
                        qDebug() << "Успешная авторизация с ID:" << m_expectedMsg.userId.toString();
                    emit signalAuthSuccess(m_expectedMsg.userId);
                }
                break;

            case 1: // TEXT
                if (m_debug) {
                    qDebug() << "=== TEXT MESSAGE ===";
                    qDebug() << "Отправитель:" << m_expectedMsg.userId.toString();
                    qDebug() << "Сообщение:" << QString::fromUtf8(messageData);
                    qDebug() << "Размер сообщения:" << messageData.size();
                }
                emit signalMessageReceived(m_expectedMsg.userId, QString::fromUtf8(messageData));
                break;

            default:
                qWarning() << "Неизвестный тип сообщения:" << m_expectedMsg.messageType;
                qWarning() << "Данные:" << messageData.toHex();
                break;
        }

        // Удаляем обработанное сообщение из буфера ПОЛНОСТЬЮ
        m_buffer = m_buffer.mid(fullMessageSize);
        if (m_debug) {
            qDebug() << "Буфер после удаления сообщения:" << m_buffer.size() << "байт";
        }

        // Сбрасываем ожидаемый размер для следующего сообщения
        m_expectedMsg.messageSize = 0;

        // Если буфер пуст или меньше заголовка, выходим из цикла
        if (m_buffer.isEmpty() || m_buffer.size() < expectedHeaderSize) {
            if (m_debug) {
                qDebug() << "Буфер пуст или недостаточно данных для следующего заголовка";
            }
            break;
        }

        if (m_debug) {
            qDebug() << "Продолжаем обработку, оставшийся буфер:" << m_buffer.size() << "байт";
        }
    }

    if (m_debug) {
        qDebug() << "=== Конец Socket::onReadyRead ===";
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
    sendAuthMessage(m_myId);
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

void Socket::updateUserId(const QUuid &newId)
{
    QUuid oldId = m_myId;
    m_myId = newId;

    if (isConnected()) {
        sendAuthMessage(m_myId);
        if (m_debug) {
            qDebug() << "ID обновлен с" << oldId.toString() << "на" << newId.toString();
        }
    }
}
