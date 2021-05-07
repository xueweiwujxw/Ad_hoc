#pragma once
#include <Adnode_ctrl_simple.hpp>
#include <iostream>

using namespace std;
using namespace opnet;

void Adnode_ctrl_simple::updateRepeatTable(message_packet mp) {
    if (mp.TTL <= 0 || mp.originatorAddress == this->nodeId) {
        // cout << "direc return" << endl;
        // cout << mp.TTL << endl;
        return;
    }
    // cout << op_node_id() << " repeat " << this->packetSequenceNumber << endl;
    bool exist = false;
    for (auto &i : this->repeatTable) {
        if (i.D_addr == mp.originatorAddress && i.D_seq_num == mp.messageSequenceNumber) {
            cout << "in this repeatTable" << endl;
            exist = true;
            i.D_time = op_sim_time() + DUP_HOLD_TIME;
            if (i.D_retransmitted == false && mp.messageType != HELLO) {
                this->need2Forward.push_back(mp);
                i.D_retransmitted = true;
            }
            break;
        }
    }
    // cout << op_node_id() << " end loop" << endl;
    if (!exist) {            
        duplicat_set item;
        item.D_addr = mp.originatorAddress;
        item.D_seq_num = mp.messageSequenceNumber;
        item.D_retransmitted = false;
        item.D_received = true;
        item.D_time = DUP_HOLD_TIME + op_sim_time();
        this->repeatTable.push_back(item);
        if (mp.messageType == TC) {
            cout << op_node_id() << " ";
            cout << "start process TC" << endl;
            tm->updateTopologyTable(mp);
        }
        else if (mp.messageType == HELLO) {
            // cout << op_node_id() << " ";
            // cout << "start process HELLO from: " << mp.originatorAddress << endl;
            tm->updateLocalLink(mp);
            // cout << "update local link end" << endl;
            // tm->createMprSet();
            // cout << "create mpr set end" << endl;
            // tm->getRouteTable();
            // cout << "create route table end" << endl;
            // cout << "process HELLO end" << endl;
        }
    }
    tm->freshTables();
    for (vector<duplicat_set>::iterator it = this->repeatTable.begin(); it != this->repeatTable.end(); ++it)
        if (it->D_time < op_sim_time()) {
            cout << "erase happened" << endl;
            this->repeatTable.erase(it);
            it--;
        }
    // cout << this->repeatTable.size() << endl;
    // tm->print();
    // cout << "end repeat" << endl;
}

void Adnode_ctrl_simple::recvPackets(OLSR_packet opack) {
    // if (opack->packetLenth == 4)
    //     return;
    for (auto &i : opack.messagePackets)
        this->updateRepeatTable(i);
}

pair<OLSR_packet, UNINT> Adnode_ctrl_simple::getOLSRPackets(bool hello, bool tc) {
    // cout << op_node_id() << " get OLSR packets: ";
    OLSR_packet opack;
    if (hello) {
        // cout << " HELLO ";// << endl;
        opack.messagePackets.push_back(tm->getHelloMsg());
    }
    if (tc) {
        // cout << " TC ";// << endl;
        message_packet tmp = tm->getTCMsg();
        if (!tm->mprEmpty())
            opack.messagePackets.push_back(tmp);
    }
    if (!this->need2Forward.empty()) {
        // cout << " FORWARD ";// << endl;
        for (auto &i : this->need2Forward)
            opack.messagePackets.push_back(i);
    }
    // cout << " get packets success" << endl;
    // opack->packetLenth = 4;
    opack.packetSequenceNumber = this->packetSequenceNumber++;
    return make_pair<OLSR_packet&, UNINT>(opack, opack.getSize()*4);
}

void Adnode_ctrl_simple::scheduleSelf(double interval, int code) {
    op_intrpt_schedule_self(op_sim_time() + interval, code);
}