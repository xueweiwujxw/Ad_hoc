#include <opnet.h>
#include <opnet_ctrller.hpp>
#include <iostream>

using namespace std;
using namespace opnet;
#define pksize 1024

opnet_ctrller::opnet_ctrller() {    
    this->packetSequence = 0;
    this->packetCount = 0;
    this->resId = 0;
}
opnet_ctrller::~opnet_ctrller() { 

}


void opnet_ctrller::schedule_self(double interval) {
    op_intrpt_schedule_self(op_sim_time() + interval, 0);
}

void opnet_ctrller::send(void *data, unsigned int len) {
    Packet *p = op_pk_create(len * 8);
    void *custom = op_prg_mem_alloc(len);
    op_prg_mem_copy(data, custom, len);
    op_pk_fd_set_ptr(p, 0, custom, 8 * len, op_prg_mem_copy_create, op_prg_mem_free, len);
    op_pk_send(p, 0);
    nodePackets* np = reinterpret_cast<nodePackets*>(data);
    results item(this->resId, op_sim_time(), "SEND", np->origin, np->number, -1, -1, -1, -1);
    this->resId++;
    this->res.push_back(item);
}

void opnet_ctrller::on_sim_start() {
    // cout << "node " << op_node_id() << " start at time : " << op_sim_time() << endl;
    this->schedule_self(op_node_id() / 20.0);
}

void opnet_ctrller::on_self() {
    this->schedule_self(3);
    nodePackets *np = new nodePackets;
    np->number = this->packetSequence;
    np->origin = op_node_id();
    void *data = reinterpret_cast<void *>(np);
    unsigned int len = 8;
    this->packetSequence++;
    this->send(data, len);
}

void opnet_ctrller::on_stream(int id) {
    Packet *p = op_pk_get(id);
    int len = op_pk_total_size_get(p) / 8;
    nodePackets *np;
    void *buffer;
    op_pk_fd_get_ptr(p, 0, &buffer);
    np = reinterpret_cast<nodePackets*>(buffer);
    if (np->origin != op_node_id()) {
        results item(this->resId, op_sim_time(), "RECV", np->origin, np->number, op_td_get_dbl(p, OPC_TDA_RA_END_DIST), op_td_get_dbl(p, OPC_TDA_RA_END_PROPDEL), op_td_get_dbl(p, OPC_TDA_RA_ACTUAL_BER), op_td_get_dbl(p, OPC_TDA_RA_SNR));
        this->resId++;
        this->res.push_back(item);
        this->packetCount++;
    }
    op_pk_destroy(p);
}

void opnet_ctrller::on_stat(int id) {
    cout << "opnet_stat" << endl;

}

void opnet_ctrller::on_sim_stop() {
    cout << "node " << op_node_id() << " stop at time : " << op_sim_time() << endl;
    cout << "node " << op_node_id() << " received packets: " << this->packetCount << endl;
    this->printRess();
}

void opnet_ctrller::printRess() {
    cout << "id   |time      |send/recv |ori       |seq       |dist      |delay     |BER       |SNR      " << endl;
    cout << "-----|----------|----------|----------|----------|----------|----------|----------|---------" << endl;
    for (auto &i : this->res)
        i.printRes();
}