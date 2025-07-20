#include "network.h"
#include "ui_network.h"

#include <QMessageBox>
#include <QDebug>

Network::Network(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Network),
    m_server(Server::instance()) // Используем Singleton без явного удаления
{
    ui->setupUi(this);
    this->setWindowTitle("Network (Server)");
}

Network::~Network()
{
    delete ui;
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

    // Если сервер уже запущен, информируем пользователя
    if (m_server->isRunning()) {
        QMessageBox::information(this, "Инфо", "Сервер уже запущен");
        return;
    }

    // Попытка запуска сервера
    if (!m_server->start(host, port)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось запустить сервер");
        return;
    }

    QMessageBox::information(this, "Инфо", "Сервер запущен");
}

void Network::on_disconnect_clicked()
{
    if (!m_server->isRunning()) {
        QMessageBox::information(this, "Инфо", "Сервер не запущен");
        return;
    }

    m_server->stop(); // Останавливаем сервер
    QMessageBox::information(this, "Инфо", "Сервер остановлен");

    emit disconnected(); // Генерируем сигнал о завершении работы
}
