#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include <QMainWindow>
#include <QPushButton>

#include "calchandler.h"

class CalcHandler;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupConnections();

private slots:
    // Групповые обработчики
    void handleDigitClick();
    void handleOperationClick();

private:
    void connectButtons();
    void updateDisplay();

    Ui::MainWindow *ui;
    CalcHandler *calcHandler; // Логика калькулятора
};
#endif // MAINWINDOW_H
