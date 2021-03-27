#include <opnet.h>
#include <opnet_ctrller.hpp>
#include <iostream>

using namespace std;
using namespace opnet;
#define pksize 1024

opnet_ctrller::opnet_ctrller() {    
    // cout << "opnet_ctrller()" << endl;
    this->packetCount = 0;
}
opnet_ctrller::~opnet_ctrller() { 
    // cout << "~opnet_ctrller" << endl;
}

void opnet_ctrller::schedule_self() {
    op_intrpt_schedule_self(op_sim_time(), 0);
    cout << "current sim time: " << op_sim_time() << endl;
}

void opnet_ctrller::send() {
    Packet *p = op_pk_create(pksize);
    // Packet *p = op_pk_create_fmt("example_pkt");
    
    // op_pk_send(p, 0);
    op_pk_bcast_general (p);
    cout << "node " << op_node_id() << " sent a packet" << endl;
}

void opnet_ctrller::on_sim_start() {
    cout << "this is node " << op_node_id() << " start." << endl;
    // for (int i = 0; i < 5; ++i)
        // op_intrpt_schedule_self(op_sim_time(), 0);
    this->schedule_self();
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
    this->schedule_self();
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