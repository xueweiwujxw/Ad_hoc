#pragma once
#include <opnet.h>
#include <protocol.h>
#include <iostream>
#include <vector>
#include <set>


using namespace std;

namespace opnet
{
    class Adnode_ctrl
    {
    private:
        unsigned int nodeId;
        set<local_link> localLinkTable;
        set<one_hop_neighbor> oneHopNeighborTable;
        set<two_hop_neighbor> twoHopNeighborTable;
        set<MPR> mprTable;
        set<duplicate_set> repeatTable;
        set<topology> topologyTable;
        set<route> routeTable;
        unsigned int MSSN;
    public:
        Adnode_ctrl();
        ~Adnode_ctrl();
        int run();
        void start();
        void stop();
        void send(message_packet ms);
    protected:
        message_hello createHello();
        message_tc createTC();
        void recvPacket(void *data);
        void forward(set<message_packet> needToForward);
        unsigned int createMprTable();
        void handleHello(message_packet mh);
        void handleTc(message_packet mt);
    };

    
        

} // namespace opnet