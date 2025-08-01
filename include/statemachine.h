#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <QTimer>
#include <QObject>

class StateMachine : public QObject
{
    Q_OBJECT
public:
    explicit StateMachine(QObject *parent = nullptr);

public slots:
    void run();  // entry point after start button
    void transition();
    void stopExecution();


signals:

protected:
    enum BotState { Idle, Searching, Attacking, Collecting, Done };
    BotState currentState;
    QTimer* m_startTimer=nullptr;
};

#endif // STATEMACHINE_H
