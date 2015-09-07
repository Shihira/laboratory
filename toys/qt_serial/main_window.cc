#include <iostream>
#include <algorithm>
#include <iomanip>
#include <string>

#include <QtWidgets>

#include "main_window.h"

using namespace std;

void MainWindow::refresh_serial() {
        auto info_list = QSerialPortInfo::availablePorts();

        form.port_selector->clear();
        for(QSerialPortInfo& info : info_list)
                form.port_selector->addItem(info.portName());

        form.baudrate_selector->clear();
        QList<int> baud_list = {
                1200  , 2400  , 4800  ,
                9600  , 19200 , 38400 ,
                57600 , 115200,
        };
        for(int baud : baud_list)
                form.baudrate_selector->addItem(QString::number(baud));
        form.baudrate_selector->setCurrentIndex(3 /* 9600 */);

}

void MainWindow::watch_serial() {
        if(serial) {
                if(serial->isOpen()) serial->close();
                serial->deleteLater();
        }
        serial = new QSerialPort();

        serial->setPortName(form.port_selector->currentText());
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        //serial->setBaudRate(QSerialPort::Baud9600);
        serial->setBaudRate(form.baudrate_selector->currentText().toLong());
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        serial->open(QIODevice::ReadWrite);
        if(!serial->isOpen()) 
                form.serial_data->append("Failed to open serial port.");

        connect(serial, SIGNAL(readyRead()),
                this, SLOT(data_arrived()));
}

void MainWindow::data_arrived()
{
        string data = serial->readAll().toStdString();
        serial_stream << data;

        int lines = count(data.begin(), data.end(), '\n');
        for(int i = 0; i < lines; i++) {
                string line;
                getline(serial_stream, line);

                istringstream is(line);

                int r, g, b;
                is >> r >> g >> b;

                if(is.fail()) {
                        form.serial_data->append(line.c_str());
                        return;
                } else {
                        r = min(255, r);
                        g = min(255, g);
                        b = min(255, b);

                        char css_color[8];
                        snprintf(css_color, sizeof(css_color),
                                        "#%02x%02x%02x", r, g, b);

                        QString org_content = form.serial_data->toHtml();

                        while(org_content.size() > (2 << 13)) {
                                int index = org_content.indexOf('\n');
                                if(index < 0) break;
                                org_content = org_content.mid(index + 1);
                        }

                        form.serial_data->setHtml(org_content +
                                QString("<p style=\"background: %1;"
                                "margin: 0px\">%1</p>\n").arg(css_color));
                }

                form.serial_data->verticalScrollBar()->setValue(
                        form.serial_data->verticalScrollBar()->maximum());
        }
}

int main(int argc, char* argv[])
{
        QApplication app(argc, argv);

        MainWindow main_window;
        main_window.show();

        return app.exec();
}
