#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_config(new Config(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Chat (Server)");

    m_server = Server::instance(parent, m_debug);

    QObject::connect(m_server, &Server::signalReloadConStatus, this, &MainWindow::reloadConStatus);
    m_server->start(m_config->getHost(), m_config->getPort());
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
