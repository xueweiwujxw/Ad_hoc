#pragma once
#include <Adnode_ctrl.hpp>
#include <future>

namespace opnet {
    // class opnet_ctrller
    // {
    // private:
    //     int packetCount;
    // public:
    //     opnet_ctrller();
    //     ~opnet_ctrller();
    //     void on_sim_start();
    //     void on_strm(int);
    //     void on_stat(int);
    //     void on_sim_stop();
    // };  
    struct nodePackets
    {
        int number;
        int origin;
    };
    class opnet_ctrller
    {
    private:
        bool rx_blocked;
        int packetCount;
        int packetSequence;
        int isrunning;
        future<int> m_future;

    public:
        opnet_ctrller();
        void on_sim_start();
        void on_stream(int);
        void on_stat(int);
        void on_sim_stop();
        void on_self();
        void send(void *data, unsigned int len);
        void schedule_self();
        int run();
        ~opnet_ctrller();
    };  
}