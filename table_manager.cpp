#pragma once
#include <table_manager.hpp>

using namespace std;
using namespace opnet;

vector<local_link> local_link_table::getTable() {
    return this->localLinkTable;
}

vector<one_hop_neighbor> one_hop_neighbor_table::getTable() {
    return this->oneHopNeighborTable;
}

vector<two_hop_neighbor> two_hop_neighbor_table::getTable() {
    return this->twoHopNeighborTable;
}

vector<MPR> mpr_table::getTable() {
    return this->mprtable;
}

vector<TC_repeat> tc_repeat_table::getTable() {
    return this->tcRepeatTable;
}

vector<topology> topology_table::getTable() {
    return this->topologyTable;
}

vector<route> route_table::getTable() {
    return this->routeTable;
}