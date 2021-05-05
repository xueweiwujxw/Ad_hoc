#include <opnet.h>
#include <opnet_ctrller.hpp>
#include <iostream>

using namespace std;
using namespace opnet;
#define pksize 1024

opnet_ctrller::opnet_ctrller() {    
    // this->packetSequence = 0;
    this->packetCount = 0;
    this->resId = 0;
}
opnet_ctrller::~opnet_ctrller() { 
    delete acs;
}


void opnet_ctrller::schedule_self(double interval, UNINT code) {
    op_intrpt_schedule_self(op_sim_time() + interval, code);
}

void opnet_ctrller::send(void *data, UNINT len, UNINT type) {
    if (type == OPC_HELLO_SEND)
        this->schedule_self(HELLO_INTERVAL, OPC_HELLO_SEND);
    if (type == OPC_TC_SEND)
        this->schedule_self(TC_INTERVAL, OPC_TC_SEND);
    Packet *p = op_pk_create(0);
    void *custom = op_prg_mem_alloc(len);
    op_prg_mem_copy(data, custom, len);
    op_pk_fd_set_ptr(p, 0, custom, 8 * len, op_prg_mem_copy_create, op_prg_mem_free, len);
    op_pk_send(p, 0);
    OLSR_packet* np = reinterpret_cast<OLSR_packet*>(data);
    results item(this->resId, op_sim_time(), "SEND", op_node_id(), np->packetSequenceNumber, -1, -1, -1, -1);
    // nodePackets* np = reinterpret_cast<nodePackets*>(data);
    // results item(this->resId, op_sim_time(), "SEND", np->origin, np->number, -1, -1, -1, -1);
    this->resId++;
    this->res.push_back(item);
}

void opnet_ctrller::on_sim_start() {
    // cout << "node " << op_node_id() << " start at time : " << op_sim_time() << endl;
    int nodeId = op_node_id();
    this->acs = new Adnode_ctrl_simple(op_node_id());
    this->schedule_self(nodeId / 20.0, OPC_HELLO_SEND);
    this->schedule_self(nodeId / 20.0+0.005, OPC_TC_SEND);
}

void opnet_ctrller::on_self() {
    // cout << "op_intrpt_code(): " << op_intrpt_code() << endl;
    // nodePackets *np = new nodePackets;
    // np->number = this->packetSequence;
    // np->origin = op_node_id();
    // void *data = reinterpret_cast<void *>(np);
    // unsigned int len = 8;
    // this->packetSequence++;
    OLSR_packet* opack;
    UNINT len;
    if (op_intrpt_code() == OPC_TC_SEND) {
        opack = this->acs->getOLSRPackets(0, 1);
        len = opack->packetLenth;
        void* data = reinterpret_cast<void*>(opack);
        this->send(data, len, OPC_TC_SEND);
    }
    if (op_intrpt_code() == OPC_HELLO_SEND) {
        opack = this->acs->getOLSRPackets(1, 0);
        len = opack->packetLenth;
        void* data = reinterpret_cast<void*>(opack);
        this->send(data, len, OPC_HELLO_SEND);
    }
}

void opnet_ctrller::on_stream(int id) {
    Packet *p = op_pk_get(id);
    int len = op_pk_total_size_get(p) / 8;
    void *buffer;
    op_pk_fd_get_ptr(p, 0, &buffer);
    op_pk_print(p);
    // nodePackets* np;
    // np = reinterpret_cast<nodePackets*>(buffer);
    // if (np->origin != op_node_id()) {
    //     results item(this->resId, op_sim_time(), "RECV", np->origin, np->number, op_td_get_dbl(p, OPC_TDA_RA_END_DIST), op_td_get_dbl(p, OPC_TDA_RA_END_PROPDEL), op_td_get_dbl(p, OPC_TDA_RA_ACTUAL_BER), op_td_get_dbl(p, OPC_TDA_RA_SNR));
    //     this->resId++;
    //     this->res.push_back(item);
    //     this->packetCount++;
    // }
    OLSR_packet *np;
    np = reinterpret_cast<OLSR_packet*>(buffer);
    // cout << "recv packets len: " << len << endl;
    if (np->packetLenth != 4) {
        for (auto &i : np->messagePackets) {
            if (i.originatorAddress != op_node_id()) {
                results item(this->resId, op_sim_time(), "RECV", i.originatorAddress, i.messageSequenceNumber, op_td_get_dbl(p, OPC_TDA_RA_END_DIST), op_td_get_dbl(p, OPC_TDA_RA_END_PROPDEL), op_td_get_dbl(p, OPC_TDA_RA_ACTUAL_BER), op_td_get_dbl(p, OPC_TDA_RA_SNR));
                this->resId++;
                this->res.push_back(item);
                this->packetCount++;
            }
        }
    }
    op_pk_destroy(p);
}

void opnet_ctrller::on_stat(int id) {

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