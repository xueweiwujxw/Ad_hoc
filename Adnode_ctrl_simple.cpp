#pragma once
#include <Adnode_ctrl_simple.hpp>

using namespace std;
using namespace opnet;

void Adnode_ctrl_simple::updateRepeatTable(message_packet* mp) {
    if (mp->TTL <= 0 || mp->originatorAddress == this->nodeId)
        return;
    bool exist = false;
    for (auto &i : this->repeatTable) {
        if (i.D_addr == mp->originatorAddress && i.D_seq_num == mp->messageSequenceNumber) {
            exist = true;
            i.D_time = op_sim_time() + DUP_HOLD_TIME;
            break;
        }
    }
    if (!exist) {            
        duplicat_set item;
        item.D_addr = mp->originatorAddress;
        item.D_seq_num = mp->messageSequenceNumber;
        item.D_retransmitted = false;
        item.D_received = true;
        item.D_time = DUP_HOLD_TIME + op_sim_time();
        this->repeatTable.push_back(item);
        if (mp->messageType == TC)
            tm->updateTopologyTable(mp);
    }
    for (vector<duplicat_set>::iterator it = this->repeatTable.begin(); it != this->repeatTable.end(); ++it)
        if (it->D_time < op_sim_time())
            this->repeatTable.erase(it);
}

void Adnode_ctrl_simple::recvPackets(OLSR_packet* opack) {
    if (opack->messagePackets.empty())
        return;
    for (auto &i : opack->messagePackets)
        this->updateRepeatTable(&i);
}

OLSR_packet* Adnode_ctrl_simple::getOLSRPackets(bool hello, bool tc) {
    OLSR_packet *opack = new OLSR_packet;
    if (hello)
        opack->messagePackets.push_back(*(tm->getHelloMsg()));
    if (tc) {
        message_packet *tmp = tm->getTCMsg();
        if (tmp != nullptr)
            opack->messagePackets.push_back(*tmp);
    }
    opack->packetLenth = 4;
    opack->packetSequenceNumber = this->packetSequenceNumber++;
    for (auto &i : opack->messagePackets)
        opack->packetLenth += i.getSize();
    return opack;
}

void Adnode_ctrl_simple::scheduleSelf(double interval, int code) {
    op_intrpt_schedule_self(op_sim_time() + interval, code);
}