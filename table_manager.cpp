#pragma once
#include <table_manager.hpp>
#include <set>
#include <map>


using namespace std;
using namespace opnet;

void table_manager::updateLocalLink(message_packet *mh) {
    if (mh->messageType != HELLO)
        return;
    vector<local_link>::iterator it = find(this->localLinkTable.begin(), this->localLinkTable.end(), mh->originatorAddress);
    if (it != this->localLinkTable.end()) {
        local_link item;
        item.L_neighbor_iface_addr = mh->originatorAddress;
        item.L_SYM_time = op_sim_time() - 1;
        item.L_time = op_sim_time() + mh->vTime;
        item.L_ASYM_time = op_sim_time() + mh->vTime;
        this->localLinkTable.push_back(item);
    }
    else {
        it->L_ASYM_time = op_sim_time() + mh->vTime;
        for (auto &i : mh->helloMessage.links) {
            vector<UNINT>::iterator lit = find(i.neighborAddress.begin(), i.neighborAddress.end(), this->nodeId);
            if (lit != i.neighborAddress.end() && i.linkcode == LOST_LINK)
                it->L_SYM_time = op_sim_time() - 1;
            else if (lit != i.neighborAddress.end() && (i.linkcode == SYM_LINK || i.linkcode == ASYM_LINK)) {
                it->L_SYM_time = op_sim_time() + mh->vTime;
                it->L_time = it->L_SYM_time + NEIGHB_HOLD_TIME;
            }
        }
        it->L_time = max(it->L_time, it->L_ASYM_time);
    }
    this->updateOneHop(mh);
    this->updateTwoHop(mh);
}

void table_manager::updateOneHop(message_packet *mh) {
    vector<one_hop_neighbor>::iterator it = find(this->oneHopNeighborTable.begin(), this->oneHopNeighborTable.end(), mh->originatorAddress);
    if (it != this->oneHopNeighborTable.end()) {
        it->N_willingness = mh->helloMessage.willingness;
        for (auto &i : this->localLinkTable)
            if (i.L_neighbor_iface_addr == it->N_neighbor_addr) {
                if (i.L_SYM_time >= op_sim_time())
                    it->N_status = SYM;
                else 
                    it->N_status = NOT_SYM;
                break;
            }
    }
    else {
        one_hop_neighbor item;
        item.N_neighbor_addr = mh->originatorAddress;
        item.N_willingness = mh->helloMessage.willingness;
        for (auto &i : this->localLinkTable)
            if (i.L_neighbor_iface_addr == item.N_neighbor_addr) {
                if (i.L_SYM_time >= op_sim_time())
                    item.N_status = SYM;
                else
                    item.N_status = NOT_SYM;
                this->oneHopNeighborTable.push_back(item);
                break;
            }        
    }
}

void table_manager::updateTwoHop(message_packet *mh) {
    for (auto &i : this->oneHopNeighborTable) {
        if (i.N_status == SYM && i.N_neighbor_addr == mh->originatorAddress) {
            for (auto &j : mh->helloMessage.neighs) {
                if (j.neighcode == MPR_NEIGH || j.neighcode == SYM_NEIGH) {
                    for (auto &k : j.neighborAddress) {
                        if (k != this->nodeId) {
                            bool intable = false;
                            for (auto &l : i.N_2hop)
                                if (k == l.N_2hop_addr) {
                                    l.N_time = op_sim_time() + mh->vTime;
                                    intable = true;
                                    break;
                                }
                            if (!intable) {
                                two_hop_neighbor item;
                                item.N_time = op_sim_time() + mh->vTime;
                                item.N_2hop_addr = k;
                                i.N_2hop.push_back(item);
                            }
                        }
                    }
                }
                else if (j.neighcode == NOT_NEIGH) {
                    for (auto &k : j.neighborAddress)
                        for (vector<two_hop_neighbor>::iterator tit = i.N_2hop.begin(); tit != i.N_2hop.end(); ++tit)
                            if (tit->N_2hop_addr == k)
                                i.N_2hop.erase(tit);
                }
            }
        }
    }
}

UNINT table_manager::createMprTable() {
    set<unsigned int> N;
    map<unsigned int, set<unsigned int>> N2;
    map<unsigned int, set<unsigned int>> N_neghbor;
    map<unsigned int, int> dy;
    set<unsigned int> mpr;

    for (auto &i: this->oneHopNeighborTable) {
        if (i.N_status == SYM)
            N.insert(i.N_neighbor_addr);
    }

    for (auto &i : this->oneHopNeighborTable) {
        if (i.N_status == SYM) {
            set<unsigned int> tmp;
            for (auto &j : i.N_2hop)
                if (i.N_willingness != WILL_NEVER && N.find(j.N_2hop_addr) != N.end() && j.N_2hop_addr != this->nodeId) {
                    tmp.insert(j.N_2hop_addr);
                    if (N2.find(j.N_2hop_addr) == N2.end()) {
                        set<UNINT> oneNeighbor;
                        oneNeighbor.insert(i.N_neighbor_addr);
                        N2.insert(make_pair(j.N_2hop_addr, oneNeighbor));
                    }
                    else
                        N2[j.N_2hop_addr].insert(i.N_neighbor_addr);
                }
            N_neghbor.insert(make_pair(i.N_neighbor_addr, tmp));
        }
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
        mprItem.MS_addr = i;
        this->mprTable.push_back(mprItem);
    }
    return this->mprTable.size();
}

void table_manager::updateTopologyTable(message_packet *mt) {
    if (mt->messageType != TC)
        return;
    for (vector<topology_item>::iterator it = this->topologyTable.begin(); it != this->topologyTable.end(); ++it)
        if (it->T_last_addr == mt->originatorAddress && it->T_seq < mt->tcMessage.MSSN)
            this->topologyTable.erase(it);
    for (auto &i : mt->tcMessage.MPRSelectorAddresses) {
        bool intable = false;
        for (auto &j : this->topologyTable)
            if (j.T_dest_addr == i && j.T_last_addr == mt->originatorAddress) {
                intable = true;
                j.T_time = op_sim_time() + mt->vTime;
                break;
            }
        if (!intable) {
            topology_item item;
            item.T_dest_addr = i;
            item.T_last_addr = mt->originatorAddress;
            item.T_seq = mt->tcMessage.MSSN;
            item.T_time = op_sim_time() + mt->vTime;
            this->topologyTable.push_back(item);
        }
    }
}

void table_manager::getRouteTable() {
    this->routeTable.clear();
    // 加入邻居
    for (auto &i : this->oneHopNeighborTable) {
        if (i.N_status == SYM) {
            route_item item;
            item.R_dest_addr = i.N_neighbor_addr;
            item.R_next_addr = i.N_neighbor_addr;
            item.R_dist = 1;
            this->routeTable.push_back(item);
            if (!i.N_2hop.empty()) {
                for (auto &j : i.N_2hop) {
                    if (j.N_time >= op_sim_time()) {
                        item.R_dest_addr = j.N_2hop_addr;
                        item.R_next_addr = i.N_neighbor_addr;
                        item.R_dist = 2;
                        this->routeTable.push_back(item);
                    }
                }
            }
        }
    }
    // 迭代加入拓扑项
    int h = 2;
    while (1) {
        bool hasNewItem = false;
        for (auto &i : this->topologyTable) {
            bool intable = false;
            for (auto &j: this->routeTable) {
                if (i.T_dest_addr == j.R_dest_addr) {
                    intable = true;
                    break;
                }
            }
            if (!intable) {
                for (auto &j: this->routeTable) {
                    if (j.R_dest_addr == i.T_last_addr && j.R_dist == h) {
                        route_item item;
                        item.R_dest_addr = i.T_dest_addr;
                        item.R_next_addr = j.R_dest_addr;
                        item.R_dist = h + 1;
                        hasNewItem = true;
                    }
                }
            }
            else continue;
        }
        if (!hasNewItem)
            break;
    }
    
}

message_packet* table_manager::getHelloMsg() {
    message_packet* mh;
    mh->vTime = NEIGHB_HOLD_TIME;
    
    return mh;
}

message_packet* table_manager::getTCMsg() {
    message_packet* mt;

    return mt;
}