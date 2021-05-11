#include <opnet.h>
#include <opnet_ctrller.hpp>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace opnet;
#define pksize 1024

opnet_ctrller::opnet_ctrller() {    
    // this->packetSequence = 0;
    this->packetCount = 0;
    this->resId = 0;
}
opnet_ctrller::~opnet_ctrller() { 
    delete acs;
}


void opnet_ctrller::schedule_self(double interval, UNINT code) {
    // cout << op_node_id() << " node: " << op_sim_time()+interval << " " << code << endl;
    op_intrpt_schedule_self(op_sim_time() + interval, code);
}

void opnet_ctrller::send(OLSR_packet data, UNINT len, UNINT type) {
    // cout << op_sim_time() <<  " node: " << op_node_id() << endl;
    if (type == OPC_HELLO_SEND)
        this->schedule_self(HELLO_INTERVAL, OPC_HELLO_SEND);
    if (type == OPC_TC_SEND)
        this->schedule_self(TC_INTERVAL, OPC_TC_SEND);
    int needSpace = data.getSize();
    UNINT *format = new UNINT[needSpace];
    int mark = 0;
    format[mark++] = data.packOri;
    format[mark++] = (data.packetLenth << 16) | (data.packetSequenceNumber & 0xffff);
    for (auto &i : data.messagePackets) {
        format[mark++] = (i.messageType << 24) | ((i.vTime & 0xff) << 16) |  (i.messageSize & 0xffff);
        format[mark++] = i.originatorAddress;
        format[mark++] = (i.TTL << 24) | ((i.hopCount & 0xff) << 16) | (i.messageSequenceNumber & 0xffff);
        if (i.messageType == HELLO)  {
            format[mark++] = i.helloMessage.willingness;
            format[mark++] = (i.helloMessage.linkNum << 16) | (i.helloMessage.neighNum & 0xffff);
            for (auto &j : i.helloMessage.links) {
                format[mark++] = (j.linkcode << 16) | (j.num & 0xffff);
                for (auto &k : j.neighborAddress)
                    format[mark++] = k;
            }
            for (auto &j : i.helloMessage.neighs) {
                format[mark++] = (j.neighcode << 16) | (j.num & 0xffff);
                for (auto &k : j.neighborAddress)
                    format[mark++] = k;
            }
        }
        else if (i.messageType == TC) {
            format[mark++] = (i.tcMessage.MSSN << 16) | (i.tcMessage.num & 0xffff);
            for (auto &j : i.tcMessage.MPRSelectorAddresses)
                format[mark++] = j;
        }
        else if (i.messageType == NORMAL) {
            format[mark++] = i.normalMessage.id;
            format[mark++] = i.normalMessage.time;
        }
    }
    for (auto &i : data.messagePackets) {
        // if (type != i.messageType)
        //     cout << "type error" << endl;
        // else 
        //     cout << "type right" << endl;
        results item(this->resId, op_sim_time(), "SEND", i.originatorAddress, i.messageSequenceNumber, -1, -1, -1, -1, i.messageType);
        this->resId++;
        this->res.push_back(item);
    }
    // cout << op_sim_time() << ": build packets end" << endl;
    // cout << op_node_id() << " send packet " << data.packetSequenceNumber << endl;
    // cout << mark << endl;
    Packet *p = op_pk_create(0);
    void *custom = op_prg_mem_alloc(len);
    op_prg_mem_copy(format, custom, len);
    delete format;
    op_pk_fd_set_ptr(p, 0, custom, 8 * len, op_prg_mem_copy_create, op_prg_mem_free, len);
    op_pk_send(p, 0);
}

void opnet_ctrller::on_sim_start() {
    // cout << "node " << op_node_id() << " start at time : " << op_sim_time() << endl;
    int nodeId = op_node_id();
    this->acs = new Adnode_ctrl_simple(op_node_id());
    this->schedule_self(nodeId / 20.0, OPC_HELLO_SEND);
    this->schedule_self(nodeId / 20.0+0.005, OPC_TC_SEND);
    this->schedule_self(nodeId / 20.0+0.001+WILL_UP_TIME, OPC_QUA_CAL);
}

void opnet_ctrller::on_self() {
    // cout << op_sim_time() << " node: " << op_node_id() << endl;
    pair<OLSR_packet, UNINT> data;
    if (op_intrpt_code() == OPC_TC_SEND) {
        data = this->acs->getOLSRPackets(0, 1);
        this->send(data.first, data.second, OPC_TC_SEND);
        // cout << "send TC" << endl;
    }
    else if (op_intrpt_code() == OPC_HELLO_SEND) {
        data = this->acs->getOLSRPackets(1, 0);
        this->send(data.first, data.second, OPC_HELLO_SEND);
        // cout << "send HELLO" << endl;
    }
    else if (op_intrpt_code() == OPC_QUA_CAL) {
        this->cacluate(TYPE_BER);
    }
}

void opnet_ctrller::cacluate(calType ct) {
    this->schedule_self(WILL_UP_TIME, OPC_QUA_CAL);
    // cout << "WILL_UP_TIME: " << WILL_UP_TIME << endl;
    // 时间权重分配
    double weigh[WILL_UP_TIME];
    if (WILL_UP_TIME & 1 == 1) {
        int mid = WILL_UP_TIME / 2;
        double cnt = 1 / double(WILL_UP_TIME) / (double(WILL_UP_TIME) - 1);
        weigh[mid] = 1 / double(WILL_UP_TIME);
        for (int i = 1; i < (WILL_UP_TIME / 2 + 1); ++i) {
            weigh[mid - i] = 1 / double(WILL_UP_TIME) - cnt * i;
            weigh[mid + i] = 1 / double(WILL_UP_TIME) + cnt * i;
        }
    }
    else {
        int midbe = WILL_UP_TIME / 2 - 1;
        int midaf = WILL_UP_TIME / 2;
        double cnt = 1 / double(WILL_UP_TIME) / double(WILL_UP_TIME);
        for (int i = 0; i < (WILL_UP_TIME / 2); ++i) {
            weigh[midbe - i] = 1 / double(WILL_UP_TIME) - cnt * (i + 1);
            weigh[midaf + i] = 1 / double(WILL_UP_TIME) + cnt * (i + 1);
        }
    }
    double res = 0;
    if (ct == TYPE_DIST) {
        res = 0;
        vector<vector<double>> rec;
        for (int j = WILL_UP_TIME; j > 0; --j) {
            vector<double> tmp;
            rec.push_back(tmp);
        }
        for (auto &i : this->distArray) {
            double diff = op_sim_time() - i.first;
            for (int k = 0; k < WILL_UP_TIME; ++k)
                if (diff >= k && diff < k+1) {
                    rec[k].push_back(i.second);
                    break;
                }
        }
        for (int i = 0; i < rec.size(); ++i) {
            if (rec[i].empty())
                continue;
            double inres = 0;
            for (auto &j : rec[i])
                inres += j * weigh[i];
            res += inres / double(rec[i].size());
        }
        this->acs->updateTMWill(res, ct);
    }
    else if (ct == TYPE_BER) {
        res = 0;
        vector<vector<double>> rec;
        for (int j = WILL_UP_TIME; j > 0; --j) {
            vector<double> tmp;
            rec.push_back(tmp);
        }
        for (auto &i : this->berArray) {
            double diff = op_sim_time() - i.first;
            for (int k = 0; k < WILL_UP_TIME; ++k)
                if (diff >= k && diff < k+1) {
                    rec[k].push_back(i.second);
                    break;
                }
        }
        for (int i = 0; i < rec.size(); ++i) {
            if (rec[i].empty())
                continue;
            double inres = 0;
            for (auto &j : rec[i])
                inres += j * weigh[i];
            res += inres / double(rec[i].size());
        }
        this->acs->updateTMWill(res, ct);
    }
    else if (ct == TYPE_DELAY) {
        res = 0;
        vector<vector<double>> rec;
        for (int j = WILL_UP_TIME; j > 0; --j) {
            vector<double> tmp;
            rec.push_back(tmp);
        }
        for (auto &i : this->delayArray) {
            double diff = op_sim_time() - i.first;
            for (int k = 0; k < WILL_UP_TIME; ++k)
                if (diff >= k && diff < k+1) {
                    rec[k].push_back(i.second);
                    break;
                }
        }
        for (int i = 0; i < rec.size(); ++i) {
            if (rec[i].empty())
                continue;
            double inres = 0;
            for (auto &j : rec[i])
                inres += j * weigh[i];
            res += inres / double(rec[i].size());
        }
        this->acs->updateTMWill(res, ct);
    }
    else if (ct == TYPE_SNR) {
        res = 0;
        vector<vector<double>> rec;
        for (int j = WILL_UP_TIME; j > 0; --j) {
            vector<double> tmp;
            rec.push_back(tmp);
        }
        for (auto &i : this->snrArray) {
            double diff = op_sim_time() - i.first;
            for (int k = 0; k < WILL_UP_TIME; ++k)
                if (diff >= k && diff < k+1) {
                    rec[k].push_back(i.second);
                    break;
                }
        }
        for (int i = 0; i < rec.size(); ++i) {
            if (rec[i].empty())
                continue;
            double inres = 0;
            for (auto &j : rec[i])
                inres += j * weigh[i];
            res += inres / double(rec[i].size());
        }
        this->acs->updateTMWill(res, ct);
    }
    // cout << this->distArray.size() << " " << this->delayArray.size() << " " << this->berArray.size() << " " << this->snrArray.size() << endl;
    this->distArray.clear();
    this->delayArray.clear();
    this->berArray.clear();
    this->snrArray.clear();
}

void opnet_ctrller::on_stream(int id) {
    Packet *p = op_pk_get(id);
    int len = op_pk_total_size_get(p) / 8;
    void *buffer;
    op_pk_fd_get_ptr(p, 0, &buffer);
    UNINT* format = reinterpret_cast<UNINT*>(buffer);
    OLSR_packet np;
    int mark = 0;
    np.packOri = format[mark++];
    np.packetSequenceNumber = format[mark] & 0xffff;
    np.packetLenth = (format[mark++] & 0xffff0000) >> 16;
    // cout << op_node_id() << " recv " << len << " packet len: " << np.packetLenth << endl;
    // cout << np.packetLenth << endl;
    for (int i = np.packetLenth; i > 0; --i) {
        // cout << "step 1" << endl;
        UNINT messageType = (format[mark] & 0xff000000) >> 24;
        UNINT vTime = (format[mark] & 0x00ff0000) >> 16;
        UNINT messageSize = (format[mark++] & 0xffff);
        UNINT originatorAddress = format[mark++];
        UNINT TTL = (format[mark] & 0xff000000) >> 24;
        UNINT hopCount = (format[mark] & 0x00ff0000) >> 16;
        UNINT messageSequenceNumber = (format[mark++] & 0xffff);
        message_packet mp(messageType, vTime, originatorAddress, TTL, hopCount, messageSequenceNumber);
        mp.messageSize = messageSize;
        // cout << mp.messageType << " " << mp.vTime << " " << mp.messageSize << " ";
        // cout << mp.originatorAddress << " ";
        // cout << mp.TTL << " " << mp.hopCount << " " << mp.messageSequenceNumber << endl;
        // cout << "step 2" << endl;
        if (mp.messageType == HELLO) {
            // cout << "mark: " << mark << endl;
            mp.helloMessage.willingness = format[mark++];
            mp.helloMessage.linkNum = (format[mark] & 0xffff0000) >> 16;
            mp.helloMessage.neighNum = format[mark++] & 0xffff;
            // cout << mp.helloMessage.willingness << " " << mp.helloMessage.linkNum << " " << mp.helloMessage.neighNum << endl;
            for (int j = mp.helloMessage.linkNum; j > 0; --j) {
                link_status ltmp;
                ltmp.linkcode = (format[mark] & 0xffff0000) >> 16;
                ltmp.num = (format[mark++] & 0xffff);
                for (int k = ltmp.num; k > 0; --k)
                    ltmp.neighborAddress.push_back(format[mark++]);
                mp.helloMessage.links.push_back(ltmp);
            }
            for (int j = mp.helloMessage.neighNum; j > 0; --j) {
                neigh_status ntmp;
                ntmp.neighcode = (format[mark] & 0xffff0000) >> 16;
                ntmp.num = (format[mark++] & 0xffff);
                for (int k = ntmp.num; k > 0; --k)
                    ntmp.neighborAddress.push_back(format[mark++]);
                mp.helloMessage.neighs.push_back(ntmp);
            }
        }
        else if (mp.messageType == TC) {
            mp.tcMessage.MSSN = (format[mark] & 0xffff0000) >> 16;
            mp.tcMessage.num = (format[mark++] & 0xffff);
            for (int j = mp.tcMessage.num; j > 0; --j)
                mp.tcMessage.MPRSelectorAddresses.push_back(format[mark++]);
        }
        else if (mp.messageType == NORMAL) {
            mp.normalMessage.id = format[mark++];
            mp.normalMessage.time = format[mark++];
        }
        // cout << "step last" << endl;
        np.messagePackets.push_back(mp);
    }
    // cout << "recv packets len: " << len << endl;
    if (np.packOri != op_node_id()) {        
        for (auto &i : np.messagePackets) {
            if (i.originatorAddress != op_node_id()) {
                // cout << op_node_id() << " " << i.originatorAddress << " " << i.messageSequenceNumber << endl;
                results item(this->resId, op_sim_time(), "RECV", i.originatorAddress, i.messageSequenceNumber, op_td_get_dbl(p, OPC_TDA_RA_END_DIST), op_td_get_dbl(p, OPC_TDA_RA_END_PROPDEL), op_td_get_dbl(p, OPC_TDA_RA_ACTUAL_BER), op_td_get_dbl(p, OPC_TDA_RA_SNR), i.messageType);
                this->resId++;
                this->res.push_back(item);
                this->packetCount++;
                this->acs->recvPackets(np);
            }
        }
        this->delay.push_back(make_pair(op_sim_time(), op_td_get_dbl(p, OPC_TDA_RA_END_PROPDEL)));
        this->distArray.push_back(make_pair(op_sim_time(), op_td_get_dbl(p, OPC_TDA_RA_END_DIST)));
        this->delayArray.push_back(make_pair(op_sim_time(), op_td_get_dbl(p, OPC_TDA_RA_END_PROPDEL)));
        this->berArray.push_back(make_pair(op_sim_time(), op_td_get_dbl(p, OPC_TDA_RA_ACTUAL_BER)));
        this->snrArray.push_back(make_pair(op_sim_time(), op_td_get_dbl(p, OPC_TDA_RA_SNR)));
    }
    op_pk_destroy(p);
    // cout << "recv end" << endl;
}

void opnet_ctrller::on_stat(int id) {

}

void opnet_ctrller::on_sim_stop() {
    // cout << "node " << op_node_id() << " stop at time : " << op_sim_time() << endl;
    // cout << "node " << op_node_id() << " received packets: " << this->packetCount << endl;
    cout << "node " << op_node_id() << " forwarded packets: " << this->acs->getForwardCount() << endl;
    cout << "node " << op_node_id() << " forwarded times: " << this->acs->getForwardCountSave() << endl;
    // this->printRess();
    // if (op_node_id() == 7)
    //     this->printRess();
    // fstream f;
    // string filename = to_string(op_node_id()) + "delay.txt";
    // f.open(filename, ios::out);
    // for (auto &i : this->delay) {
    //     f << i.first << " " << i.second << endl;
    // }
    // f.close();
}

void opnet_ctrller::printRess() {
    cout << "id   |time      |send/recv |type      |ori       |seq       |dist      |delay     |BER       |SNR      " << endl;
    cout << "-----|----------|----------|----------|----------|----------|----------|----------|----------|---------" << endl;
    for (auto &i : this->res)
        i.printRes();
}