/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include "MainWindow.h"

#include <iostream>

using namespace std;

MainWindow::MainWindow()
    : buttonState(0), singleShift(false), cl(nullptr)
{
    form.setupUi(this);
    cursorPulse = new QTimer(this);
    wheelMotion = new QVariantAnimation(this);

    // signal-slots settings
    connect(form.actionConnect, SIGNAL(triggered()),
            this, SLOT(triggeredGoConnect()));
    connect(form.actionSwitch_Keyboard, SIGNAL(triggered()),
            this, SLOT(triggeredGoSwitch_Keyboard()));
    connect(cursorPulse, SIGNAL(timeout()),
            this, SLOT(sendCursorPos()));
    connect(wheelMotion, SIGNAL(valueChanged(const QVariant&)),
            this, SLOT(sendWheelMotion(const QVariant&)));
    connect(wheelMotion, SIGNAL(finished()),
            this, SLOT(finishedMotion()));

    cursorPulse->start(15);
}

char* MainWindow::getPassword(rfbClient* cl)
{
    static const char* password = "remote/Shihira";
    char* dyn_password = (char*)malloc(sizeof(password));
    strcpy(dyn_password, password);
    return dyn_password;
}

void MainWindow::triggeredGoConnect()
{
    if(!form.actionConnect->isChecked()) {
        if(cl) rfbClientCleanup(cl);
        cl = nullptr;
        return;
    }

    static const char* serverHost = "127.0.0.1";
    char* dyn_serverHost = (char*)malloc(sizeof(serverHost));
    strcpy(dyn_serverHost, serverHost);

    cl = rfbGetClient(8, 3, 4);

    cl->serverHost = dyn_serverHost;
    cl->serverPort = 15900;
    cl->GetPassword = getPassword;

    if(!rfbInitClient(cl, nullptr, nullptr)) {
        cl = nullptr;
        return;
    }

    sendCursorClick(QPoint(0, 0));
}

void MainWindow::triggeredGoSwitch_Keyboard()
{
    if(!cl) return;

    SendKeyEvent(cl, XK_Control_L, true);
    SendKeyEvent(cl, XK_space, true);
    SendKeyEvent(cl, XK_space, false);
    SendKeyEvent(cl, XK_Control_L, false);
}

MainWindow::~MainWindow()
{
    if(cl) rfbClientCleanup(cl);
    cl = nullptr;
}

QPoint MainWindow::getVeencyCursor()
{
    if(!form.actionToggle_VS_Entry->isChecked()) {
        QPoint orgPos = QCursor::pos();
        if(orgPos.x() > 1365 && orgPos.y() < 700)
            orgPos.setX(1365);
        QCursor::setPos(orgPos);
    }

    QPoint p = QCursor::pos();
    QRect rt = form.centralwidget->geometry();

    p = p - geometry().topLeft() - rt.topLeft();
    if(p.x() < 0) p.setX(0);
    if(p.y() < 0) p.setY(0);
    if(p.x() >= rt.width()) p.setX(rt.width());
    if(p.y() >= rt.height()) p.setY(rt.height());


    // Veency bug
    return QPoint(
        (rt.height() - p.y()) / float(rt.height()) * 1536,
        p.x() / float(rt.width()) * 2048
    );
}

void MainWindow::sendCursorPos(QPoint p, unsigned mask)
{
    if(p.x() < 0 && mask == ~0U) {
        p = getVeencyCursor();
        mask = buttonState;
    }

    if(!cl) return;

    SendPointerEvent(cl, p.x(), p.y(), buttonState);
}

void MainWindow::sendCursorClick(QPoint p)
{
    if(!cl) return;

    if(p.x() < 0)
        p = getVeencyCursor();
    SendPointerEvent(cl, p.x(), p.y(), rfbButton1Mask);
    SendPointerEvent(cl, p.x(), p.y(), 0);
}

void MainWindow::sendWheelMotion(const QVariant& var)
{
    if(!cl) return;

    QPoint p = var.toPoint();
    sendCursorPos(p, buttonState);
}

void MainWindow::finishedMotion()
{
    buttonState = 0;
    cursorPulse->start();
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if(!cl) return;

    if(event->button() & Qt::LeftButton)
        buttonState |= rfbButton1Mask;
    /*
    if(event->button() & Qt::RightButton)
        buttonState |= rfbButton3Mask;
    if(event->button() & Qt::MiddleButton)
        buttonState |= rfbButton2Mask;
    */
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    QPoint p = getVeencyCursor();

    form.actionToggle_VS_Entry->setChecked(true);

    if(event->button() & Qt::LeftButton)
        buttonState &= ~rfbButton1Mask;
    /*
    if(event->button() & Qt::RightButton)
        buttonState &= ~rfbButton3Mask;
    if(event->button() & Qt::MiddleButton)
        buttonState &= ~rfbButton2Mask;
    */
    if(event->button() & Qt::RightButton) {
        cursorPulse->stop();
        sendCursorClick(QPoint(910, 1950));
        QTimer::singleShot(500, [this]()
            { sendCursorClick(QPoint(567, 1024)); finishedMotion(); });
    }
    if(event->button() & Qt::MiddleButton) {
        cursorPulse->stop();
        sendCursorClick(QPoint(910, 1950));
        QTimer::singleShot(500, [this]()
            { sendCursorClick(QPoint(625, 1248)); finishedMotion(); });
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if(!cl) return;

    SendKeyEvent(cl, event->nativeVirtualKey(), true);

    if(event->key() == Qt::Key_Shift)
        singleShift = true;
    else singleShift = false;
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if(!cl) return;

    SendKeyEvent(cl, event->nativeVirtualKey(), false);

    if(event->key() == Qt::Key_Shift && singleShift) {
        triggeredGoSwitch_Keyboard();
    }
}

void MainWindow::wheelEvent(QWheelEvent* event)
{
    if(wheelMotion->state() == QVariantAnimation::Running)
        return;

    cursorPulse->stop();
    QPoint p = getVeencyCursor();

    buttonState = ~0UL & rfbButton1Mask;

    wheelMotion->setDuration(200);
    wheelMotion->setStartValue(p + QPoint(event->angleDelta().y(), 0));
    wheelMotion->setEndValue(p - QPoint(event->angleDelta().y(), 0));
    wheelMotion->setEasingCurve(QEasingCurve::Linear);
    wheelMotion->start();
    event->angleDelta();
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MainWindow mainWindow;

    mainWindow.show();
    app.exec();
}

