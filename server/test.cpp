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

int main(int argc, char **argv) {
    MPI_Comm slave_comm;

    MPI_Init(&argc, &argv);

    if (argc != 9) {
        cout << "command: ./VolumerRendererServer_master vifo_path slave_size "
             << "hread_per_node width height blk_x blk_y blk_z" << endl;
        cout << "Example: ./VolumerRendererServer_master data/engine.vifo 2 4 512 512 32 32 32" << endl;
        exit(0);
    }

    string vifo_path = string(argv[1]);
    int slave_size = atoi(argv[2]);
    int thread_nums = atoi(argv[3]);
    int width = atoi(argv[4]);
    int height = atoi(argv[5]);
    int blk_x = atoi(argv[6]);
    int blk_y = atoi(argv[7]);
    int blk_z = atoi(argv[8]);

    vifo v = load_vifo(vifo_path);

    ostringstream output;
    output << width << " " << height << " " << thread_nums << " ";
    output << v.dims.x << " " << v.dims.y << " " << v.dims.z << " ";
    output << blk_x << " " << blk_y << " " << blk_z << " ";
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

    int ibuf_size = (int) (ceil((float) height / slave_size)) * width * 3;
    char ibuf[slave_size][ibuf_size];
    MPI_Request slave_request[slave_size];
    MPI_Status slave_status[slave_size];

    char msg_buf[MSG_BUF_SIZE] = "0 0 2 0 0 0 0 1 0";

    int image_nums = 5;
    MPI_Send(&image_nums, 1, MPI_INT, 0, 0, slave_comm);

    double t1, t2, t_sum;
    cout << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << " "
         << argv[5] << " " << argv[6] << " " << argv[7] << " " << argv[8] << " " << endl;

    for (int j = 0; j < image_nums; j++) {
        MPI_Send(msg_buf, MSG_BUF_SIZE, MPI_CHAR, 0, 0, slave_comm);
        t1 = MPI_Wtime();
        for (int i = 0; i < slave_size; i++) {
            MPI_Irecv(ibuf[i], ibuf_size, MPI_CHAR, i, i, slave_comm, &slave_request[i]);
        }
        MPI_Waitall(slave_size, slave_request, slave_status);
        t2 = MPI_Wtime();
        if (j >= 2) {
            cout << t2 - t1 << " ";
            t_sum += (t2 - t1);
        }
    }
    cout << endl;

    cout << "Rendering time cost:" << t_sum / (image_nums - 2) << endl;

    write_image_disk((Pixel *) ibuf[0], width, height, "test.jpg");

    MPI_Finalize();
    exit(0);
}
