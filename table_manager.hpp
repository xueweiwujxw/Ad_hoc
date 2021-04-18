#pragma once
#include <protocol.h>
#include <vector>

using namespace std;

namespace opnet
{
    class local_link_table
    {
    private:
        vector<local_link> localLinkTable;
    public:
        local_link_table();
        ~local_link_table();
        vector<local_link> getTable();
    };

    class one_hop_neighbor_table
    {
    private:
        vector<one_hop_neighbor> oneHopNeighborTable;
    public:
        one_hop_neighbor_table();
        ~one_hop_neighbor_table();
        vector<one_hop_neighbor> getTable();
    };

    class two_hop_neighbor_table
    {
    private:
        vector<two_hop_neighbor> twoHopNeighborTable;
    public:
        two_hop_neighbor_table();
        ~two_hop_neighbor_table();
        vector<two_hop_neighbor> getTable();
    };

    class mpr_table
    {
    private:
        vector<MPR> mprtable;
    public:
        mpr_table();
        ~mpr_table();
        vector<MPR> getTable();
    };

    class tc_repeat_table
    {
    private:
        vector<TC_repeat> tcRepeatTable;
    public:
        tc_repeat_table();
        ~tc_repeat_table();
        vector<TC_repeat> getTable();
    };

    class topology_table
    {
    private:
        vector<topology> topologyTable;
    public:
        topology_table();
        ~topology_table();
        vector<topology> getTable();
    };

    class route_table
    {
    private:
        vector<route> routeTable;
    public:
        route_table();
        ~route_table();
        vector<route> getTable();
    };
} // namespace opnet
