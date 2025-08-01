#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "statemachine.h"
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartBoT();

protected:
void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::MainWindow *ui;
    StateMachine* m_SM=nullptr;
    QThread* m_SMThread=nullptr;



};
#endif // MAINWINDOW_H
