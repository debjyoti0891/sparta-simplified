// <Pipe.cpp> -*- C++ -*-

#include "Pipe.hpp"
#include "MessageCategories.hpp"

const char * Pipe::name = "pipe";

Pipe::Pipe (sparta::TreeNode * node, const PipeParameterSet * p) :
    sparta::Unit(node, name),
    pipe_info_(node, message_categories::INFO, "Pipe Info Messages")
{
    
    (void)p;

    // Register a handler when the producer sends a data
    pipe_in_port_.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(Pipe, receiveData_, uint32_t));

    // Register a go-handler when the consumer sends a go request
    pipe_go_port_.registerConsumerHandler(CREATE_SPARTA_HANDLER(Pipe, receiveSignal_));

    data = 0;
    valid = false; 


}

void Pipe::receiveData_(const uint32_t & dat)
{
   ILOG("Pipe received data: " << dat << "at tick" << getClock()->currentCycle());
   data = dat;
   valid = true;
   event_do_some_work_.schedule();
}

void Pipe::sendData_(){
    if(valid){
        ILOG("Pipe sending data: " << data << "at tick" << getClock()->currentCycle());
        valid = false;
        pipe_out_port_.send(data);
    }
}

void Pipe::receiveSignal_(){
    ILOG("Pipe received go from consumer  at tick" << getClock()->currentCycle());
    pipe_go_producer_port_.send();
    ILOG("Pipe sent go to producer at tick" << getClock()->currentCycle());
    ++num_processed_;
}
