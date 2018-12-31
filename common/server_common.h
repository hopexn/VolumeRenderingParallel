#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

inline static
vifo load_vifo(const string &vifo_path) {
    ifstream input(vifo_path);
    if (!input.is_open()) {
        cout << "Could not open vifo file!" << endl;
        exit(1);
    }
    vifo v;
    input >> v.dims.x >> v.dims.y >> v.dims.z;
    input >> v.bmin.x >> v.bmin.y >> v.bmin.z;
    input >> v.bmax.x >> v.bmax.y >> v.bmax.z;
    input >> v.blk_size.x >> v.blk_size.y >> v.blk_size.z;
    input >> v.mmu_ratio;
    input >> v.volume_file;
    string base = vifo_path.substr(0, vifo_path.rfind('/') + 1);
    v.volume_file = base + v.volume_file;
    return v;
}

inline static
void send_tf1d(const vifo &v, MPI_Comm slave_comm) {
    string tf1d_file = v.volume_file.substr(0, v.volume_file.rfind('.') + 1) + "tf1d";
    FILE *fp = fopen(tf1d_file.c_str(), "rb");
    if (fp == nullptr) {
        cout << "Could not open tf1d file!" << endl;
        exit(1);
    }
    char tf1d_buf[TF1D_BUF_SIZE];
    fread(tf1d_buf, sizeof(char), TF1D_BUF_SIZE, fp);
    MPI_Send(tf1d_buf, TF1D_BUF_SIZE, MPI_CHAR, 0, 0, slave_comm);
    fclose(fp);
}

inline static
string recv_bcast_tf1d(MPI_Comm master_comm, int my_rank) {
    MPI_Status status;
    char tf1d_buf[TF1D_BUF_SIZE];
    if (my_rank == 0) {
        MPI_Recv(tf1d_buf, TF1D_BUF_SIZE, MPI_CHAR, 0, 0, master_comm, &status);
    }
    MPI_Bcast(tf1d_buf, TF1D_BUF_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
    return string(tf1d_buf);
}

#endif //SERVER_COMMON_H
