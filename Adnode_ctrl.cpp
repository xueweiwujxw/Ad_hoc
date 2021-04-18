#pragma once
#include <Adnode_ctrl.hpp>

using namespace std;
using namespace opnet;

Adnode_ctrl::Adnode_ctrl() {
    this->nodeId = op_node_id();
}

Adnode_ctrl::~Adnode_ctrl() {

}

int Adnode_ctrl::run() {

}

void Adnode_ctrl::start() {

}

void Adnode_ctrl::stop() {

}

message_hello Adnode_ctrl::createHello() {
    message_hello mh;
    mh.hTime = 2;
    mh.willingness = WILL_DEFAULT;
    link_status ls(SYM_LINK);
    link_status las(ASYM_LINK);
    link_status lmpr(MPR_LINK);
    for (auto &i : this->oneHopNeighborTable.getTable()) {
        if (i.N_status == SYM_LINK)
            ls.neighborAddresses.push_back(i.N_neighbor_main_addr);
        else if (i.N_status == ASYM_LINK)
            las.neighborAddresses.push_back(i.N_neighbor_main_addr);
        else if (i.N_status == MPR_LINK)
            lmpr.neighborAddresses.push_back(i.N_neighbor_main_addr);
    }
    ls.linkMessageSize = 4 + 4 * ls.neighborAddresses.size();
    las.linkMessageSize = 4 + 4 * las.neighborAddresses.size();
    lmpr.linkMessageSize = 4 + 4 * lmpr.neighborAddresses.size();
    return mh;
}

message_tc Adnode_ctrl::createTC() {
    message_tc mt;
    mt.MSSN = this->createMprTable();
    for (auto &i: this->mprTable.getTable())
        mt.MPRSelectorAddresses.push_back(i.MS_main_addr);
    return mt;
}

void Adnode_ctrl::recvMessage(void *data) {
    message_packet* messp = reinterpret_cast<message_packet *>(data);
    if (messp->messageType == HELLO) {
        message_hello messh = messp->helloMessage;
    } else if (messp->messageType == TC) {
        message_tc messt = messp->tcMessage;
    }
}
