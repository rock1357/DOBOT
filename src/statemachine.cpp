#include "statemachine.h"
#include <QDateTime>
#include <QDir>

// Constructor: initializes logging and calls the run() function
StateMachine::StateMachine(QObject *parent)
    : QObject{parent}
{
    // Set the filename and path explicitly
    m_logFile.setFileName(QDir::homePath() + "/OneDrive/Desktop/Qt log/Exercise/bot_log.txt");
    // Attempt to open the log file in append mode
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
        QTimer::singleShot(0, this, [this]() {
            emit UIlogMessage("log file opened.");
        });
    } else {
        qWarning("Could not open log file.");
        QTimer::singleShot(0, this, [this]() {
            log("Could not open log file.");
        });

    }

    // Automatically call run() when the object is created. Single shot is used in the constructor.
    QTimer::singleShot(0, this, &StateMachine::run);
}


// run(): Starts the timer and begins the state machine operation
void StateMachine::run()
{
    // Create the timer to schedule transition() calls
    m_startTimer = new QTimer(this); //QTimer has 'this' as parent, so it will be deleted when the state machine will be.

    // Connect the timer timeout to the transition() slot
    connect(m_startTimer, &QTimer::timeout, this, &StateMachine::transition);

    // Start the timer to call transition() every 100 milliseconds (0.1 second)
    m_startTimer->start(100);  // Adjust the interval as needed

    // Initial logging and state
    log("Bot started. Entering run loop...");

}

// transition(): Advances the bot's behavior based on current state
void StateMachine::transition()
{
    switch (currentState) {
    case Idle:
        // Example transition: from Idle → Searching
        log("Bot is in Idle state");

        break;

        // Placeholder for future state transitions (currently commented out)
        /*
    case Searching:
        emit startBot();                    // Start movement logic in GameController
        emit changeStatus("Attacking...");
        currentState = Attacking;
        break;

    case Attacking:
        emit changeStatus("Collecting...");
        currentState = Collecting;
        break;

    case Collecting:
        emit stopBot();                     // Stop the bot's movement
        emit changeStatus("Done.");
        currentState = Done;
        break;
    */

    case Done:
        m_startTimer->stop();  // Stop the timer when done
        break;

    default:
        break;
    }
}

// stopExecution(): Called externally (e.g., via key press) to stop the bot
void StateMachine::stopExecution()
{
    // Stop the periodic transition timer
    if (m_startTimer)
        m_startTimer->stop();

    // Reset state to Idle
    currentState = Idle;

    // Optional: emit a signal to GUI or other component
    log("Bot stopped by user.");
}


// log(): Logs a message to both file and console, with a timestamp
void StateMachine::log(const QString &message)
{
    // Format message with timestamp
    QString timestamped = QDateTime::currentDateTime()
                              .toString("yyyy-MM-dd hh:mm:ss.zzz") + " | " + message;

    // Write to log file if open
    if (m_logFile.isOpen()) {
        m_logStream << timestamped << '\n';
        m_logStream.flush();
    }

    // Optionally forward the message to the GUI log display
    emit UIlogMessage(timestamped);
}

