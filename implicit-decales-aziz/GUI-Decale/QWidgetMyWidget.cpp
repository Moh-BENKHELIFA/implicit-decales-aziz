//
// Created by Loïc Barthe on 03/04/2020.
//

#include "QWidgetMyWidget.h"
#include <iostream>
#include <QMouseEvent>


QWidgetMyWidget::QWidgetMyWidget () : timerSet(0), QWidget () {

    timer = new QTimer();
    timer->setInterval(1000/30);
    connect(timer, &QTimer::timeout, this, &QWidgetMyWidget::timerHandler);
    //updating each frame at 30fps
    timer->start();

    setAttribute(Qt::WA_AcceptTouchEvents);
}


void QWidgetMyWidget::timerHandler()
{
    if(isSolverPrepared and needUpdate){
        //std::cout<<"timer"<<std::endl;
        internal_preupdate_solve();
        innerPaintColorDecaleMouseUpdate();
        needUpdate = false;
    }
}
void QWidgetMyWidget::mousePressEvent(QMouseEvent *event) {

    //int xm = event->x();
    //int xm = 0;

    //std::cout << "coucou souris " << xm << std::endl;
    std::cout << "(x,y) = " << event->pos().x() << "  "<< event->pos().y()<<std::endl;
}

void QWidgetMyWidget::mouseMoveEvent(QMouseEvent *event) {

    //int xm = event->x();
    //int xm = 0;

    //std::cout << "coucou souris " << xm << std::endl;
    std::cout << "(x,y) = " << event->pos().x() << "  "<< event->pos().y()<<std::endl;
    repaint();//update() in general, use repaint for animation only
}
