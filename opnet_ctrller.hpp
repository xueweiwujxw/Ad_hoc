#pragma once
#include <Adnode_ctrl.hpp>
#include <future>
#include <string>
#include <vector>

using namespace std;

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
    class results
    {
    private:
        double time;
        char* behavior;
        int origin;
        int number;
        double dist;
        double delay;
        double BER;
        int id;
    public:
        results(int id, double time, char* behavior, int origin, int number, double dist, double delay, double BER) {
            this->id = id;
            this->time = time;
            this->behavior = behavior;
            this->origin = origin;
            this->number = number;
            this->dist = dist;
            this->delay = delay;
            this->BER = BER;
        }
        ~results() {}
        void printRes() {
            if (strcmp(behavior, "SEND") == 0)
                printf("%-4d |%-9.6f |%-8s |%-9d |%-9d |%-9s |%-9s |%-9s\n", id, time, behavior, origin, number, "NAN", "NAN", "NAN");
            else if (strcmp(behavior, "RECV") == 0)
                printf("%-4d |%-9.6f |%-8s |%-9d |%-9d |%-9.3f |%-9.6f |%-9.6f\n",id, time, behavior, origin, number, dist, delay, BER);
        }

    };
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
        vector<results> res;
        int resId;

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
        void printRess();
    };  
}