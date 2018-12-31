#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <boost/asio.hpp>

using namespace std;
using namespace boost;
using boost::asio::ip::tcp;

#include "../common/util.h"

#define IMG_BUF_SIZE 10000000

int main(int argc, char **argv) {
    char *msg_buf[MSG_BUF_SIZE];
    char img_size_buf[32];

    if (argc != 2) {
        cout << "usage: ./proxy ip_address" << endl;
        exit(1);
    }

    string target_ip(argv[1]);

    boost::asio::io_service io_service;
    tcp::socket socket1(io_service);
    boost::system::error_code error;
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(target_ip), (unsigned short) PORT);
    socket1.connect(ep, error);

    if (socket1.is_open()) {
        cout << "Socket1 open ~" << endl;
    } else {
        cout << "can't not open socket1!" << endl;
        exit(1);
    }
    char *buffer = new char[IMG_BUF_SIZE];

    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), PORT + 1));
    tcp::socket socket2(io_service);
    try {
        acceptor.accept(socket2);
        while (true) {
            socket2.read_some(boost::asio::buffer(msg_buf, MSG_BUF_SIZE));
            socket1.write_some(boost::asio::buffer(msg_buf, MSG_BUF_SIZE));

            boost::asio::read(socket1, boost::asio::buffer(img_size_buf, 32));
            socket2.write_some(boost::asio::buffer(img_size_buf, 32));

            int img_size = atoi(img_size_buf);
            if (img_size > IMG_BUF_SIZE) {
                cout << "Image size exceed buffer: " << img_size << endl;
                exit(1);
            }
            boost::asio::read(socket1, boost::asio::buffer(buffer, (size_t) img_size));
            socket2.write_some(boost::asio::buffer(buffer, (size_t) img_size));
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    socket1.close();
    socket2.close();
    acceptor.close();
    delete[] buffer;

    return 0;
}