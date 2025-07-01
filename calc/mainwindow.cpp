#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QRegularExpression>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , calcHandler(new CalcHandler(this))
{
    ui->setupUi(this);
    setupConnections();

    // Связь сигналов CalcHandler с обновлением UI
    connect(calcHandler, &CalcHandler::displayChanged,
            ui->label_result, &QLabel::setText);
    connect(calcHandler, &CalcHandler::errorOccurred,
            this, [this](const QString& msg) {
                QMessageBox::warning(this, "Error", msg);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections() {
    // Цифровые кнопки (0-9)
    auto digitButtons = findChildren<QPushButton*>(QRegularExpression("pushButton_\\d"));
    for (auto *button : digitButtons) {
        connect(button, &QPushButton::clicked, this, [this, button]() {
            calcHandler->handleDigit(button->text().toInt());
        });
    }

    // Операции
    connect(ui->pushButton_sum, &QPushButton::clicked, this, [this]() {
        calcHandler->setOperation("+");
    });
    connect(ui->pushButton_diff, &QPushButton::clicked, this, [this]() {
        calcHandler->setOperation("-");
    });
    connect(ui->pushButton_mult, &QPushButton::clicked, this, [this]() {
        calcHandler->setOperation("×");
    });
    connect(ui->pushButton_div, &QPushButton::clicked, this, [this]() {
        calcHandler->setOperation("÷");
    });

    // Специальные кнопки
    connect(ui->pushButton_equals, &QPushButton::clicked, this, [this]() {
        calcHandler->calculate();
    });
    connect(ui->pushButton_fraction, &QPushButton::clicked, this, [this]() {
        calcHandler->addDecimalPoint();
    });
    connect(ui->pushButton_C, &QPushButton::clicked, this, [this]() {
        calcHandler->clear();
    });
    connect(ui->pushButton_DEL, &QPushButton::clicked, this, [this]() {
        calcHandler->deleteLast();
    });
}

// Обработчик цифр
void MainWindow::handleDigitClick() {
    if (auto *button = qobject_cast<QPushButton*>(sender())) {
        int digit = button->text().toInt();
        calcHandler->handleDigit(digit);
        updateDisplay();
    }
}

// Обработчик операций
void MainWindow::handleOperationClick() {
    if (auto *button = qobject_cast<QPushButton*>(sender())) {
        QString op = button->text();
        calcHandler->setOperation(op);
        updateDisplay();
    }
}

void MainWindow::updateDisplay() {
    ui->label_result->setText(calcHandler->currentDisplay());
}

