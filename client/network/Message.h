#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

using namespace std;

#include <cstdlib>
#include <cassert>

#include "Camera.hpp"


class Message {
public:
    Message(Camera *_camera, void *_pixel_buffer, int _pbuffer_size) : camera(_camera), socket(io_service) {
        pbuffer_size = _pbuffer_size;
        pixel_buffer = _pixel_buffer;
    }

    string getRequest() {
        assert(camera);

        ostringstream output;

        const Vector3f position = camera->getPosition();
        const Vector3f focus = camera->getFocus();
        const Vector3f right = camera->getRightDirection();

        output << position.x << " " << position.y << " " << position.z << endl;
        output << focus.x << " " << focus.y << " " << focus.z << endl;
        output << right.x << " " << right.y << " " << right.z << endl;

        return output.str();
    }

    void connect(const string &_ip, int _port) {
        ip = _ip;
        port = _port;
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(_ip), (unsigned short) _port);
        socket.connect(ep, error);
        if (socket.is_open()) {
            cout << "Socket open ~" << endl;
        }

    }

    void disconnect() {
        socket.close();
    }

    int req_and_recv() {
        if (!socket.is_open()) {
            cout << "Socket doesn't open!" << endl;
            exit(0);
        }

        string req_msg = getRequest();

        socket.write_some(boost::asio::buffer(req_msg), error);

        boost::asio::read(socket, boost::asio::buffer(img_size_buf, 32));
        int img_size = std::atoi(img_size_buf);
        boost::asio::read(socket, boost::asio::buffer(pixel_buffer, (size_t)img_size));

        return img_size;
    }

private:
    Camera *camera;

    string ip;
    int port;

    boost::asio::io_service io_service;
    tcp::socket socket;
    boost::system::error_code error;

    void *pixel_buffer;
    int pbuffer_size;

    char img_size_buf[32];
};

#endif //MESSAGE_H
