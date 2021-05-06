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

void opnet_ctrller::send(OLSR_packet data, UNINT len, UNINT type) {
    if (type == OPC_HELLO_SEND)
        this->schedule_self(HELLO_INTERVAL, OPC_HELLO_SEND);
    if (type == OPC_TC_SEND)
        this->schedule_self(TC_INTERVAL, OPC_TC_SEND);
    
    int needSpace = data.getSize();
    UNINT *format = new UNINT[needSpace];
    int mark = 0;
    format[mark++] = (data.packetLenth << 16) | (data.packetSequenceNumber & 0xffff);
    for (auto &i : data.messagePackets) {
        format[mark++] = (i.messageType << 24) | ((i.vTime & 0xff) << 16) |  (i.messageSize & 0xffff);
        format[mark++] = i.originatorAddress;
        format[mark++] = (i.TTL << 24) | ((i.hopCount & 0xff) << 16) | (i.messageSequenceNumber & 0xffff);
        if (i.messageType == HELLO)  {
            format[mark++] = i.helloMessage.willingness;
            format[mark++] = (i.helloMessage.linkNum << 16) | (i.helloMessage.neighNum & 0xffff);
            for (auto &j : i.helloMessage.links) {
                format[mark++] = (j.linkcode << 16) | (j.num & 0xffff);
                for (auto &k : j.neighborAddress)
                    format[mark++] = k;
            }
            for (auto &j : i.helloMessage.neighs) {
                format[mark++] = (j.neighcode << 16) | (j.num & 0xffff);
                for (auto &k : j.neighborAddress)
                    format[mark++] = k;
            }
        }
        else if (i.messageType == TC) {
            format[mark++] = (i.tcMessage.MSSN << 16) | (i.tcMessage.num & 0xffff);
            for (auto &j : i.tcMessage.MPRSelectorAddresses)
                format[mark++] = j;
        }
        else if (i.messageType == NORMAL) {
            format[mark++] = i.normalMessage.id;
            format[mark++] = i.normalMessage.time;
        }
    }
    assert(mark == needSpace);
    // cout << op_node_id() << " send packet " << data.packetSequenceNumber << endl;
    // cout << mark << endl;

    Packet *p = op_pk_create(0);
    void *custom = op_prg_mem_alloc(len);
    op_prg_mem_copy(format, custom, len);
    delete format;
    op_pk_fd_set_ptr(p, 0, custom, 8 * len, op_prg_mem_copy_create, op_prg_mem_free, len);
    op_pk_send(p, 0);
    results item(this->resId, op_sim_time(), "SEND", op_node_id(), data.packetSequenceNumber, -1, -1, -1, -1);
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
    pair<OLSR_packet, UNINT> data;
    if (op_intrpt_code() == OPC_TC_SEND) {
        data = this->acs->getOLSRPackets(0, 1);
        this->send(data.first, data.second, OPC_TC_SEND);
    }
    if (op_intrpt_code() == OPC_HELLO_SEND) {
        data = this->acs->getOLSRPackets(1, 0);
        this->send(data.first, data.second, OPC_HELLO_SEND);
    }
}

void opnet_ctrller::on_stream(int id) {
    Packet *p = op_pk_get(id);
    int len = op_pk_total_size_get(p) / 8;
    void *buffer;
    op_pk_fd_get_ptr(p, 0, &buffer);
    UNINT* format = reinterpret_cast<UNINT*>(buffer);
    int mark = 0;
    OLSR_packet np;
    np.packetSequenceNumber = format[mark] & 0xffff;
    np.packetLenth = (format[mark++] & 0xffff0000) >> 16;
    // cout << op_node_id() << " recv " << len << " packet len: " << np.packetLenth << endl;
    for (int i = np.packetLenth; i > 0; --i) {
        // cout << "step 1" << endl;
        UNINT tmp = format[mark] & 0xffff;
        message_packet mp((format[mark] & 0xff000000) >> 24, \
                          (format[mark++] & 0x00ff0000) >> 16, \
                          (format[mark++]), \
                          (format[mark] & 0xff000000) >> 24, \
                          (format[mark] & 0x00ff0000) >> 16, \
                          (format[mark++] & 0xffff));
        mp.messageSize = tmp;
        // cout << "step 2" << endl;
        if (mp.messageType == HELLO) {
            mp.helloMessage.willingness = format[mark++];
            mp.helloMessage.linkNum = (format[mark] & 0xffff0000) >> 16;
            mp.helloMessage.neighNum = format[mark++] & 0xffff;
            for (int j = mp.helloMessage.linkNum; j > 0; --j) {
                link_status ltmp;
                ltmp.linkcode = (format[mark] & 0xffff0000) >> 16;
                ltmp.num = (format[mark++] & 0xffff);
                for (int k = ltmp.num; k > 0; --k)
                    ltmp.neighborAddress.push_back(format[mark++]);
                mp.helloMessage.links.push_back(ltmp);
            }
            for (int j = mp.helloMessage.neighNum; j > 0; --j) {
                neigh_status ntmp;
                ntmp.neighcode = (format[mark] & 0xffff0000) >> 16;
                ntmp.num = (format[mark++] & 0xffff);
                for (int k = ntmp.num; k > 0; --k)
                    ntmp.neighborAddress.push_back(format[mark++]);
                mp.helloMessage.neighs.push_back(ntmp);
            }
        }
        else if (mp.messageType == TC) {
            mp.tcMessage.MSSN = (format[mark] & 0xffff0000) >> 16;
            mp.tcMessage.num = (format[mark++] & 0xffff);
            for (int j = mp.tcMessage.num; j > 0; --j)
                mp.tcMessage.MPRSelectorAddresses.push_back(format[mark++]);
        }
        else if (mp.messageType == NORMAL) {
            mp.normalMessage.id = format[mark++];
            mp.normalMessage.time = format[mark++];
        }
        // cout << "step last" << endl;
        np.messagePackets.push_back(mp);
    }
    // cout << "recv packets len: " << len << endl;
    if (np.packetLenth != 4) {
        for (auto &i : np.messagePackets) {
            if (i.originatorAddress != op_node_id()) {
                // cout << i.originatorAddress << " " << i.messageSequenceNumber << endl;
                results item(this->resId, op_sim_time(), "RECV", i.originatorAddress, i.messageSequenceNumber, op_td_get_dbl(p, OPC_TDA_RA_END_DIST), op_td_get_dbl(p, OPC_TDA_RA_END_PROPDEL), op_td_get_dbl(p, OPC_TDA_RA_ACTUAL_BER), op_td_get_dbl(p, OPC_TDA_RA_SNR));
                this->resId++;
                this->res.push_back(item);
                this->packetCount++;
            }
        }
    }
    // this->acs->recvPackets(&np);
    op_pk_destroy(p);
}

void opnet_ctrller::on_stat(int id) {

}

void opnet_ctrller::on_sim_stop() {
    cout << "node " << op_node_id() << " stop at time : " << op_sim_time() << endl;
    cout << "node " << op_node_id() << " received packets: " << this->packetCount << endl;
    // this->printRess();
}

void opnet_ctrller::printRess() {
    cout << "id   |time      |send/recv |ori       |seq       |dist      |delay     |BER       |SNR      " << endl;
    cout << "-----|----------|----------|----------|----------|----------|----------|----------|---------" << endl;
    for (auto &i : this->res)
        i.printRes();
}