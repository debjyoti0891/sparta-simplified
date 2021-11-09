// <Pipe.hpp> -*- C++ -*-

#pragma once

#include <cinttypes>
#include <string>
#include <memory>

#include "sparta/simulation/Unit.hpp"
#include "sparta/ports/PortSet.hpp"
#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"
#include "sparta/events/UniqueEvent.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#define ILOG(msg) \
    if(SPARTA_EXPECT_FALSE(pipe_info_)) { \
        pipe_info_ << msg; \
    }
 

// Possible to create this class outside of the Pipe class, but
// simply not that clean.  It's better to put it in the Pipe class
// (for namespacing), but definitely not required.


//
// The Pipe class
//
class Pipe : public sparta::Unit
{
public:

    

    // Name of this resource. Required by sparta::ResourceFactory
    static const char * name;


    class PipeParameterSet : public sparta::ParameterSet
    {
    public:
        PipeParameterSet(sparta::TreeNode* n) :
            sparta::ParameterSet(n)
        {
            // // See test_arch_with_override.sh for explanation about this
            // // parameter. It is being used for a test as part of make regress.
            // arch_override_test_param = "reset_in_constructor";
            // auto non_zero_validator = [](uint32_t & val, const sparta::TreeNode*)->bool {
            //     if(val > 0) {
            //         return true;
            //     }
            //     return false;
            // };
            // pipe_delay.addDependentValidationCallback(non_zero_validator,
            //                                                 "Num to send must be greater than 0");
        }

        // PARAMETER(uint32_t, pipe_delay, 10, "Delay between receive and send")
        // PARAMETER(std::string, arch_override_test_param, "arch_override_default_value", "Set this to true in ParameterSet construction")
    };
    Pipe (sparta::TreeNode * name, const PipeParameterSet * p);
    

private:
    uint32_t data;
    bool valid;
    // Pipe's ports
    sparta::DataOutPort<uint32_t> pipe_out_port_{&unit_port_set_, "pipe_out_port"};
    sparta::DataInPort<uint32_t>  pipe_in_port_ {&unit_port_set_, "pipe_in_port", sparta::SchedulingPhase::PortUpdate, 1};
    sparta::SignalInPort          pipe_go_port_ {&unit_port_set_, "pipe_go_port"};
    sparta::SignalOutPort         pipe_go_producer_port_ {&unit_port_set_, "pipe_go_producer_port"};

    // An event to be scheduled in the sparta::SchedulingPhase::Tick
    // phase if data is received
    sparta::Event<> event_do_some_work_{&unit_event_set_, "do_work_event",
                                        CREATE_SPARTA_HANDLER(Pipe, sendData_),5};

    // Pipe receive handler
    // receive the data from the producer 
    void receiveData_(const uint32_t & dat);

    // Pipe send data to the consumer 
    void sendData_();

    // receive signal from the consumer and send a singal to the producer
    void receiveSignal_();


    uint32_t current_ints_count_ = 0;

    // Stats
    sparta::Counter num_processed_{&unit_stat_set_, "num_processed_",
                                  "Number of items produced", sparta::Counter::COUNT_NORMAL};

    // Loggers
    sparta::log::MessageSource pipe_info_;
};

