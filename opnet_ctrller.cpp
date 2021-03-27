#include <opnet.h>
#include <opnet_ctrller.hpp>
#include <iostream>

using namespace std;
using namespace opnet;

opnet_ctrller::opnet_ctrller() {
    this->packetCount = 0;
}
opnet_ctrller::~opnet_ctrller() {
    
}

void opnet_ctrller::on_sim_start() {
    cout << "opnet_sim_start" << endl;
    int id;
    op_ima_obj_attr_get_int32(op_node_id(), "user id", &id);
    cout << "id: " << id << endl;
    
}


void opnet_ctrller::on_self() {
    double pksize = 1024;
    Packet *p = op_pk_create(pksize);
    op_pk_send(p, 0);
}

void opnet_ctrller::on_stream(int id) {
    cout << "opnet_strm" << endl;
    this->packetCount++;
    Packet *p = op_pk_get(id);
    // void *buffer = 0;
    // op_pk_fd_get_ptr(p, 0, &buffer);
    cout << "opnet_pk_recv" << endl;
    op_pk_destroy(p);
}

void opnet_ctrller::on_stat(int id) {
    cout << "opnet_stat" << endl;

}

void opnet_ctrller::on_sim_stop() {
    cout << "Received packet count: " << this->packetCount << endl;
    cout << "opnet_sim_stop" << endl;
}