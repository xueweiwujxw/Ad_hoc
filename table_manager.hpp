#pragma once
#include <protocol_simple.hpp>
#include <vector>

using namespace std;

namespace opnet
{
    class table_manager
    {
    private:
        vector<local_link> localLinkTable;
        vector<one_hop_neighbor> oneHopNeighborTable;
        vector<two_hop_neighbor> twoHopNeighborTable;
        vector<MPR> mprTable;
        vector<duplicat_set> repeatTable;
        vector<topology_item> topologyTable;
        vector<route_item> routeTable;
        UNINT MSSN;
        UNINT messageSequenceNumber;
        UNINT packetSequenceNumber;
        int nodeId;
    public:
        table_manager(int nodeId) : MSSN(0), messageSequenceNumber(0), packetSequenceNumber(0), nodeId(nodeId) {}
        ~table_manager() {}
        void updateLocalLink();
        void updateOneHop();
        void updateTwoHop();
        UNINT createMprTable(); 
    };
} // namespace opnet
