#pragma once
#include <opnet.h>
#include <protocol.h>
#include <iostream>
#include <vector>
#include <table_manager.hpp>

using namespace std;

namespace opnet
{
    class Adnode_ctrl
    {
    private:
        unsigned int nodeId;
        vector<unsigned int> portId;
        local_link_table localLinkTable;
        one_hop_neighbor_table oneHopNeighborTable;
        two_hop_neighbor_table twoHopNeighborTable;
        mpr_table mprTable;
        tc_repeat_table tcRepeatTable;
        topology_table topologyTable;
        route_table routeTable;
    public:
        Adnode_ctrl();
        ~Adnode_ctrl();
        int run();
        void start();
        void stop();
    protected:
        message_hello createHello();
        message_tc createTC();
        void recvMessage(void *data);
        unsigned int createMprTable();
    };

    
        

} // namespace opnet