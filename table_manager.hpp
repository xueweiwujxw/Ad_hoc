#pragma once
#include <protocol_simple.hpp>
#include <vector>
#include <algorithm>
#include <opnet.h>
#include <set>

using namespace std;

namespace opnet
{
    class table_manager
    {
    private:
        vector<local_link> localLinkTable;
        vector<one_hop_neighbor> oneHopNeighborTable;
        vector<two_hop_neighbor> twoHopNeighborTable;
        set<UNINT> mprSet;
        vector<MPR> mprTable;
        vector<topology_item> topologyTable;
        vector<route_item> routeTable;
        UNINT MSSN;
        UNINT messageSequenceNumber;
        UNINT packetSequenceNumber;
        int nodeId;
        UNINT willSelf;
    public:
        table_manager(int nodeId) : MSSN(0), messageSequenceNumber(0), packetSequenceNumber(0), nodeId(nodeId), willSelf(WILL_DEFAULT) {}
        ~table_manager() {}
        void updateLocalLink(message_packet mh);
        UNINT createMprSet(); 
        UNINT createOldMprSet();
        void updateTopologyTable(message_packet mt);
        void getRouteTable();
        void freshTables();
        message_packet getHelloMsg();
        message_packet getTCMsg();
        bool mprEmpty();
        void print();
        void updateWill(double, calType);
        bool isInMprTable(UNINT);
    protected:
        void updateMprTable(message_packet mh);
        void updateOneHop(message_packet mh);
        void updateTwoHop(message_packet mh);
    };
} // namespace opnet
