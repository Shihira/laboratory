/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include <QtWidgets>

#include <rfb/rfbclient.h>

#include "build/ui_MainWindow.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

protected:
    //// form logic
    Ui::MainWindow form;
    QTimer* cursorPulse;
    QVariantAnimation* wheelMotion;

    // flags
    unsigned buttonState;
    bool singleShift;

    QPoint getVeencyCursor();

    //// rfb protocol stuff
    rfbClient* cl;

    static char* getPassword(rfbClient* cl);

    //// self-event handler
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void wheelEvent(QWheelEvent* event);

protected slots:
    void triggeredGoConnect();
    void triggeredGoSwitch_Keyboard();
    void sendCursorPos(QPoint p = QPoint(-1, -1), unsigned mask = ~0U);
    void sendCursorClick(QPoint p = QPoint(-1, -1));
    void sendWheelMotion(const QVariant& var);
    void finishedMotion();
};

