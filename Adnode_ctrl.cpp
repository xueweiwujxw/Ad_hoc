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
    for (auto &i : this->localLinkTable) {
        Linkcode lc;
        Neighborcode nc;
        if (i.L_SYM_time >= op_sim_time()) lc = SYM_LINK;
        else if (i.L_ASYM_time >= op_sim_time()) lc = ASYM_LINK;
        else lc = LOST_LINK;
        bool inmpr = false, inneigh = false;
        unsigned int nt;
        for (auto &j : this->mprTable)
            if (j.MS_main_addr == i.L_neighbor_iface_addr) {
                inmpr = true;
                break;
            }
        for (auto &j : this->oneHopNeighborTable)
            if (j.N_neighbor_main_addr == i.L_neighbor_iface_addr) {
                nt = j.N_status;
                inneigh = true;
                break;
            }
        if (inmpr) nc = MPR_NEIGH;
        else if (inneigh && nt == SYM) nc = SYM_NEIGH;
        else if (inneigh && nt == NOT_SYM) nc = NOT_NEIGH;
        else if (!inneigh) {
            lc = UNSPEC_LINK;
            nc = NOT_NEIGH;
        }
    } 
    return mh;
}

message_tc Adnode_ctrl::createTC() {
    
}

void Adnode_ctrl::recvPacket(void *data) {
    
}

void Adnode_ctrl::forward(set<message_packet> needToForward) {
    
}

void Adnode_ctrl::handleHello(message_packet mh) {
    // 更新链路信息表
    
}

void Adnode_ctrl::handleTc(message_packet mt) {
    
}