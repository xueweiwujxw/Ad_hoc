#pragma once
#include <opnet.h>
#include <protocol_simple.hpp>
#include <vector>
#include <table_manager.hpp>

using namespace std;

namespace opnet
{
    class Adnode_ctrl_simple
    {
    private:
        UNINT nodeId;
        table_manager* tm;
        vector<duplicat_set> repeatTable;
        UNINT packetSequenceNumber;
        vector<message_packet> need2Forward;
        UNINT forwardCount;
        vector<pair<double, UNINT>> forwardCountSave;
    public:
        Adnode_ctrl_simple(UNINT nodeId) {
            this->nodeId = nodeId;
            this->tm = new table_manager(this->nodeId);
            this->packetSequenceNumber = 0;
            this->forwardCount = 0;
        }
        ~Adnode_ctrl_simple() {
            delete tm;
        }
        void updateRepeatTable(message_packet mp, UNINT packOri);
        pair<OLSR_packet, UNINT> getOLSRPackets(bool hello, bool tc);
        void recvPackets(OLSR_packet opack);
        void forwardPackets(message_packet mp);
        void updateTMWill(double, calType);
        UNINT getForwardCount() {return this->forwardCount;}
        UNINT getForwardCountSave();
    };
} // namespace opnet
