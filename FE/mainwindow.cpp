#include "mainwindow.h"
#include "gameareaselector.h"
#include <iostream>


// Constructor: initializes the UI and connects the Start button
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this); // Sets up the UI from the .ui file
    //Connection to the start of the bot
    connect(ui->pbStart, &QPushButton::clicked, this, &MainWindow::onStartBoT); // When the "Start" button is clicked, call onStartBoT()


    GameAreaSelector selector("Dark Orbit");
    selector.selectAreas();
    RECT playable = selector.getPlayableArea();
    RECT map      = selector.getMapArea();


}

// Destructor: cleans up thread and UI
MainWindow::~MainWindow()
{
    delete ui; // Delete the UI elements

    // If the StateMachine thread is running, ask it to quit and wait for it to finish
    if (m_SMThread && m_SMThread->isRunning()) {
        m_SMThread->quit();   // Gracefully stop the thread's event loop
        m_SMThread->wait();   // Block until the thread has finished
    }

    // m_SM is deleted by deleteLater() when the thread finishes
    // m_SMThread is deleted automatically because it has MainWindow as parent
}

// Slot called when Start button is pressed
void MainWindow::onStartBoT()
{

    // If a previous thread exists, clean it up safely
    if (m_SMThread) {
        m_SMThread->quit();   // Request the thread to exit its event loop
        m_SMThread->wait();   // Block until the thread has fully stopped
        delete m_SMThread;    // Free the QThread memory
        m_SMThread = nullptr; // Clear the pointer
        m_SM = nullptr;       // Also clear the StateMachine pointer
    }

    // Create a new QThread to run the StateMachine
    m_SMThread = new QThread(this);        // Thread with MainWindow as parent
    m_SM = new StateMachine();             // Create the bot logic (no parent!)

    // Move the StateMachine to the background thread
    m_SM->moveToThread(m_SMThread);

    // When the thread finishes, safely delete the StateMachine
    connect(m_SMThread, &QThread::finished, m_SM, &QObject::deleteLater);
    connect(m_SM, &StateMachine::UIlogMessage,this, &::MainWindow::onUIlogMessage);
    // Start the background thread (this triggers its event loop and any connected slots)
    m_SMThread->start();

}

// Key event handler: listens for key presses
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // If 'S' is pressed and the bot exists, call stopExecution() on the bot
    if (event->key() == Qt::Key_S && m_SM) {
        onUIlogMessage("Key 'S' pressed — stopping StateMachine.");
        // Use invokeMethod to ensure thread-safe call (queued to run in StateMachine's thread)
        QMetaObject::invokeMethod(m_SM, "stopExecution", Qt::QueuedConnection);
    }

    // Pass any unhandled keys to the base class (standard behavior)
    QMainWindow::keyPressEvent(event);
}
