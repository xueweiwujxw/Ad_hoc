#pragma once
#include <Adnode_ctrl_simple.hpp>
#include <future>
#include <string>
#include <vector>
#include <protocol_simple.hpp>

using namespace std;

namespace opnet {
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
        double snr;
        int id;
        UNINT type;
    public:
        results(int id, double time, char* behavior, int origin, int number, double dist, double delay, double BER, double snr, UNINT type) {
            this->id = id;
            this->time = time;
            this->behavior = behavior;
            this->origin = origin;
            this->number = number;
            this->dist = dist;
            this->delay = delay;
            this->BER = BER;
            this->snr = snr;
            this->type = type;
        }
        ~results() {}
        void printRes() {
            char str[7];
            if (this->type == HELLO) 
                strcpy(str, "HELLO");
            else if (this->type == TC)
                strcpy(str, "TC");
            else if (this->type == NORMAL)
                strcpy(str, "NORMAL");
            if (strcmp(behavior, "SEND") == 0)
                printf("%-4d |%-9.6f |%-9s |%-9s |%-9d |%-9d |%-9s |%-9s |%-9s |%-9s\n", id, time, behavior, str, origin, number, "NAN", "NAN", "NAN", "NAN");
            else if (strcmp(behavior, "RECV") == 0)
                printf("%-4d |%-9.6f |%-9s |%-9s |%-9d |%-9d |%-9.3f |%-9.6f |%-9.6f |%-9.6f\n", id, time, behavior, str, origin, number, dist, delay, BER, snr);
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
        int packetCount;
        int packetSequence;
        vector<results> res;
        int resId;
        double SNR;
        double DIST;
        double DELAY;
        double BER;
        double tmpCount;
        Adnode_ctrl_simple* acs;
        vector<pair<double, double>> distArray;
        vector<pair<double, double>> delayArray;
        vector<pair<double, double>> berArray;
        vector<pair<double, double>> snrArray;

    public:
        opnet_ctrller();
        void on_sim_start();
        void on_stream(int);
        void on_stat(int);
        void on_sim_stop();
        void on_self();
        void send(OLSR_packet data, UNINT len, UNINT type);
        void schedule_self(double, UNINT);
        void cacluate(calType ct);
        ~opnet_ctrller();
        void printRess();
    };  
}