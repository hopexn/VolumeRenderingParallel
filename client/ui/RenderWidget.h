#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <string>
#include <boost/timer/timer.hpp>

using namespace boost;
using namespace std;

#include "../common/util.h"
#include "Camera.hpp"
#include "Image.hpp"
#include "../network/Message.h"

class RenderWidget : public QWidget {
    Q_OBJECT

public:
    RenderWidget(int _width, int _height, string ip, int port);

    ~RenderWidget() override;

protected:
    void paintEvent(QPaintEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void setNetMsg(Message *msg) {
        net_msg = msg;
    }

private:
    Camera *camera = nullptr;
    QImage *qimage = nullptr;

    QPoint lastPosition;

    Message *net_msg;

    QTimer *qtimer;

    Image *image;
};

#endif //RENDERWIDGET_H
