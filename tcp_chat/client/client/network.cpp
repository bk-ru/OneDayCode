#include "network.h"
#include "ui_network.h"

#include <QMessageBox>
#include <QDebug>

Network::Network(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Network), m_socket(Socket::instance())
{
    ui->setupUi(this);
    this->setWindowTitle("Network (Client)");
}

Network::~Network()
{
    delete ui;
}

void Network::setHost(const QString &host)
{
    ui->host->setText(host);
}

void Network::setPort(quint16 port)
{
    ui->port->setText(QString::number(port));
}

void Network::on_connect_clicked()
{
    QString host = ui->host->text();
    if (host.isEmpty()) {
        QMessageBox::warning(this, "Инфо", "Поле host должно быть заполненно, пример: 127.0.0.1");
        return;
    }
    bool ok;
    quint16 port = ui->port->text().toUShort(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Инфо", "Порт должен быть числом от 0 до 65535");
        return;
    }

    if (m_socket && m_socket->isConnected()) {
        QMessageBox::information(this, "Инфо", "Вы уже подключены");
        return;
    }

    if (m_socket)
        m_socket->connectToHost(host, port);
    else {
        qDebug() << "socket не инициализирован";
        return;
    }

    QTimer::singleShot(100, [this]() {
        if (m_socket->isConnected()) {
            QMessageBox::information(this, "Инфо", "Успешное подключение");
        } else
            QMessageBox::information(this, "Инфо", "Не удалось подключиться");
    });

}


void Network::on_disconnect_clicked()
{

    if (!m_socket) {
        qDebug() << "socket не инициализирован";
        return;
    }

    if (!m_socket->isConnected()) {
        QMessageBox::information(this, "Инфо", "Вы не подключены");
        return;
    }

    m_socket->disconnectFromHost();

    QMessageBox::information(this, "Инфо", "Успешное отключение");
    emit disconnected();
}
