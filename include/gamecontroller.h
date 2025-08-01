#pragma once
#include <QObject>
#include <QTimer>
#include <QRect>

class GameController : public QObject {
    Q_OBJECT

public:
    GameController(QObject *parent = nullptr);

    void setTargetArea(const QRect &area);

private slots:
    void tick();

private:
    QTimer *timer;
    QRect moveArea;
    QPoint currentPos;
    int dx = 10;
    int dy = 10;

    void moveMouse(const QPoint &pos);
};
