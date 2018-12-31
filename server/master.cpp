#include <mpi.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

using namespace std;
using namespace boost;
using boost::asio::ip::tcp;

#include "../common/util.h"
#include "../common/server_common.h"
#include "../common/Vector.hpp"
#include "../common/Image.hpp"

#define MAX_STR_LEN 128

int main(int argc, char **argv) {
    MPI_Comm slave_comm;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int slave_size = atoi(argv[3]);
    int thread_nums = atoi(argv[4]);
    string vifo_path = string(argv[5]);

    if (argc != 6) {
        cout << "command: ./VolumerRendererServer_master width height slave_size thread_per_node vifo_path" << endl;
        cout << "Example: ./VolumerRendererServer_master 512 512 2 4 data/engine.vifo" << endl;
        exit(0);
    }

    vifo v = load_vifo(vifo_path);

    ostringstream output;
    output << width << " " << height << " " << thread_nums << " ";
    output << v.dims.x << " " << v.dims.y << " " << v.dims.z << " ";
    output << v.blk_size.x << " " << v.blk_size.y << " " << v.blk_size.z << " ";
    output << v.bmin.x << " " << v.bmin.y << " " << v.bmin.z << " ";
    output << v.bmax.x << " " << v.bmax.y << " " << v.bmax.z << " ";
    output << v.mmu_ratio << " ";
    output << v.volume_file << endl;

    string msg = output.str();

    char **slave_argv = (char **) malloc(sizeof(char *) * 2);
    slave_argv[0] = new char[1024];
    slave_argv[1] = nullptr;
    strcpy(slave_argv[0], msg.c_str());

    char command[MAX_STR_LEN] = "./VolumeRendererServer_slave";
    auto error_codes = (int *) malloc(sizeof(int) * slave_size);
    MPI_Comm_spawn(command, slave_argv, slave_size,
                   MPI_INFO_NULL, 0, MPI_COMM_WORLD,
                   &slave_comm, error_codes);         //创建子进程
    delete[] slave_argv;

    send_tf1d(v, slave_comm);

    boost::asio::io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), PORT));
    tcp::socket socket(io_service);

    int lines_per_slave = (int) (ceil((float) height / slave_size));
    int ibuf_size = lines_per_slave * width * 3;
    char ibuf_src[slave_size * ibuf_size];
    char ibuf_dst[slave_size * ibuf_size];

    MPI_Request slave_request[slave_size];
    MPI_Status slave_status[slave_size];

    char msg_buf[MSG_BUF_SIZE];
    char img_size_buf[32];

    int image_nums = 500;
    MPI_Send(&image_nums, 1, MPI_INT, 0, 0, slave_comm);

    double t1, t2;
    try {
        acceptor.accept(socket);
        cout << "accept" << endl;
        for (int j = 0; j < image_nums; j++) {
            size_t len = socket.read_some(boost::asio::buffer(msg_buf, MSG_BUF_SIZE));

            MPI_Send(msg_buf, MSG_BUF_SIZE, MPI_CHAR, 0, 0, slave_comm);
            t1 = MPI_Wtime();
            for (int i = 0; i < slave_size; i++) {
                MPI_Irecv(ibuf_src + ibuf_size * i, ibuf_size, MPI_CHAR, i, i, slave_comm, &slave_request[i]);
            }
            MPI_Waitall(slave_size, slave_request, slave_status);
            t2 = MPI_Wtime();
            cout << "Rendering time cost(second):" << t2 - t1 << endl;

            int size_per_row = width * 3;
            char *dst_base = ibuf_dst;
            for (int row = 0; row < height; row++) {
                int slave_no = row % slave_size;
                int slave_row = row / slave_size;
                char *src_base = ibuf_src + ibuf_size * slave_no + slave_row * width * 3;
                memcpy(dst_base, src_base, (size_t) width * sizeof(Pixel));
                dst_base += size_per_row;
            }

            jpeg_img jpeg_info = write_image_mem((Pixel *) ibuf_dst, width, height);
            string size_str = std::to_string((long long) jpeg_info.size);
            strcpy(img_size_buf, size_str.c_str());
            socket.write_some(boost::asio::buffer(img_size_buf, 32));
            socket.write_some(boost::asio::buffer(jpeg_info.buffer, jpeg_info.size));
            delete jpeg_info.buffer;
        }
        socket.close();
        acceptor.close();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    MPI_Finalize();
}
