#ifndef NETWORK_H
#define NETWORK_H

#include <QDialog>
#include "socket.h"

namespace Ui {
class Network;
}

class Network : public QDialog
{
    Q_OBJECT

public:
    explicit Network(QWidget *parent = nullptr);
    ~Network();

public:
    void setHost(const QString& host);
    void setPort(quint16 port);

signals:
    void connectRequested(const QString& host, quint16 port);
    void disconnected();

private slots:
    void on_connect_clicked();
    void on_disconnect_clicked();

private:
    Ui::Network *ui;
    Socket *m_socket{nullptr};
};

#endif // NETWORK_H
