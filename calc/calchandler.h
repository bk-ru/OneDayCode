#ifndef CALCHANDLER_H
#define CALCHANDLER_H

#include <QObject>
#include <QString>

class CalcHandler : public QObject
{
    Q_OBJECT
public:
    explicit CalcHandler(QObject *parent = nullptr);

    // Основные операции
    void handleDigit(int digit);
    void setOperation(const QString &op);
    void calculate();
    void addDecimalPoint();
    void clear();
    void deleteLast();

    // Состояние
    QString currentDisplay() const;
    bool hasError() const { return errorState; }

signals:
    void displayChanged(const QString &text);
    void errorOccurred(const QString &message);

private:
    void resetState();
    void performCalculation();
    void updateCurrentValue(double value);

    double currentValue = 0.0;
    double storedValue = 0.0;
    QString currentOperation;
    QString pendingOperation;

    bool waitingForOperand = true;
    bool hasDecimalPoint = false;
    double decimalFactor = 0.1;
    bool errorState = false;
};

#endif // CALCHANDLER_H
