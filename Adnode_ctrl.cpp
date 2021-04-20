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
    vector<pair<pair<Linkcode, Neighborcode>, unsigned int>> tuples;
    // 根据本地链路表、MPR表和一跳邻居表，判断每一个链路的Linkcode和Neighborcode
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
        pair<Linkcode, Neighborcode> code = make_pair(lc, nc);
        pair<pair<Linkcode, Neighborcode>, unsigned int> item = make_pair(code, i.L_neighbor_iface_addr);
        tuples.push_back(item);
    } 
    // 加入对应状态的邻居地址
    link_status links[15];
    for (int i = 0; i < 15; ++i){
        links[i].linkcode = i % 5;
        links[i].neighcode = i / 5;
    }
    for (auto &item : tuples) {
        if (item.first.first == UNSPEC_LINK && item.first.second == SYM_NEIGH) links[0].neighborAddress.push_back(item.second);
        else if (item.first.first == ASYM_LINK && item.first.second == SYM_NEIGH) links[1].neighborAddress.push_back(item.second);
        else if (item.first.first == SYM_LINK && item.first.second == SYM_NEIGH) links[2].neighborAddress.push_back(item.second);
        else if (item.first.first == MPR_LINK && item.first.second == SYM_NEIGH) links[3].neighborAddress.push_back(item.second);
        else if (item.first.first == LOST_LINK && item.first.second == SYM_NEIGH) links[4].neighborAddress.push_back(item.second);

        else if (item.first.first == UNSPEC_LINK && item.first.second == MPR_NEIGH) links[5].neighborAddress.push_back(item.second);
        else if (item.first.first == ASYM_LINK && item.first.second == MPR_NEIGH) links[6].neighborAddress.push_back(item.second);
        else if (item.first.first == SYM_LINK && item.first.second == MPR_NEIGH) links[7].neighborAddress.push_back(item.second);
        else if (item.first.first == MPR_LINK && item.first.second == MPR_NEIGH) links[8].neighborAddress.push_back(item.second);
        else if (item.first.first == LOST_LINK && item.first.second == MPR_NEIGH) links[9].neighborAddress.push_back(item.second);

        else if (item.first.first == UNSPEC_LINK && item.first.second == NOT_NEIGH) links[10].neighborAddress.push_back(item.second);
        else if (item.first.first == ASYM_LINK && item.first.second == NOT_NEIGH) links[11].neighborAddress.push_back(item.second);
        else if (item.first.first == SYM_LINK && item.first.second == NOT_NEIGH) links[12].neighborAddress.push_back(item.second);
        else if (item.first.first == MPR_LINK && item.first.second == NOT_NEIGH) links[13].neighborAddress.push_back(item.second);
        else if (item.first.first == LOST_LINK && item.first.second == NOT_NEIGH) links[14].neighborAddress.push_back(item.second);        
    }
    // 将链路状态整合在HelloMessage中
    for (int i = 0; i < 15; ++i) {
        if (!links[i].neighborAddress.empty()) {
            links[i].linkMessageSize = 4 + 4 * links[i].neighborAddress.size();
            mh.linkMessage.insert(links[i]);
        }
    }
    // willingness 可以在之后动态修改
    mh.willingness = WILL_DEFAULT;
    mh.hTime = HELLO_INTERVAL;
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