#include "calchandler.h"
#include <cmath>
#include <QDebug>

CalcHandler::CalcHandler(QObject *parent)
    : QObject(parent)
{
    clear();
}

void CalcHandler::handleDigit(int digit) {
    if (errorState) return;

    if (waitingForOperand) {
        currentValue = 0;
        hasDecimalPoint = false;
        decimalFactor = 0.1;
        waitingForOperand = false;
    }

    if (hasDecimalPoint) {
        currentValue = currentValue + digit * decimalFactor;
        decimalFactor *= 0.1;
    } else {
        currentValue = currentValue * 10 + digit;
    }

    emit displayChanged(currentDisplay());
}

void CalcHandler::setOperation(const QString &op) {
    if (errorState) return;

    if (!waitingForOperand) {
        // Если есть предыдущая операция - вычисляем
        if (!currentOperation.isEmpty()) {
            performCalculation();
            storedValue = currentValue;
        } else {
            storedValue = currentValue;
        }
        emit displayChanged(currentDisplay());
    }

    currentOperation = op;
    pendingOperation.clear();
    waitingForOperand = true;
    hasDecimalPoint = false;
}

void CalcHandler::calculate() {
    if (errorState || currentOperation.isEmpty()) return;

    if (!waitingForOperand) {
        performCalculation();
    } else if (!pendingOperation.isEmpty()) {
        // Для цепочки операций: 5 + = = (5+5=10, 10+5=15)
        currentOperation = pendingOperation;
        performCalculation();
    }

    pendingOperation = currentOperation;
    currentOperation.clear();
    waitingForOperand = true;
    hasDecimalPoint = false;

    emit displayChanged(currentDisplay());
}

void CalcHandler::addDecimalPoint() {
    if (errorState) return;

    if (waitingForOperand) {
        currentValue = 0;
        waitingForOperand = false;
    }

    if (!hasDecimalPoint) {
        hasDecimalPoint = true;
        decimalFactor = 0.1;
        emit displayChanged(currentDisplay());
    }
}

void CalcHandler::clear() {
    resetState();
    currentValue = 0.0;
    storedValue = 0.0;
    emit displayChanged(currentDisplay());
}

void CalcHandler::deleteLast() {
    if (errorState) return;

    if (waitingForOperand) return;

    if (hasDecimalPoint) {
        // Отмена последнего десятичного разряда
        double temp = currentValue * 10;
        temp = std::floor(temp + 0.5);
        currentValue = temp * 0.1;

        // Проверка остались ли десятичные
        if (currentValue == std::floor(currentValue)) {
            hasDecimalPoint = false;
            decimalFactor = 0.1;
        }
    } else {
        currentValue = std::floor(currentValue / 10);
    }

    // Если удалили все цифры
    if (currentValue == 0 && !hasDecimalPoint) {
        waitingForOperand = true;
    }

    emit displayChanged(currentDisplay());
}

QString CalcHandler::currentDisplay() const {
    if (errorState) return "Error";

    QString display;
    double value = waitingForOperand ? storedValue : currentValue;

    // Проверка на целое число
    if (value == std::floor(value) && !hasDecimalPoint) {
        display = QString::number(static_cast<qint64>(value));
    } else {
        // Определение количества знаков после запятой
        int precision = 10;
        if (hasDecimalPoint) {
            precision = std::abs(std::log10(decimalFactor * 10));
        }
        display = QString::number(value, 'f', precision);
    }

    // Удаление лишних нулей в конце
    if (display.contains('.')) {
        while (display.endsWith('0')) {
            display.chop(1);
        }
        if (display.endsWith('.')) {
            display.chop(1);
        }
    }

    return display;
}

void CalcHandler::resetState() {
    currentOperation.clear();
    pendingOperation.clear();
    waitingForOperand = true;
    hasDecimalPoint = false;
    decimalFactor = 0.1;
    errorState = false;
}

void CalcHandler::performCalculation() {
    double result = storedValue;

    if (currentOperation == "+") {
        result += currentValue;
    } else if (currentOperation == "-") {
        result -= currentValue;
    } else if (currentOperation == "×") {
        result *= currentValue;
    } else if (currentOperation == "÷") {
        if (currentValue == 0) {
            errorState = true;
            emit errorOccurred("Division by zero");
            return;
        }
        result /= currentValue;
    }

    // Проверка на переполнение
    if (std::isinf(result) || std::isnan(result)) {
        errorState = true;
        emit errorOccurred("Arithmetic error");
        return;
    }

    updateCurrentValue(result);
}

void CalcHandler::updateCurrentValue(double value) {
    storedValue = value;
    currentValue = value;
}
