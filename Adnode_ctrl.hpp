#pragma once
#include <opnet.h>
#include <protocol.h>
#include <iostream>
#include <vector>

using namespace std;

namespace opnet
{
    class Adnode_ctrl
    {
    private:
        unsigned int nodeId;
        vector<unsigned int> portId;
        vector<local_link> localLinkTable;
        vector<one_hop_neighbor> oneHopNeighborTable;
        vector<two_hop_neighbor> twoHopNeighborTable;
        vector<MPR_table> mprTable;
        vector<TC_repeat_table> tcRepeatTable;
        vector<topology_table> topologyTable;
        vector<route_table> routeTable;
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
        void handleLocalLinkTable();
        void handleOneHopNeighborTable();
        void handleTwoHopNeighborTable();
        unsigned int handleMprTable();
        void handleTcRepeatTable();
        void handleTopologyTable();
        void handleRouteTable();
    };

} // namespace opnet