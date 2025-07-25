#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>

#include "network.h"
#include "socket.h"
#include "config.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    template<typename DialogType>
    void ensureDialogVisible(QPointer<DialogType> &dialog, QWidget* parent);

private slots:
    void reloadConStatus(const QString& status);

private slots:
    void on_network_clicked();
    void on_send_clicked();
    void on_history_clicked();

private:
    Ui::MainWindow *ui;
    Socket *m_socket{nullptr};
    Config *m_config{nullptr};
    QPointer<Network> m_network;
    bool m_debug{true};
};
#endif // MAINWINDOW_H
