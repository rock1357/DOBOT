#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QMessageBox"
#include <QKeyEvent>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->Start, &QPushButton::clicked, this, &MainWindow::onStartBoT);

}

MainWindow::~MainWindow()
{
    delete ui;
    if (m_SMThread && m_SMThread->isRunning()) {
        m_SMThread->quit();      // Ask the event loop to exit
        m_SMThread->wait();      // Block until it’s done
    }

    delete m_SM;        // Or connect it to the thread's finished() signal
    // m_SMThread will be auto-deleted because of parent relationship
}

void MainWindow::onStartBoT()
{
    // You can replace this with whatever functionality you want
    QMessageBox::information(this, "Button Clicked", "starting of the BOT!");
    m_SMThread=new QThread(this);
    m_SM=new StateMachine();
    m_SM->moveToThread(m_SMThread);
    connect(m_SMThread, &QThread::finished, m_SM, &QObject::deleteLater);
    m_SMThread->start();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_S && m_SM) {
        qDebug() << "Key 'S' pressed — stopping StateMachine.";
        QMetaObject::invokeMethod(m_SM, "stopExecution", Qt::QueuedConnection);
    }

    QMainWindow::keyPressEvent(event); // Pass to base class
}

