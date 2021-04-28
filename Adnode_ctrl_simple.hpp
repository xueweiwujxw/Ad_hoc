#pragma once
#include <opnet.h>
#include <protocol_simple.hpp>
#include <vector>
#include <table_manager.hpp>

using namespace std;

namespace opnet
{
    class Adnode_ctrl_simple
    {
    private:
        UNINT nodeId;
        table_manager* tm;        
    public:
        Adnode_ctrl_simple() {
            tm = new table_manager(this->nodeId);
        }
        ~Adnode_ctrl_simple() {
            delete tm;
        }
    };
} // namespace opnet
