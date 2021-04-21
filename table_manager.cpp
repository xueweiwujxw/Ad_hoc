#pragma once
#include <table_manager.hpp>
#include <set>
#include <map>


using namespace std;
using namespace opnet;

UNINT table_manager::createMprTable() {
    set<unsigned int> N;
    map<unsigned int, set<unsigned int>> N2;
    map<unsigned int, set<unsigned int>> N_neghbor;
    map<unsigned int, int> dy;
    set<unsigned int> mpr;

    for (auto &i: this->oneHopNeighborTable) {
        N.insert(i.N_neighbor_addr);
    }

    for (auto &i : this->oneHopNeighborTable) {
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