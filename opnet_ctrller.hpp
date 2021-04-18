#pragma once
#include <Adnode_ctrl.hpp>

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
    class opnet_ctrller
    {
    private:
        bool rx_blocked;
        int packetCount;

    public:
        opnet_ctrller();
        void on_sim_start();
        void on_stream(int);
        void on_stat(int);
        void on_sim_stop();
        void on_self();
        void send();
        void schedule_self();
        ~opnet_ctrller();
    };  
}