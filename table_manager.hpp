#pragma once
#include <protocol_simple.hpp>
#include <vector>
#include <algorithm>
#include <opnet.h>

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
        void updateLocalLink(message_packet *mh);
        UNINT createMprTable(); 
        void updateTopologyTable(message_packet *mt);
        void getRouteTable();
        void freshTables();
        message_packet* getHelloMsg();
        message_packet* getTCMsg();
    protected:
        void updateOneHop(message_packet *mh);
        void updateTwoHop(message_packet *mh);
    };
} // namespace opnet
