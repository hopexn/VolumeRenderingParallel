#include <iostream>
#include <qapplication.h>
#include "ui/RenderWidget.h"

using namespace std;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    if (argc != 5) {
        cout << "command: ./VolumerRendererClient ip_address port width height" << endl;
        cout << "Example: ./VolumerRendererClient 127.0.0.1 9523 512 512" << endl;
        exit(0);
    }
    string ip = string(argv[1]);
    int port = atoi(argv[2]);
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);

    RenderWidget renderWidget(width, height, ip, port);
    renderWidget.show();
    return app.exec();
}