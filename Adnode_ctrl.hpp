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
        vector<local_link> localLinkTable;
        vector<one_hop_neighbor> oneHopNeighborTable;
        vector<two_hop_neighbor> twoHopNeighborTable;
        vector<MPR> mprTable;
        vector<duplicate_set> repeatTable;
        vector<topology> topologyTable;
        vector<route> routeTable;
        unsigned int MSSN;
        unsigned int messageSequenceNumber;
        unsigned int packetSequenceNumber;
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
        olsr_packet createPacket(bool helloEnable, bool tcEnable);
        void recvPacket(void *data);
        void forward(vector<message_packet> needToForward);
        unsigned int createMprTable();
        void handleHello(message_packet mh);
        void handleTc(message_packet mt);
    };

    
        

} // namespace opnet