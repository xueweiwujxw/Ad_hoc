#pragma once

#include <vector>
#include <iostream>
using namespace std;

namespace opnet
{
#define HELLO_INTERVAL 2
#define TC_INTERVAL 5
#define NEIGHB_HOLD_TIME 6
#define TOP_HOLD_TIME 15
#define DUP_HOLD_TIME 30
#define WILL_UP_TIME max(HELLO_INTERVAL, TC_INTERVAL)

#define UNINT unsigned int

    enum ScheduleCode
    {
        OPC_HELLO_SEND,
        OPC_TC_SEND,
        OPC_QUA_CAL
    };

    enum calType 
    {
        TYPE_DIST,
        TYPE_BER,
        TYPE_DELAY,
        TYPE_SNR,
    };

    enum LinkCode
    {
        ASYM_LINK,
        SYM_LINK,
        LOST_LINK
    };

    enum NeighCode
    {
        NOT_NEIGH,
        SYM_NEIGH,
        MPR_NEIGH
    };

    enum NeighbStatus
    {
        NOT_SYM,
        SYM,
    };

    enum Willingness
    {
        WILL_NEVER,
        WILL_LOW,
        WILL_DEFAULT,
        WILL_HIGH,
        WILL_ALWAYS
    };

    enum LinkQuality
    {
        FATAL,
        LOW,
        DEFAULT,
        HIGH,
        PERFECT
    };

    enum messageType
    {
        HELLO,
        TC,
        NORMAL
    };

    struct link_status
    {
        UNINT linkcode: 16;
        UNINT num: 16;
        vector<UNINT> neighborAddress;
        link_status(LinkCode lc) {
            this->linkcode = lc;
        }
        link_status() {}
        UNINT getSize() {
            this->num = this->neighborAddress.size();
            return 1 + this->neighborAddress.size();
        }
    };

    struct neigh_status
    {
        UNINT neighcode: 16;
        UNINT num: 16;
        vector<UNINT> neighborAddress;
        neigh_status(NeighCode nc) {
            this->neighcode = nc;
        }
        neigh_status() {}
        UNINT getSize() {
            this->num = this->neighborAddress.size();
            return 1 + this->neighborAddress.size();
        }
    };
    
    struct message_hello
    {
        UNINT willingness;
        UNINT linkNum: 16;
        UNINT neighNum: 16;
        vector<link_status> links;
        vector<neigh_status> neighs;
        UNINT getSize() {
            UNINT count = 2;
            for (auto &i : this->links)
                count += i.getSize();
            for (auto &i : this->neighs)
                count += i.getSize();
            this->linkNum = this->links.size();
            this->neighNum = this->neighs.size();
            return count;
        }
    };

    struct message_tc
    {
        UNINT MSSN: 16;
        UNINT num: 16;
        vector<UNINT> MPRSelectorAddresses;
        UNINT getSize() {
            this->num = this->MPRSelectorAddresses.size();
            return 1 + this->MPRSelectorAddresses.size();
        }
    };

    struct message_normal
    {
        UNINT id;
        UNINT time;
        UNINT getSize() {
            return 2;
        }
        message_normal(UNINT id, UNINT time) {
            this->id = id;
            this->time = time;
        }
        message_normal() {}
    };
        
    struct message_packet
    {
        UNINT messageType: 8;
        UNINT vTime: 8;
        UNINT messageSize: 16;
        UNINT originatorAddress;
        UNINT TTL: 8;
        UNINT hopCount: 8;
        UNINT messageSequenceNumber: 16;
        message_hello helloMessage;
        message_tc tcMessage;
        message_normal normalMessage;
        int getSize() {
            if (this->messageType == HELLO) {
                this->messageSize = this->helloMessage.getSize() + 3;
            }
            else if (this->messageType == TC) {
                this->messageSize = this->tcMessage.getSize() + 3;
            }
            else if (this->messageType == NORMAL) {
                this->messageSize = this->normalMessage.getSize() + 3;
            }
            return this->messageSize;
        }
        message_packet(UNINT messageType, UNINT vTime, UNINT originatorAddress, UNINT TTL, UNINT hopCount, UNINT messageSequenceNumber) {
            this->messageType = messageType;
            this->vTime = vTime;
            this->originatorAddress = originatorAddress;
            this->TTL = TTL;
            this->hopCount = hopCount;
            this->messageSequenceNumber = messageSequenceNumber;
        }
    };
    
    struct OLSR_packet
    {
        UNINT packOri;
        UNINT packetLenth: 16;
        UNINT packetSequenceNumber: 16;
        vector<message_packet> messagePackets;
        int getSize() {
            UNINT size = 2;
            for (auto &i: this->messagePackets) {
                size += i.getSize();
            }
            this->packetLenth = this->messagePackets.size();
            return size;
        }
    };

    struct local_link
    {
        UNINT L_neighbor_iface_addr;
        UNINT L_SYM_time;
        UNINT L_ASYM_time;
        UNINT L_time;
        UNINT L_quality;
    };

    struct two_hop_neighbor
    {
        UNINT N_2hop_addr;
        UNINT N_time;
    };
    
    struct one_hop_neighbor
    {
        UNINT N_neighbor_addr;
        UNINT N_status;
        UNINT N_willingness;
        vector<two_hop_neighbor> N_2hop;
    };

    struct MPR
    {
        UNINT MS_addr;
        UNINT MS_time;
    };  

    struct duplicat_set
    {
        UNINT D_addr;
        UNINT D_seq_num;
        UNINT D_time;
        bool D_retransmitted;
        bool D_received;
    };

    struct topology_item
    {
        UNINT T_dest_addr;
        UNINT T_last_addr;
        UNINT T_seq;
        UNINT T_time;
    };
    
    struct route_item
    {
        UNINT R_dest_addr;
        UNINT R_next_addr;
        UNINT R_dist;
    }; 
} // namespace opnet
