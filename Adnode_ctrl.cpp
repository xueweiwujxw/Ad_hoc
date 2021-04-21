#pragma once
#include <Adnode_ctrl.hpp>
#include <set>
#include <map>

using namespace std;
using namespace opnet;

Adnode_ctrl::Adnode_ctrl() {
    this->nodeId = op_node_id();
    this->MSSN = 0;
    this->messageSequenceNumber = 0;
    this->packetSequenceNumber = 0;
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
            mh.linkMessage.push_back(links[i]);
        }
    }
    // willingness 可以在之后动态修改
    mh.willingness = WILL_DEFAULT;
    mh.hTime = HELLO_INTERVAL;
    return mh;
}

message_tc Adnode_ctrl::createTC() {
    message_tc mt;
    mt.MSSN = this->MSSN;
    if (this->mprTable.empty())
        return mt;
    else {
        for (auto &i : this->mprTable)
            mt.MPRSelectorAddresses.push_back(i.MS_main_addr);
        return mt;
    }
}

olsr_packet Adnode_ctrl::createPacket(bool helloEnable, bool tcEnable) {
    olsr_packet olsrp;
    olsrp.packetSequenceNumber = this->packetSequenceNumber;
    this->packetSequenceNumber = (this->packetSequenceNumber + 1) % 65536;
    if (helloEnable) {
        message_hello mh = this->createHello();
        message_packet mp;
        mp.messageType = HELLO;
        mp.vTime = HELLO_VALIDITY_TIME;
        mp.messageSize = 4 + 4 + 4 + 4;
        for (auto &i : mh.linkMessage)
            mp.messageSize += i.linkMessageSize;
        mp.originatorAddress = this->nodeId;
        mp.TTL = NODE_COUNT;
        mp.hopCount = 0;
        mp.messageSequenceNumber = this->messageSequenceNumber;
        this->messageSequenceNumber = (this->messageSequenceNumber + 1) % 65536;
        mp.helloMessage = mh;
        olsrp.messagePackets.push_back(mp);
    }
    if (tcEnable) {
        message_tc mt = this->createTC();
        message_packet mp;
        mp.messageType = TC;
        mp.vTime = TOP_HOLD_TIME;
        mp.messageSize = 4 + 4 + 4 + 4 + mt.MPRSelectorAddresses.size() * 4;
        mp.originatorAddress = this->nodeId;
        mp.TTL = 255;
        mp.hopCount = 0;
        mp.messageSequenceNumber = this->messageSequenceNumber;
        this->messageSequenceNumber = (this->messageSequenceNumber + 1) % 65536;
        mp.tcMessage = mt;
        olsrp.messagePackets.push_back(mp);
    }
    olsrp.packetLength = 4;
    for (auto &i : olsrp.messagePackets)
        olsrp.packetLength += i.messageSize;
    return olsrp;
}

void Adnode_ctrl::recvPacket(void *data) {
    
}

void Adnode_ctrl::forward(vector<message_packet> needToForward) {
    
}

void Adnode_ctrl::handleHello(message_packet mh) {
    if (mh.messageType != HELLO) {
        cout << "message type error" << endl;
        return;
    }
    // 更新链路信息表
    bool inLinkTable = false;
    for (auto &i : this->localLinkTable) {
        if (i.L_neighbor_iface_addr == mh.originatorAddress) {
            i.L_ASYM_time = op_sim_time() + mh.vTime;
            for (auto &j : mh.helloMessage.linkMessage) {
                for (auto &k : j.neighborAddress) {
                    if (k == this->nodeId) {
                        if (j.linkcode == LOST_LINK) {
                            i.L_SYM_time = op_sim_time() - 1;
                        }
                        else if (j.linkcode == SYM_LINK || j.linkcode == ASYM_LINK) {
                            i.L_SYM_time = op_sim_time() + mh.vTime;
                            i.L_time = i.L_SYM_time + NEIGHB_HOLD_TIME;
                        }
                    }
                }
            }
            i.L_time = max(i.L_time, i.L_ASYM_time);
            inLinkTable = true;
            // 更新一跳邻居表
            for (auto &j : this->oneHopNeighborTable) {
                if (j.N_neighbor_main_addr == mh.originatorAddress) {
                    if (i.L_SYM_time >= op_sim_time())
                        j.N_status = SYM;
                    else j.N_status = NOT_SYM;
                    break;
                }
            }
            // 更新两跳邻居表
            if (i.L_SYM_time >= op_sim_time()) {
                for (auto &j : mh.helloMessage.linkMessage) {
                    for (auto &k : j.neighborAddress) {
                        if (j.neighcode == SYM_NEIGH || j.neighcode == MPR_NEIGH) {
                            if (k != this->nodeId) {
                                bool in2NeighborTable = false;
                                for (auto &l: this->twoHopNeighborTable)
                                    if (l.N_neighbor_main_addr == mh.originatorAddress && l.N_2hop_addr == k) {
                                        in2NeighborTable = true;
                                        l.N_time = op_sim_time() + mh.vTime;
                                        break;
                                    }
                                if (!in2NeighborTable) {
                                    two_hop_neighbor thnItem;
                                    thnItem.N_neighbor_main_addr = mh.originatorAddress;
                                    thnItem.N_2hop_addr = k;
                                    thnItem.N_time = op_sim_time() + mh.vTime;
                                    this->twoHopNeighborTable.push_back(thnItem);
                                }
                            }
                        }
                        else if (j.neighcode == NOT_NEIGH) {
                            for (vector<two_hop_neighbor>::iterator l = this->twoHopNeighborTable.begin(); l != this->twoHopNeighborTable.end(); ++l) {
                                if (l->N_neighbor_main_addr == mh.originatorAddress && l->N_2hop_addr == k)
                                    this->twoHopNeighborTable.erase(l);
                            }
                        }
                    }
                }
            }
            break;
        }
    }
    if (!inLinkTable) {
        local_link item;
        item.L_local_iface_addr = this->nodeId;
        item.L_neighbor_iface_addr = mh.originatorAddress;
        item.L_SYM_time = op_sim_time() - 1;
        item.L_time = op_sim_time() + mh.vTime;
        item.L_ASYM_time = op_sim_time() + mh.vTime;    //这一部分协议中没有明确规定，推测是这种规则
        this->localLinkTable.push_back(item);
        // 创建一跳邻居表项
        one_hop_neighbor ohnItem;
        ohnItem.N_neighbor_main_addr = mh.originatorAddress;
        this->oneHopNeighborTable.push_back(ohnItem);
    }
    // 更新一跳邻居表
    for (auto &i : this->oneHopNeighborTable) {
        if (i.N_neighbor_main_addr == mh.originatorAddress)
            i.N_willingness = mh.helloMessage.willingness;
    }   
    
    for (auto &i : mh.helloMessage.linkMessage) {
        if (i.neighcode == MPR_NEIGH) {
            bool inMpr = false;
            for (auto &j : this->mprTable)
                if (j.MS_main_addr == mh.originatorAddress) {
                    inMpr = true;
                    j.MS_time = op_sim_time() + mh.vTime;
                }
            if (!inMpr) {
                MPR mprItem;
                mprItem.MS_main_addr = mh.originatorAddress;
                this->mprTable.push_back(mprItem);
            }
        }
    }
}

unsigned int Adnode_ctrl::createMprTable() {
    set<unsigned int> N;
    map<unsigned int, set<unsigned int>> N2;
    map<unsigned int, set<unsigned int>> N_neghbor;
    map<unsigned int, int> dy;
    set<unsigned int> mpr;

    for (auto &i: this->oneHopNeighborTable) {
        N.insert(i.N_neighbor_main_addr);
    }
    
    for (auto &i : this->oneHopNeighborTable) {
        set<unsigned int> tmp;
        for (auto &j : this->twoHopNeighborTable) 
            if (i.N_neighbor_main_addr == j.N_neighbor_main_addr && i.N_willingness != WILL_NEVER && j.N_2hop_addr != this->nodeId && N.find(j.N_2hop_addr) == N.end()) {
                tmp.insert(j.N_2hop_addr);
                if (N2.find(j.N_2hop_addr) == N2.end()) {
                    set<unsigned int> oneNeighbor;
                    oneNeighbor.insert(i.N_neighbor_main_addr);
                    N2.insert(make_pair(j.N_2hop_addr, oneNeighbor));
                }
                else
                    N2[j.N_2hop_addr].insert(i.N_neighbor_main_addr);
            }
        N_neghbor.insert(make_pair(i.N_neighbor_main_addr, tmp));
    }
    
    // 计算D(y)
    for (auto &i : N_neghbor) {
        int count = i.second.size();
        dy.insert(make_pair(i.first, count));
    }
    
    while (true) {
        // 在N2中选择唯一可达的点，将其对应的N加入MPR，删除N2中被MPR覆盖的点
        for(auto &i : N2) {
            if (i.second.size() == 1) {
                unsigned int mprN = *i.second.begin();
                mpr.insert(mprN);
                for (auto &k : N_neghbor[mprN])
                    N2.erase(k);
                N_neghbor.erase(mprN);
            }
        }
        // N2被全覆盖，则完成搜索
        if (N2.empty())
            break;
        else {
            // 从N中选择可达性最小的点，并将其删除
            unsigned int min = INT_MAX;
            unsigned int order;
            for (auto &k: N_neghbor) {
                unsigned int count = 0;
                for (auto &l : k.second) 
                    if (N2.find(l) != N2.end())
                        count++;
                if (count < min) {
                    min = count;
                    order = k.first;
                }
            }
            // 删除前需要将N2中记录的邻接关系删除
            for (auto &k : N_neghbor[order]) {
                map<unsigned int, set<unsigned int>>::iterator it = N2.find(k);
                if (it != N2.end())
                    it->second.erase(k);
            }
            N_neghbor.erase(order);
        }
    } 
    for (auto &i : mpr) {
        MPR mprItem;
        mprItem.MS_main_addr = i;
        this->mprTable.push_back(mprItem);
    }
    return this->mprTable.size();
}

void Adnode_ctrl::handleTc(message_packet mt) {
    for (vector<topology>::iterator it = this->topologyTable.begin(); it != this->topologyTable.end(); ++it) {
        if (it->T_last_addr == mt.originatorAddress) {
            if (it->T_seq < mt.tcMessage.MSSN)
                this->topologyTable.erase(it);
        }
    }
    for (auto &i : mt.tcMessage.MPRSelectorAddresses) {
        bool inTopo = false;
        for (auto &j : this->topologyTable) {
            if (j.T_dest_addr == i && j.T_last_addr == mt.originatorAddress) {
                j.T_time = op_sim_time() + mt.vTime;
                inTopo = true;
                break;
            }
        }
        if (!inTopo) {
            topology topoItem;
            topoItem.T_dest_addr = i;
            topoItem.T_last_addr = mt.originatorAddress;
            topoItem.T_seq = mt.tcMessage.MSSN;
            topoItem.T_time = op_sim_time() + mt.vTime;
            this->topologyTable.push_back(topoItem);
        }
    }
}

void Adnode_ctrl::createRouteTable() {
    this->routeTable.clear();
    
    
}