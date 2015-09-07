#include <sstream>

#include <QtWidgets>
#include <QtSerialPort>

#include "ui_main_window.h"

class MainWindow : public QWidget {
        Q_OBJECT

protected:
        Ui_Form form;
        QSerialPort* serial = nullptr;
        std::stringstream serial_stream;

public:
        MainWindow(QWidget* parent = nullptr) : QWidget(parent) {
                form.setupUi(this);

                refresh_serial();
                
                connect(form.btn_watch, SIGNAL(clicked()),
                        this, SLOT(watch_serial()));
                connect(form.btn_refresh, SIGNAL(clicked()),
                        this, SLOT(refresh_serial()));
        }

private slots:
        void refresh_serial();
        void watch_serial();
        void data_arrived();
};

