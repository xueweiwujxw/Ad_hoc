#pragma once

#include <vector>

using namespace std;

namespace opnet
{
    /*协议分组基本格式*/

    //Link code
    enum Linkcode
    {
        ASYM_LINK,      //链路非对称
        SYM_LINK,       //链路对称
        MPR_LINK        //表中节点被HELLO message的发送节点选为MPR
    };

    //hello message
    struct message_hello
    {
        unsigned int reserved0 : 16;                //保留字段：0000_0000_0000_0000
        unsigned int hTime : 8;                     //HELLO发送时间间隔
        unsigned int willingness : 8;               //一个节点为其他节点携带网络流量的意愿
        Linkcode linkcode : 8;                      //链路类型
        unsigned int reserved1 : 8;                 //保留字段：0000_0000
        unsigned int linkMessageSize: 16;           //本链路消息的大小
        vector<unsigned int> neighborAddresses;     //邻居地址列表，HELLO的发送节点到邻居列表的所有链路均为前面的类型
    };

    //TC message
    struct message_tc
    {
        unsigned int MSSN : 16;                     //MPR Selector序列号
        unsigned int reserved : 16;                 //保留字段0000_0000_0000_0000
        vector<unsigned int> MPRSelectorAddresses;  //多点中继选择的地址
    };

    //message structure
    struct message_packet
    {
        unsigned int messageType : 8;               //MESSAGE域中将要被发现的类型
        unsigned int vTime : 8;                     //分组携带消息的有效期
        unsigned messageSize : 16;                  //消息的长度
        unsigned int originatorAddress;             //产生该消息的主地址，重传中也不会变化，和IP地址不同
        unsigned int TTL : 8;                       //消息被传送的最大跳数
        unsigned int hopCount : 8;                  //跳数
        unsigned int messageSequenceNumber : 16;    //消息的序列号
        union message                               //MESSAGE，HELLO和TC均属于MESSAGE
        {
            message_hello mh;
            message_tc mt;
        } message;
    };

    //packet structure
    struct packet
    {
        unsigned int packetLength : 16;             //分组长度
        unsigned int packetSequenceNumber : 16;     //分组序列号
        vector<message_packet> messagePackets;      //消息
    };

    /*节点保存的信息表*/

    //本地链路信息表
    struct local_link
    {
        unsigned int L_local_iface_addr;
        unsigned int L_neighbor_iface_addr;
        unsigned int L_SYM_time;
        unsigned int L_ASYM_time;
        unsigned int L_time;
    };  

    //邻居表
    struct one_hop_neighbor
    {
        unsigned int N_neighbor_main_addr;
        unsigned int N_statusl;
        unsigned int N_willingness;
    };
    
    //两跳邻居表
    struct two_hop_neighbor
    {
        unsigned int N_neighbor_main_addr;
        unsigned int N_2hop_addr;
        unsigned int N_time;
    };
    
    //MPR Selector表
    struct MPR_table
    {
        unsigned int MS_main_addr;
        unsigned int MS_time;
    };
    
    //TC分组重复记录表
    struct TC_repeat_table
    {
        unsigned int D_addr;
        unsigned int D_seq_num;
        unsigned int D_retransmitted;
        unsigned int D_iface_list;
        unsigned int D_time;
    };
    
    //拓扑表
    struct topology_table
    {
        unsigned int T_dest_addr;
        unsigned int T_last_addr;
        unsigned int T_seq;
        unsigned int T_time;
    };
    
    //路由表
    struct route_table
    {
        unsigned int R_dest_addr;
        unsigned int R_next_addr;
        unsigned int R_dist;
        unsigned int R_iface_addr;
    };
    
    
    

} // namespace opnet
