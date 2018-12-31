#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <mpi.h>

using namespace std;
using namespace boost;
using boost::asio::ip::tcp;

#include "../common/Camera.hpp"
#include "volume/Volume.hpp"
#include "volume/Renderer.hpp"
#include "../common/server_common.h"

int main(int argc, char **argv) {
    int my_rank, comm_size;
    MPI_Comm master_comm;
    MPI_Status status;
    MPI_Request request;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_get_parent(&master_comm);

    int width, height, thread_nums;
    vifo v;
    istringstream input(argv[1]);
    input >> width >> height >> thread_nums;
    input >> v.dims.x >> v.dims.y >> v.dims.z;
    input >> v.blk_size.x >> v.blk_size.y >> v.blk_size.z;
    input >> v.bmin.x >> v.bmin.y >> v.bmin.z;
    input >> v.bmax.x >> v.bmax.y >> v.bmax.z;
    input >> v.mmu_ratio;
    input >> v.volume_file;

    if (my_rank == 0) {
        volume_rearrangement(v.dims, v.blk_size, v.volume_file);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    Volume *volume = new Volume(v.dims, v.blk_size, v.bmin, v.bmax);

    volume->setMmu(new MMU(v.dims, v.blk_size, v.mmu_ratio, v.volume_file));

    string tf1d_str = recv_bcast_tf1d(master_comm, my_rank);

    TF1D *tf1d = new TF1D;
    tf1d->load(tf1d_str);
    volume->setTf1d(tf1d);

    volume->checkStatus(0);
    if (my_rank == 0) {
        volume->show_empty_block_nums();
    }

    char msg_buf[MSG_BUF_SIZE];
    Vector3f position, focus, right;
    Camera camera;

    int ibuf_size = (int) ceil((float) height / comm_size) * width * 3;

    char ibuf[2][ibuf_size];
    int ibuf_idx = 0;

    Renderer renderer(width, height, thread_nums);

    int tag = 0, first = 1;
    int image_nums;
    if (my_rank == 0) {
        MPI_Recv(&image_nums, 1, MPI_INT, 0, 0, master_comm, &status);
    }
    MPI_Bcast(&image_nums, 1, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < image_nums; i++) {
        if (my_rank == 0) {
            MPI_Recv(msg_buf, MSG_BUF_SIZE, MPI_CHAR, 0, 0, master_comm, &status);
        }
        MPI_Bcast(msg_buf, MSG_BUF_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);

        istringstream iss(msg_buf);
        iss >> position.x >> position.y >> position.z;
        iss >> focus.x >> focus.y >> focus.z;
        iss >> right.x >> right.y >> right.z;

        camera.setPosition(position);
        camera.setFocus(focus);
        camera.setRightDirection(right);

        renderer.renderImage(volume, &camera, (Pixel *) (ibuf[ibuf_idx % 2]), comm_size, my_rank);

        if (first) {
            first = 0;
        } else {
            MPI_Wait(&request, &status);
        }
        MPI_Isend(ibuf[ibuf_idx % 2], ibuf_size, MPI_CHAR, 0, my_rank, master_comm, &request);
        ibuf_idx++;
    }
    MPI_Finalize();
}
