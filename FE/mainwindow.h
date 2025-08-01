#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "FE/ui_mainwindow.h"
#include "statemachine.h"
#include <QThread>

// Forward declaration of the UI namespace generated by Qt Designer
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// MainWindow class declaration
class MainWindow : public QMainWindow
{
    Q_OBJECT  // Enables Qt's signal and slot system

public:
    // Constructor: initializes the window
    MainWindow(QWidget *parent = nullptr);

    // Destructor: cleans up allocated resources (thread, UI, etc.)
    ~MainWindow();

private slots:
    // Slot connected to the "Start" button click
    void onStartBoT();
    void onUIlogMessage(const QString &logmex){ui->TEShowLog->append(logmex+'\n');};

protected:
    // Reimplementation of the key press event handler
    // Used to handle user input (e.g., stopping the bot with a key press)
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::MainWindow *ui;       // Pointer to the UI (auto-generated by Qt Designer)

    StateMachine* m_SM = nullptr;      // Pointer to the bot state machine
    QThread* m_SMThread = nullptr;     // Pointer to the thread running the bot
};

#endif // MAINWINDOW_H
