#include "RenderWidget.h"

RenderWidget::RenderWidget(int _width, int _height, string ip, int port) {
    this->setFixedWidth(_width);
    this->setFixedHeight(_height);

    camera = new Camera;
    qimage = new QImage(_width, _height, QImage::Format_RGB888);

    image = new Image(_width, _height);

    net_msg = new Message(camera, image->getPixelData(), image->size());

    net_msg->connect(ip.c_str(), port);
}

RenderWidget::~RenderWidget() {
    net_msg->disconnect();
    delete camera;
    delete qimage;
    delete image;
    delete net_msg;
}

/**
 * 绘制事件
 * @param event
 */
void RenderWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    timer::auto_cpu_timer auto_timer(6, "%ws real time\n");

    int img_size = net_msg->req_and_recv();
    qimage->loadFromData((const unsigned char *) image->getPixelData(), img_size);
    painter.drawImage(QRect(0, 0, this->width(), this->height()), *qimage);
}

/**
 * 鼠标事件
 * @param event
 */
void RenderWidget::mouseMoveEvent(QMouseEvent *event) {
    float dx = float(event->x() - lastPosition.x()) / this->width();
    float dy = float(event->y() - lastPosition.y()) / this->height();
    if (event->buttons() & Qt::RightButton) {
        camera->translate(dx, -dy);
    } else if (event->buttons() & Qt::LeftButton) {
        camera->rotate(dx, dy);
    }
    lastPosition = event->pos();
    update();
}

/**
 * 滚轮事件， 控制放缩
 * @param event
 */
void RenderWidget::wheelEvent(QWheelEvent *event) {
    if (event->delta() > 0) {
        camera->scale(0.88f);
    } else {
        camera->scale(1 / 0.88f);
    }
    update();
}

/*
 * 鼠标事件，当鼠标按下后记录鼠标位置
 */
void RenderWidget::mousePressEvent(QMouseEvent *event) {
    lastPosition = event->pos();
}


