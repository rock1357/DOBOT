#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <QTimer>
#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>

// The StateMachine class controls the bot's behavior and state transitions.
// It runs in its own thread and uses a QTimer for periodic logic execution.
class StateMachine : public QObject
{
    Q_OBJECT  // Enables signals and slots

public:
    // Constructor: sets up logging and initial state
    explicit StateMachine(QObject *parent = nullptr);

    // Logs a message (to file, console, and optionally UI)
    void log(const QString &message);

public slots:
    // Entry point to start the bot (called from MainWindow via QThread::started)
    void run();

    // Executes one step of the bot's logic, depending on its current state
    void transition();

    // External call to stop the bot (e.g., from UI or key press)
    void stopExecution();

signals:
    // Optional signal to send log messages to the GUI for display
    void UIlogMessage(const QString &message);

protected:
    // Enum to represent the bot's current operational state
    enum BotState {
        Idle,
        Searching,
        Attacking,
        Collecting,
        Done
    };

    BotState currentState;        // Holds the current state of the bot
    QTimer* m_startTimer = nullptr; // Timer used to schedule periodic state transitions
    QFile m_logFile;              // Log file handle
    QTextStream m_logStream;      // Stream to write logs into the file
};

#endif // STATEMACHINE_H
