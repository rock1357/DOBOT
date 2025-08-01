#include "gamecontroller.h"
#include <QCursor>
#include <QGuiApplication>

GameController::GameController(QObject *parent)
    : QObject{parent},timer(new QTimer(this))
{    connect(timer, &QTimer::timeout, this, &GameController::tick);
    timer->start(100); // every 100ms
}

void GameController::setTargetArea(const QRect &area)
{
    moveArea = area;
    currentPos = area.topLeft();
}


void GameController::tick()
{
    // Move diagonally inside the area
    currentPos += QPoint(dx, dy);

    if (!moveArea.contains(currentPos)) {
        // Bounce
        if (currentPos.x() < moveArea.left() || currentPos.x() > moveArea.right()) dx = -dx;
        if (currentPos.y() < moveArea.top() || currentPos.y() > moveArea.bottom()) dy = -dy;
        currentPos += QPoint(dx, dy);
    }

}
