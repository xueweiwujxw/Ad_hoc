#include <opnet.h>
#include <opnet_ctrller.hpp>
#include <iostream>

using namespace std;
using namespace opnet;
#define pksize 1024

opnet_ctrller::opnet_ctrller() {
    this->packetCount = 0;
}
opnet_ctrller::~opnet_ctrller() { 
}

void opnet_ctrller::send() {
    Packet *p = op_pk_create(pksize);
    op_pk_send(p, 0);
    cout << "node " << op_node_id() << " sent a packet" << endl;
}

void opnet_ctrller::on_sim_start() {
    cout << "this is node " << op_node_id() << " start." << endl;
    for (int i = 0; i < 5; ++i)
        op_intrpt_schedule_self(op_sim_time(), 0);
    // cout << op_sim_time() << endl;
}

void opnet_ctrller::on_self() {
    this->send();
}

void opnet_ctrller::on_stream(int id) {
    cout << "node " << op_node_id() << " received packet from stream of " << id;
    this->packetCount++;
    Packet *p = op_pk_get(id);
    unsigned int len = op_pk_total_size_get(p) / 8;
    cout << ", packet size: " << len << endl;
    op_pk_destroy(p);
    // cout << "simluation time: " << op_sim_time() << endl;
    // this->send();
}

void opnet_ctrller::on_stat(int id) {
    cout << "opnet_stat" << endl;

}

void opnet_ctrller::on_sim_stop() {
    cout << "node " << op_node_id() << " received packets: " << this->packetCount << endl;
    cout << "this is node " << op_node_id() << " end." << endl;
}