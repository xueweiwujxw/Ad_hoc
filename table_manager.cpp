#pragma once
#include <table_manager.hpp>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace opnet;

void table_manager::updateLocalLink(message_packet mh) {
    // cout << this->nodeId << " start " << endl;
    if (mh.messageType != HELLO)
        return;
    vector<local_link>::iterator it;
    for (it = this->localLinkTable.begin(); it != this->localLinkTable.end(); ++it)
        if (it->L_neighbor_iface_addr == mh.originatorAddress)
            break;
    if (it == this->localLinkTable.end()) {
        local_link item;
        item.L_neighbor_iface_addr = mh.originatorAddress;
        item.L_SYM_time = op_sim_time() - 1;
        item.L_time = op_sim_time() + mh.vTime;
        item.L_ASYM_time = op_sim_time() + mh.vTime;
        this->localLinkTable.push_back(item);
    }
    else {
        it->L_ASYM_time = op_sim_time() + mh.vTime;
        for (auto &i : mh.helloMessage.links) {
            vector<UNINT>::iterator lit = find(i.neighborAddress.begin(), i.neighborAddress.end(), this->nodeId);
            if (lit != i.neighborAddress.end() && i.linkcode == LOST_LINK)
                it->L_SYM_time = op_sim_time() - 1;
            else if (lit != i.neighborAddress.end() && (i.linkcode == SYM_LINK || i.linkcode == ASYM_LINK)) {
                it->L_SYM_time = op_sim_time() + mh.vTime;
                it->L_time = it->L_SYM_time + NEIGHB_HOLD_TIME;
            }
        }
        it->L_time = max(it->L_time, it->L_ASYM_time);
    }
    // cout << this->nodeId << " end " << endl;
    // cout << "update local link procedure end " << endl;
    this->updateOneHop(mh);
    this->updateTwoHop(mh);
    this->updateMprTable(mh);
}

void table_manager::updateOneHop(message_packet mh) {
    vector<one_hop_neighbor>::iterator it;
    for (it = this->oneHopNeighborTable.begin(); it != this->oneHopNeighborTable.end(); ++it) {
        if (it->N_neighbor_addr == mh.originatorAddress)
            break;
    }
    if (it != this->oneHopNeighborTable.end()) {
        it->N_willingness = mh.helloMessage.willingness;
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
        item.N_neighbor_addr = mh.originatorAddress;
        item.N_willingness = mh.helloMessage.willingness;
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
    // cout << "update one hop" << endl;
}

void table_manager::updateTwoHop(message_packet mh) {
    for (auto &i : this->oneHopNeighborTable) {
        if (i.N_status == SYM && i.N_neighbor_addr == mh.originatorAddress) {
            for (auto &j : mh.helloMessage.neighs) {
                if (j.neighcode == MPR_NEIGH || j.neighcode == SYM_NEIGH) {
                    for (auto &k : j.neighborAddress) {
                        if (k != this->nodeId) {
                            bool intable = false;
                            for (auto &l : i.N_2hop)
                                if (k == l.N_2hop_addr) {
                                    l.N_time = op_sim_time() + mh.vTime;
                                    intable = true;
                                    break;
                                }
                            if (!intable) {
                                two_hop_neighbor item;
                                item.N_time = op_sim_time() + mh.vTime;
                                item.N_2hop_addr = k;
                                i.N_2hop.push_back(item);
                            }
                        }
                    }
                }
                else if (j.neighcode == NOT_NEIGH) {
                    for (auto &k : j.neighborAddress)
                        for (vector<two_hop_neighbor>::iterator tit = i.N_2hop.begin(); tit != i.N_2hop.end(); ++tit)
                            if (tit->N_2hop_addr == k) {
                                i.N_2hop.erase(tit);
                                tit--;
                            }
                }
            }
        }
    }
    // cout << "update two hop" << endl;
}

void table_manager::updateMprTable(message_packet mh) {
    for (auto &i : mh.helloMessage.neighs) {
        if (i.neighcode == MPR_NEIGH) {
            // cout << i.neighborAddress.size() << endl;
            for (auto &k : i.neighborAddress) {
                // cout << this->nodeId << " " << k << endl;
                if (k == this->nodeId) {
                    bool intable = false;
                    for (auto &j : this->mprTable)
                        if (j.MS_addr == mh.originatorAddress) {
                            intable = true;
                            j.MS_time = op_sim_time() + mh.vTime;
                            break;
                        }
                    if (!intable) {
                        MPR item;
                        item.MS_addr = mh.originatorAddress;
                        item.MS_time = op_sim_time() + mh.vTime;
                        this->mprTable.push_back(item);
                    }
                }
            }
            
        }
    }
    // cout << this->mprTable.size() << endl;
    // cout << "update mpt table" << endl;
}

UNINT table_manager::createMprSet() {
    // cout << "start create mpr set" << endl;
    this->mprSet.clear();
    set<unsigned int> N;
    map<unsigned int, set<unsigned int>> N2;
    map<unsigned int, set<unsigned int>> N_neghbor;
    map<unsigned int, unsigned int> N_will;

    for (auto &i: this->oneHopNeighborTable) {
        if (i.N_status == SYM) {
            N.insert(i.N_neighbor_addr);
            N_will.insert(make_pair(i.N_neighbor_addr, i.N_willingness));
        }
    }
    for (auto &i : this->oneHopNeighborTable) {
        if (i.N_status == SYM) {
            set<unsigned int> tmp;
            for (auto &j : i.N_2hop)
                if (i.N_willingness != WILL_NEVER && N.find(j.N_2hop_addr) == N.end() && j.N_2hop_addr != this->nodeId) {
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
    // cout << "this node id: " << this->nodeId << endl;
    // cout << "N set: " << endl;
    // for (auto &xx: N_neghbor) {
    //     cout << xx.first << ": ";
    //     for (auto &k : xx.second)
    //         cout << k << " ";
    //     cout << endl;
    // }
    // cout << endl << "N2 set: " << endl;
    // for (auto &xx : N2) {
    //     cout << xx.first << ": ";
    //     for (auto &k : xx.second)
    //         cout << k << " ";
    //     cout << endl;
    // }
    // cout << endl;
    while (!N2.empty()) {
        // 在N2中选择唯一可达的点，将其对应的N加入MPR，删除N2中被MPR覆盖的点
        for(auto &i : N2) {
            if (i.second.size() == 1) {
                unsigned int mprN = *i.second.begin();
                this->mprSet.insert(mprN);
                for (auto &k : N_neghbor[mprN]) {
                    N2.erase(k);
                }
                N_neghbor.erase(mprN);
            }
        }
        for (auto &i : N_neghbor) {
            for (auto &j : i.second) {
                bool inN2 = false;
                for (auto &k : N2)
                    if (j == k.first) {
                        inN2 = true;
                        break;
                    }
                if (!inN2)
                    i.second.erase(j);
            }
        }
        if (N2.empty())
            break;
        // 从N中选择可达性最小的点，并将其删除，可达性相同时，删除willingness最小的点
        unsigned int minCount = INT_MAX;
        unsigned int order;
        set<unsigned int> equalSet;
        for (auto &k: N_neghbor) {
            if (k.second.size() < minCount) {
                minCount = k.second.size();
                order = k.first;
                equalSet.clear();
                equalSet.insert(k.first);
            }
            else if (k.second.size() == minCount) {
                equalSet.insert(k.first);
            }
        }
        if (equalSet.size() > 1) {
            UNINT willing = 6;
            for (auto &i : equalSet) {
                if (N_will[i] < willing) {
                    willing = N_will[i];
                    order = i;
                }
            }
        }
        for (auto &k : N2) {
            if (k.second.find(order) != k.second.end())
                k.second.erase(order);
        }
        N_neghbor.erase(order);
    }
    // cout << "end create mpr set" << endl;
    // fstream f;
    // string filename = to_string(this->nodeId) + "mpr.txt";
    // f.open(filename, ios::out | ios::app);
    // f << "node " << this->nodeId << " mpr set: " << op_sim_time() << endl;
    // for (auto &i : this->mprSet) 
    //     f << i << " ";
    // f << endl;
    // f.close();
    return this->mprSet.size();
}

UNINT table_manager::createOldMprSet() {
    // cout << "start create mpr set" << endl;
    this->mprSet.clear();
    set<unsigned int> N;
    map<unsigned int, set<unsigned int>> N2;
    map<unsigned int, set<unsigned int>> N_neghbor;
    map<unsigned int, int> dy;
    map<unsigned int, unsigned int> N_will;

    for (auto &i: this->oneHopNeighborTable) {
        if (i.N_status == SYM) {
            N.insert(i.N_neighbor_addr);
            N_will.insert(make_pair(i.N_neighbor_addr, i.N_status));
        }
    }

    for (auto &i : this->oneHopNeighborTable) {
        if (i.N_status == SYM) {
            set<unsigned int> tmp;
            for (auto &j : i.N_2hop)
                if (i.N_willingness != WILL_NEVER && N.find(j.N_2hop_addr) == N.end() && j.N_2hop_addr != this->nodeId) {
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

    for (auto &i : N_neghbor) {
        int count = i.second.size();
        dy.insert(make_pair(i.first, count));
    }
    // cout << "this node id: " << this->nodeId << endl;
    // cout << "N set: " << endl;
    // for (auto &xx: N_neghbor) {
    //     cout << xx.first << ": ";
    //     for (auto &k : xx.second)
    //         cout << k << " ";
    //     cout << endl;
    // }
    // cout << endl << "N2 set: " << endl;
    // for (auto &xx : N2) {
    //     cout << xx.first << ": ";
    //     for (auto &k : xx.second)
    //         cout << k << " ";
    //     cout << endl;
    // }
    // cout << endl;
    while (!N2.empty()) {
        // 在N2中选择唯一可达的点，将其对应的N加入MPR，删除N2中被MPR覆盖的点
        for(auto &i : N2) {
            if (i.second.size() == 1) {
                unsigned int mprN = *i.second.begin();
                this->mprSet.insert(mprN);
                for (auto &k : N_neghbor[mprN]) {
                    N2.erase(k);
                }
                N_neghbor.erase(mprN);
            }
        }
        if (N2.empty())
            break;
        int maxCount = 0;
        int mprNode;
        set<unsigned int> equalSet;
        for (auto &i : N_neghbor) {
            unsigned int count = 0;
            for (auto &l : i.second)
                if (N2.find(l) != N2.end())
                    count++;
            if (count > maxCount) {
                maxCount = count;
                mprNode = i.first;
                equalSet.clear();
                equalSet.insert(i.first);
            }
            else if (count == maxCount) {
                equalSet.insert(i.first);
            }
        }
        if (equalSet.size() > 1) {
            int maxDy = -1;
            for (auto &i : equalSet) {
                if (dy[i] > maxDy) {
                    maxDy = dy[i];
                    mprNode = i;
                }
            }
        }
        if (equalSet.size() == 0) {
            break;
        }
        this->mprSet.insert(mprNode);
        for (auto &k : N_neghbor[mprNode]) {
            N2.erase(k);
        }
        N_neghbor.erase(mprNode);
    }
    // cout << "end create mpr set" << endl;
    // fstream f;
    // string filename = to_string(this->nodeId) + "oldmpr.txt";
    // f.open(filename, ios::out | ios::app);
    // f << "node " << this->nodeId << " mpr set: " << op_sim_time() << endl;  
    // for (auto &i : this->mprSet) 
    //     f << i << " ";
    // f << endl;
    // f.close();

    return this->mprSet.size();
}

void table_manager::updateTopologyTable(message_packet mt) {
    if (mt.messageType != TC)
        return;
    for (vector<topology_item>::iterator it = this->topologyTable.begin(); it != this->topologyTable.end(); ++it)
        if (it->T_last_addr == mt.originatorAddress && it->T_seq < mt.tcMessage.MSSN) {
            this->topologyTable.erase(it);
            it--;
        }
    for (auto &i : mt.tcMessage.MPRSelectorAddresses) {
        bool intable = false;
        for (auto &j : this->topologyTable)
            if (j.T_dest_addr == i && j.T_last_addr == mt.originatorAddress) {
                intable = true;
                j.T_time = op_sim_time() + mt.vTime;
                break;
            }
        if (!intable) {
            topology_item item;
            item.T_dest_addr = i;
            item.T_last_addr = mt.originatorAddress;
            item.T_seq = mt.tcMessage.MSSN;
            item.T_time = op_sim_time() + mt.vTime;
            this->topologyTable.push_back(item);
        }
    }
    // cout << "node: " << this->nodeId << endl;
    // for (auto &i : this->topologyTable) 
    //     cout << i.T_dest_addr << " " << i.T_last_addr << " " << i.T_seq << " " << i.T_time << endl;
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
    // cout << "start loop " << endl;
    // cout << "before topo size: " << this->topologyTable.size() << endl;
    for (auto &i : this->topologyTable)
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
                        item.R_dist = h+1;
                        hasNewItem = true;
                        this->routeTable.push_back(item);
                    }
                }
            }
        }
        h++;
        if (!hasNewItem)
            break;
    }
    // cout << "after topo size: " << this->topologyTable.size() << endl;
    // // cout << "end loop " << endl;
    // cout << "node: " << this->nodeId << endl;
    // for (auto &i : this->routeTable) {
    //     cout << i.R_dest_addr << " " << i.R_next_addr << " " << i.R_dist << endl;
    // }
}

message_packet table_manager::getHelloMsg() {
    message_packet mh(HELLO, NEIGHB_HOLD_TIME, this->nodeId, 1, 0, this->messageSequenceNumber++);
    link_status lsas(ASYM_LINK);
    link_status lss(SYM_LINK);
    link_status lslo(LOST_LINK);
    neigh_status nsnot(NOT_NEIGH);
    neigh_status nssym(SYM_NEIGH);
    neigh_status nsmpr(MPR_NEIGH);
    for (auto &i : this->localLinkTable) {
        if (i.L_SYM_time >= op_sim_time()) 
            lss.neighborAddress.push_back(i.L_neighbor_iface_addr);
        else if (i.L_ASYM_time >= op_sim_time())
            lsas.neighborAddress.push_back(i.L_neighbor_iface_addr);
        else 
            lslo.neighborAddress.push_back(i.L_neighbor_iface_addr);
        bool inmpr = false, inneigh = false;
        for (auto &j : this->mprSet) {
            if (j == i.L_neighbor_iface_addr) {
                inmpr = true;
                break;
            }
        }
        for (auto &j : this->oneHopNeighborTable) {
            if (j.N_neighbor_addr == i.L_neighbor_iface_addr && j.N_status == SYM) {
                inneigh = true;
                break;
            }
        }
        if (inmpr)
            nsmpr.neighborAddress.push_back(i.L_neighbor_iface_addr);
        else if (inneigh)
            nssym.neighborAddress.push_back(i.L_neighbor_iface_addr);
        else
            nsnot.neighborAddress.push_back(i.L_neighbor_iface_addr);
    }
    message_hello mph;
    // cout << nsmpr.neighborAddress.size() << " " << nssym.neighborAddress.size() << " " << nsnot.neighborAddress.size() << endl;
    if (!lslo.neighborAddress.empty()) 
        mph.links.push_back(lslo);
    if (!lsas.neighborAddress.empty())
        mph.links.push_back(lsas);
    if (!lss.neighborAddress.empty())
        mph.links.push_back(lss);
    if (!nsnot.neighborAddress.empty())
        mph.neighs.push_back(nsnot);
    if (!nssym.neighborAddress.empty())
        mph.neighs.push_back(nssym);
    if (!nsmpr.neighborAddress.empty())
        mph.neighs.push_back(nsmpr);
    mph.willingness = this->willSelf;
    mh.helloMessage = mph;
    mh.messageSize = mh.getSize();
    return mh;
}

message_packet table_manager::getTCMsg() {
    message_packet mt(TC, TOP_HOLD_TIME, this->nodeId, 255, 0, this->messageSequenceNumber++);
    message_tc mpt;
    // if (this->mprTable.empty())
        // cout << this->nodeId << " " << op_sim_time() << ": mprTable empty, mprSet size: " << this->mprSet.size() << endl;
    mpt.MSSN = this->MSSN++;
    for (auto &i : this->mprTable) {
        if (i.MS_time >= op_sim_time()) {
            mpt.MPRSelectorAddresses.push_back(i.MS_addr);
        }
    }
    mt.tcMessage = mpt;
    mt.messageSize = mt.getSize();
    return mt;
}

void table_manager::freshTables() {
    // cout << this->nodeId << " fresh start " << endl;
    for (vector<local_link>::iterator it = this->localLinkTable.begin(); it != this->localLinkTable.end(); ++it)
        if (it->L_time < op_sim_time()) {
            for (vector<one_hop_neighbor>::iterator oit = this->oneHopNeighborTable.begin(); oit != this->oneHopNeighborTable.end(); ++oit) 
                if (oit->N_neighbor_addr == it->L_neighbor_iface_addr) {
                    this->oneHopNeighborTable.erase(oit);
                    oit--;
                }
            this->localLinkTable.erase(it);
            it--;
        }
    for (auto &i : this->oneHopNeighborTable) 
        for (vector<two_hop_neighbor>::iterator it = i.N_2hop.begin(); it != i.N_2hop.end(); ++it)
            if (it->N_time < op_sim_time()) {
                i.N_2hop.erase(it);
                it--;
            }
    for (vector<MPR>::iterator it = this->mprTable.begin(); it != this->mprTable.end(); ++it)
        if (it->MS_time < op_sim_time()) {
            this->mprTable.erase(it);
            it--;
        }
    for (vector<topology_item>::iterator it = this->topologyTable.begin(); it != this->topologyTable.end(); ++it)
        if (it->T_time < op_sim_time()) {
            this->topologyTable.erase(it);
            it--;
        }
    // cout << this->nodeId << " fresh end " << endl;
}

bool table_manager::mprEmpty() {
    return this->mprTable.empty();
}

void table_manager::print() {
    cout << this->nodeId << endl;
    // for (auto &i : this->localLinkTable)
    //     printf("%-3d %-3d %-3d %-3d\n", i.L_neighbor_iface_addr, i.L_SYM_time, i.L_ASYM_time, i.L_time);
    // for (auto &i : this->oneHopNeighborTable) {
    //     printf("%-3d %-3d %-3d\n", i.N_neighbor_addr, i.N_status, i.N_willingness);
    //     for (auto &j : i.N_2hop) 
    //         printf("%-3d %-3d\n", j.N_2hop_addr, j.N_time);
    // }
    cout << this->localLinkTable.size() << " " << this->oneHopNeighborTable.size() << endl;
    for (auto &i : this->oneHopNeighborTable)
        cout << i.N_2hop.size() << " ";
    cout << endl;
}

void table_manager::updateWill(double res, calType ct) {
    this->willSelf = WILL_DEFAULT;
    // cout << op_sim_time() << " node " << this->nodeId << ": " << res << endl;
}

bool table_manager::isInMprTable(UNINT findNode) {
    for (auto & i : this->mprTable) {
        if (i.MS_addr == findNode)
            return true;
    }
    return false;
}