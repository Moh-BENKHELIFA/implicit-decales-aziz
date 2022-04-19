//
// Created by Loïc Barthe on 03/04/2020.
//

#ifndef TEST_QWIDGETMYWIDGET_H
#define TEST_QWIDGETMYWIDGET_H

#include "QWidget"
#include <QTimer>
#include "../Solver/genericsolver.h"
#include "../Solver/mydecalsolver.h"

class QWidgetMyWidget : public QWidget {

public:

    QWidgetMyWidget();


protected:
    virtual void internal_preupdate_solve() = 0;
    virtual void innerPaintColorDecaleMouseUpdate() = 0;

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    int timerSet=0;
    bool isSolverPrepared = false;

    void timerHandler();
    bool needUpdate;

    QTimer *timer;
};


#endif //TEST_QWIDGETMYWIDGET_H
