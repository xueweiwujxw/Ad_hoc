#include <opnet.h>
#include <opnet_ctrller.hpp>
#include <iostream>

using namespace std;
using namespace opnet;
#define pksize 1024

opnet_ctrller::opnet_ctrller() {    
    // cout << "opnet_ctrller()" << endl;
    this->packetSequence = 0;
    this->packetCount = 0;
    this->isrunning = false;
}
opnet_ctrller::~opnet_ctrller() { 
    // cout << "~opnet_ctrller" << endl;
}

int opnet_ctrller::run() {
    while (this->isrunning) {
        // if (((int)op_sim_time()) % 3 == op_node_id())
            this->schedule_self();
    }
    return 0;
}

void opnet_ctrller::schedule_self() {
    op_intrpt_schedule_self(op_sim_time() + op_node_id(), 0);
}

void opnet_ctrller::send(void *data, unsigned int len) {
    Packet *p = op_pk_create(len * 8);
    void *custom = op_prg_mem_alloc(len);
    op_prg_mem_copy(data, custom, len);
    op_pk_fd_set_ptr(p, 0, custom, 8 * len, op_prg_mem_copy_create, op_prg_mem_free, len);

    // Packet *p = op_pk_create_fmt("example_pkt");
    
    // op_pk_bcast_general (p);
    op_pk_send(p, 0);
    cout << "node " << op_node_id() << " sent a packet" << endl;
}

void opnet_ctrller::on_sim_start() {
    cout << "this is node " << op_node_id() << " start." << endl;
    // for (int i = 0; i < 5; ++i)
        // op_intrpt_schedule_self(op_sim_time(), 0);
    this->schedule_self();
    this->isrunning = true;
    // m_future = async(&opnet_ctrller::run, this);

    // cout << op_sim_time() << endl;
}

void opnet_ctrller::on_self() {
    nodePackets *np = new nodePackets;
    np->number = this->packetSequence;
    np->origin = op_node_id();
    void *data = reinterpret_cast<void *>(np);
    unsigned int len = 8;
    this->packetSequence++;
    cout << "current time: " << op_sim_time() << " ";
    this->send(data, len);
}

void opnet_ctrller::on_stream(int id) {
    cout << "current time: " << op_sim_time() << " ";
    Packet *p = op_pk_get(id);
    int len = op_pk_total_size_get(p) / 8;
    nodePackets *np;
    void *buffer;
    op_pk_fd_get_ptr(p, 0, &buffer);
    np = reinterpret_cast<nodePackets*>(buffer);
    cout << "node " << op_node_id() << " received packet: node->" << np->origin << " seq->" << np->number;
    this->packetCount++;
    cout << ", packet size: " << len << endl;
    op_pk_destroy(p);
    // this->schedule_self();
    // cout << "simluation time: " << op_sim_time() << endl;
    // this->send();
}

void opnet_ctrller::on_stat(int id) {
    cout << "opnet_stat" << endl;

}

void opnet_ctrller::on_sim_stop() {
    this->isrunning = false;
    // m_future.wait();
    cout << "node " << op_node_id() << " received packets: " << this->packetCount << endl;
    cout << "this is node " << op_node_id() << " end." << endl;
}