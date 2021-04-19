#pragma once

#include <set>

using namespace std;

namespace opnet
{
    int HELLO_INTERVAL = 2;
    int REFERSH_INTERVAL = 2;
    int TC_INTERVAL = 5;
    int MID_INTERVAL = TC_INTERVAL;
    int HNA_INTERVAL = TC_INTERVAL;
    int NEIGHB_HOLD_TIME = 3 * REFERSH_INTERVAL;
    int TOP_HOLD_TIME = 3 * TC_INTERVAL;
    int DUP_HOLD_TIME = 30;
    int MID_HOLD_TME = 3 * MID_INTERVAL;
    int HNA_HOLD_TIME = 3 * HNA_INTERVAL;


    /*协议分组基本格式*/

    //Link code
    enum Linkcode
    {
        UNSPEC_LINK,
        ASYM_LINK,      //链路非对称
        SYM_LINK,       //链路对称
        MPR_LINK,       //表中节点被HELLO message的发送节点选为MPR
        LOST_LINK      //链路中断
    };

    enum Neighborcode
    {
        SYM_NEIGH,
        MPR_NEIGH,
        NOT_NEIGH
    };

    enum NeighType
    {
        NOT_SYM,
        SYM 
    };

    //Willingness
    enum Willingness
    {
        WILL_NEVER,
        WILL_LOW,
        WILL_DEFAULT,
        WILL_HIGH,
        WILL_ALWAYS
    };

    //message type
    enum Messagetype
    {
        HELLO,
        TC
    };

    //link status
    struct link_status
    {
        unsigned int linkcode : 4;                  //链路类型
        unsigned int neighcode: 4;
        unsigned int reserved : 8;                  //保留字段：0000_0000
        unsigned int linkMessageSize: 16;           //本链路消息的大小，从链路类型字段开始直到下一个链路类型字段之前(若无，则到分组结尾)
        unsigned int neighborAddress;               //邻居地址列表，HELLO的发送节点到邻居列表的所有链路均为前面的类型
        link_status(Linkcode lc) {
            this->reserved = 0;
            this->linkcode = lc;
        }
        link_status() {}
    };
    

    //hello message，执行链路检测、邻居发现的功能
    struct message_hello
    {
        unsigned int reserved : 16;                 //保留字段：0000_0000_0000_0000
        unsigned int hTime : 8;                     //HELLO发送时间间隔
        unsigned int willingness : 8;               //一个节点为其他节点携带网络流量的意愿
        set<link_status> linkMessage;
        message_hello() {
            this->reserved = 0;
        }
    };

    //TC message，执行MPR信息声明功能
    struct message_tc
    {
        unsigned int MSSN : 16;                     //MPR Selector序列号
        unsigned int reserved : 16;                 //保留字段0000_0000_0000_0000
        set<unsigned int> MPRSelectorAddresses;  //多点中继选择的地址
        message_tc() {
            this->reserved = 0;
        }
    };    

    //message structure
    struct message_packet
    {
        unsigned int messageType : 8;               //MESSAGE域中将要被发现的类型
        unsigned int vTime : 8;                     //分组携带消息的有效期
        unsigned messageSize : 16;                  //消息的长度，从消息类型的开始处计算直到下一个消息类型的开始处(若无，则到消息分组结束)
        unsigned int originatorAddress;             //产生该消息的主地址，重传中也不会变化，和IP地址不同
        unsigned int TTL : 8;                       //消息被传送的最大跳数，在消息被重传之前，TTL减一，当一个节点收到一个消息，其TTL为0或1时，这个消息在任何情况下都不应该被重传
        unsigned int hopCount : 8;                  //跳数，在一个消息被重传前，跳数加一
        unsigned int messageSequenceNumber : 16;    //消息的序列号
        message_hello helloMessage;                 //HELLO_MESSAGE域
        message_tc tcMessage;                       //TC_MESSAGE域
    };

    //packet structure
    struct olsr_packet
    {
        unsigned int packetLength : 16;             //分组长度，以bytes计
        unsigned int packetSequenceNumber : 16;     //分组序列号，每当一个新的OLSR分组传送时，分组序列号必须增加1(个人认为是每个节点自己保存独立的序列号)
        set<message_packet> messagePackets;      //消息
    };

    /*节点保存的信息表*/

    //本地链路信息表
    struct local_link
    {
        unsigned int L_local_iface_addr;            //本地节点的接口地址
        unsigned int L_neighbor_iface_addr;         //邻节点的接口地址
        unsigned int L_SYM_time;                    //直到此时刻前。链路被认为是对称的
        unsigned int L_ASYM_time;                   //直到此时刻前，链路被认为是单向的
        unsigned int L_time;                        //链路维护时刻。链路在此时刻失效，必须被删除
    };  

    //邻居表
    struct one_hop_neighbor
    {
        unsigned int N_neighbor_main_addr;          //节点i的一跳邻居地址
        unsigned int N_status;                      //节点i与其一跳邻居之间的链路状态
        unsigned int N_willingness;                 //表示邻居节点为其他节点转发分组的意愿程度
    };
    
    //两跳邻居表
    struct two_hop_neighbor
    {
        unsigned int N_neighbor_main_addr;          //表示邻节点的地址
        unsigned int N_2hop_addr;                   //表示与N_neighbor_main_addr有对称链路的两跳节点的地址
        unsigned int N_time;                        //表示表项到期必须被移除的时间
    };
    
    //MPR Selector表
    struct MPR
    {
        unsigned int MS_main_addr;                  //MPR Selector节点的地址
        unsigned int MS_time;                       //该MPR Selector集表项的保持时间，当MPR Select过期时要及时删除
    };
    
    //TC分组重复记录表
    struct duplicate_set
    {
        unsigned int D_addr;                        //最初发送该分组的节点的地址
        unsigned int D_seq_num;                     //分组的序列号，用于区分新旧分组
        bool D_retransmitted;                       //为一个布尔值，用来表示此消息是否被重传过
        bool D_received;                            //已经接收
        unsigned int D_time;                        //该表项的保持时间，表项到期时必须被删除
    };
    
    //拓扑表
    struct topology
    {
        unsigned int T_dest_addr;                   //MPR选择节点的地址，表示该节点已经选择节点T_last作为其MPR
        unsigned int T_last_addr;                   //被T_last选为MPR的节点的地址
        unsigned int T_seq;                         //表示T_last已经发布了它保存的序列号为T_seq的MPR Selector集合的控制信息
        unsigned int T_time;                        //表项的保持时间，到期后就失效，必须被删除
    };
    
    //路由表
    struct route
    {
        unsigned int R_dest_addr;                   //路由目的节点地址
        unsigned int R_next_addr;                   //路由的下一跳地址
        unsigned int R_dist;                        //本节点到目的节点的距离
        unsigned int R_iface_addr;                  //表示下一跳节点通过本地接口R_iface_addr到达
    };
} // namespace opnet
