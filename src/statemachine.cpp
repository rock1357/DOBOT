#include "statemachine.h"
#include <qdebug.h>

StateMachine::StateMachine(QObject *parent)
    : QObject{parent}
{}


void StateMachine::run()
{
    m_startTimer = new QTimer(this);
    connect(m_startTimer, &QTimer::timeout, this, &StateMachine::transition);
    m_startTimer->start(1000);  // adjust interval as needed
}


void StateMachine::transition()
{
    // switch (currentState) {
    // case Idle:
    //     emit changeStatus("Searching...");
    //     currentState = Searching;
    //     break;
    // case Searching:
    //     emit startBot();  // tell GameController to start movement
    //     emit changeStatus("Attacking...");
    //     currentState = Attacking;
    //     break;
    // case Attacking:
    //     emit changeStatus("Collecting...");
    //     currentState = Collecting;
    //     break;
    // case Collecting:
    //     emit stopBot();
    //     emit changeStatus("Done.");
    //     currentState = Done;
    //     break;
    // case Done:
    //     m_startTimer->stop();
    //     break;
    // default:
    //     break;
    // }
}

void StateMachine::stopExecution()
{
    qDebug() << "StateMachine stopping...";
    m_startTimer->stop();
    currentState = Idle;
    //emit changeStatus("Bot stopped by user.");
}
