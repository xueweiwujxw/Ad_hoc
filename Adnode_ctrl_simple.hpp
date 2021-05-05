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
    public:
        Adnode_ctrl_simple(UNINT nodeId) {
            this->nodeId = nodeId;
            this->tm = new table_manager(this->nodeId);
            this->packetSequenceNumber = 0;
        }
        ~Adnode_ctrl_simple() {
            delete tm;
        }
        void updateRepeatTable(message_packet* mp);
        OLSR_packet* getOLSRPackets(bool hello, bool tc);
        void recvPackets(OLSR_packet* opack);
        void forwardPackets(message_packet* mp);
        void scheduleSelf(double interval, int code);
    };
} // namespace opnet
