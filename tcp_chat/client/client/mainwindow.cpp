#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_config(new Config(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Chat (Client)");

    m_socket = Socket::instance(parent, m_debug);
    m_socket->setUserId(m_config->getId());

    QObject::connect(m_socket, &Socket::signalReloadConStatus, this, &MainWindow::reloadConStatus);
//    m_socket->connectToHost("127.0.0.1", 12345);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::reloadConStatus(const QString &status)
{
    ui->networkStatus->setText(status);
}

void MainWindow::on_network_clicked()
{
    ensureDialogVisible(m_network, this);
    if (m_network) {
        m_network->setHost(m_config->getHost());
        m_network->setPort(m_config->getPort());
    }
}


void MainWindow::on_send_clicked()
{

}

void MainWindow::on_history_clicked()
{

}

template<typename DialogType>
void MainWindow::ensureDialogVisible(QPointer<DialogType> &dialog, QWidget *parent)
{
    if (dialog && dialog->isVisible()) {
        dialog->raise();
        dialog->activateWindow();
    } else {
        dialog = new DialogType(parent);
        dialog->show();
    }
}
