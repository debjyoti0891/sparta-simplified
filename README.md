# A Fast and Furious Introduction to Sparta

## Usecase

Producer sends data to a Pipe. Pipe receives data and forwards this to a consumer. Consumer signals the Pipe that
data is received. In turn, Pipe signals the producer that data has been received and the cycle continues. 
We extend the basic [example](https://sparcians.github.io/map/skeleton_example.html) to introduce a new ```sparta::Unit``` Pipe. 

## Pipe definition (Pipe.hpp)

Each sparta unit should have a ParameterSet (subclass of ```sparta::ParameterSet``` ). With each parameter, there can be a validation method attached,
to check if valid values were set to the parameter and a name of the unit as well.

For the Pipe, we consider 4 ports --- two data ports and two signal ports. 
```
    sparta::DataOutPort<uint32_t> pipe_out_port_{&unit_port_set_, "pipe_out_port"};
    sparta::DataInPort<uint32_t>  pipe_in_port_ {&unit_port_set_, "pipe_in_port", sparta::SchedulingPhase::PortUpdate, 1};
    sparta::SignalInPort          pipe_go_port_ {&unit_port_set_, "pipe_go_port"};
    sparta::SignalOutPort         pipe_go_producer_port_ {&unit_port_set_, "pipe_go_producer_port"};
```

The DataInPort receives the data from the producer and sends it to the consumer
using the DataOutPort. Similarly, it receives acknowledgement of the receipt by the consumer using the SignalInPort,
which it passes along to the producer using the SignalOutPort.


Corresponding to each input port, we can define a handler to perform some action on the received data.
```

    // Pipe receive handler
    // receive the data from the producer 
    void receiveData_(const uint32_t & dat);

    // receive signal from the consumer and send a singal to the producer
    void receiveSignal_();

```

We can also define events, that will be triggered after a specified duration on scheduling. For example,
we define a event to delay signalling the producer,  after a signal is received from the consumer. In this case,
delay is 5 cycles and ```sendData_``` is called as its handler.
```
    // An event to be scheduled in the sparta::SchedulingPhase::Tick
    // phase if data is received
    sparta::Event<> event_do_some_work_{&unit_event_set_, "do_work_event",
                                        CREATE_SPARTA_HANDLER(Pipe, sendData_),5};
```

Statistics can be measured quite easily.
```
// Stats
    sparta::Counter num_processed_{&unit_stat_set_, "num_processed_",
                                  "Number of items produced", sparta::Counter::COUNT_NORMAL};
```
 To use such a counter, a simple ```++num_processed_``` would suffice.  

Finally, loggers can be set up per Unit. 
```
sparta::log::MessageSource pipe_info_;
```

## Pipe implementation (Pipe.cpp)
+ Start by defining the name of the Unit 
+ The handler methods are tied to the ports in the constructor. 
    ```
    // Register a handler when the producer sends a data
    pipe_in_port_.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(Pipe, receiveData_, uint32_t));

    // Register a go-handler when the consumer sends a go request
    pipe_go_port_.registerConsumerHandler(CREATE_SPARTA_HANDLER(Pipe, receiveSignal_));
    ```
    These methods are called when the data is received at the port.
+ We schedule the event inside ```receiveData_```. 
    ```event_do_some_work_.schedule();```
    This would trigger ```sendData_``` after 5 cycles. 


## Building the simulator tree (StageSim{.hpp,.cpp})

+ ```StageSim.hpp``` is the top level module definition.
+ In ```StageSim.cpp```, a bunch of setup needs to be done, in order to build the simulator tree.
    + In the constructor, the units and the corresponding parameter sets are made available.
    ```getResourceSet()->addResourceFactory<sparta::ResourceFactory<Pipe, Pipe::PipeParameterSet>>();```
    + The destructor is relatively simple. 
    + ```buildTree_``` is one of the important things. Basically, each node is created as a ```ResourceTreeNode```
        and added to the ```to_delete_``` vector. Reading the parameters is performed and the constructor is called using these parameters. 
    + ```configureTree_``` is not used in the current context. 
    + ```bindTree_``` is used to connect the different ports! An example is 
        ```// bind pipe to the producer
        sparta::bind(root_tree_node->getChildAs<sparta::Port>(nodeName.str() + ".ports.producer_out_port"),
                   root_tree_node->getChildAs<sparta::Port>("pipe.ports.pipe_in_port"));
        ```
        It can be observed that the port names are hierarchical. ```pipe.ports.pipe_in_port``` : ```pipe``` is the unit, under which
        ```ports``` is the set of ports and ```pipe_in_port``` is the name of the port. 

## Command line simulator setup (main_3s.cpp)
+ Not much changes were made to the original example, except the following. 
    ```
     // Create the simulator object for population -- does not
        // instantiate nor run it.
        sparta::Scheduler scheduler;
        StageSim sim(scheduler, be_noisy);
    ````
+ Now, we are ready to run the simulation
    ```
    cls.populateSimulation(&sim);
    cls.runSimulator(&sim);
    cls.postProcess(&sim);
    ```


## Running the example 

In the current environment, the following modules were available.
Currently Loaded Modulefiles:
```
 1) GCCcore/10.3.0                   5) numactl/2.0.14-GCCcore-10.3.0      9) hwloc/2.4.1-GCCcore-10.3.0      13) libfabric/1.12.1-GCCcore-10.3.0  17) FlexiBLAS/3.0.4-GCC-10.3.0      21) foss/2021a                      25) Tcl/8.6.11-GCCcore-10.3.0     29) RapidJSON/1.1.0-GCCcore-10.3.0  33) cmake-3.17.5  
 2) zlib/1.2.11-GCCcore-10.3.0       6) XZ/5.2.5-GCCcore-10.3.0           10) OpenSSL/1.1                     14) PMIx/3.2.3-GCCcore-10.3.0        18) gompi/2021a                     22) yaml-cpp-0.6.2                  26) SQLite/3.35.4-GCCcore-10.3.0  30) bzip2/1.0.8-GCCcore-10.3.0      
 3) binutils/2.36.1-GCCcore-10.3.0   7) libxml2/2.9.10-GCCcore-10.3.0     11) libevent/2.1.12-GCCcore-10.3.0  15) OpenMPI/4.1.1-GCC-10.3.0         19) FFTW/3.3.9-gompi-2021a          23) ncurses/6.2-GCCcore-10.3.0      27) Szip/2.1.1-GCCcore-10.3.0     31) GMP/6.2.1-GCCcore-10.3.0        
 4) GCC/10.3.0                       8) libpciaccess/0.16-GCCcore-10.3.0  12) UCX/1.10.0-GCCcore-10.3.0       16) OpenBLAS/0.3.15-GCC-10.3.0       20) ScaLAPACK/2.1.0-gompi-2021a-fb  24) libreadline/8.1-GCCcore-10.3.0  28) HDF5/1.10.7-gompi-2021a       32) boost-1.77.0   ```
```

To build the simulator,
 ```
 mkdir build
 cd build
 cmake ..
 make sparta_3stage
 ```

Sparta has a ton of options, which can be viewed as follows:
``` sparta_3stage --help```

+ To see the **simulation tree**,```sparta_3stage --show-tree```

    This is a relatively simple way to see which ports are connected to what.
    ```
    top : <top (root)> (privacy: 0)
    +-descendant_attached : <top.descendant_attached name:"descendant_attached" datat:(sparta::TreeNode)  observers:1 posted:55> (privacy: 0)
    +-consumer : <top.consumer resource: "consumer"> (privacy: 0)
    | +-params : <top.consumer.params 1 params> {builtin} (privacy: 0)
    | | +-num_producers : [<top.consumer.params.num_producers tags:[SPARTA_Parameter]>]<param uint32_t num_producers=1, def=1, write=0 read: 1 ignored: 0> (privacy: 0)
    | +-ports : <top.consumer.ports> (privacy: 0)
    | | +-consumer_in_port : [bound to] {pipe_out_port (top.pipe.ports.pipe_out_port)} (privacy: 0)
    | | | +-events : <top.consumer.ports.consumer_in_port.events 1 events> {builtin} (privacy: 0)
    | | | | +-consumer_in_port_forward_event : <top.consumer.ports.consumer_in_port.events.consumer_in_port_forward_event> (privacy: 0)
    | | +-producer0_go_port : [bound to] {pipe_go_port (top.pipe.ports.pipe_go_port)} (privacy: 0)
    | +-events : <top.consumer.events 1 events> {builtin} (privacy: 0)
    | | +-ev_data_arrived : <top.consumer.events.ev_data_arrived> (privacy: 0)
    | +-stats : <top.consumer.stats 0 stats, 1 counters> {builtin} (privacy: 0)
    | | +-num_consumed : <top.consumer.stats.num_consumed val:100 normal vis:100000000> (privacy: 0)
    | +-? : <top.consumer:log_msg_src cat:"info" observed:false msgs:0> (_sparta_log_msg_source_[0])  (privacy: 0)
    | +-? : <top.consumer:log_msg_src cat:"warning" observed:true msgs:0> (_sparta_log_msg_source_[1])  (privacy: 0)
    | +-? : <top.consumer:log_msg_src cat:"debug" observed:false msgs:0> (_sparta_log_msg_source_[2])  (privacy: 0)
    | +-? : <top.consumer:log_msg_src cat:"info" observed:false msgs:0> (_sparta_log_msg_source_[3])  (privacy: 0)
    +-pipe : <top.pipe resource: "pipe"> (privacy: 0)
    | +-params : <top.pipe.params 0 params> {builtin} (privacy: 0)
    | +-ports : <top.pipe.ports> (privacy: 0)
    | | +-pipe_out_port : [bound to] {consumer_in_port (top.consumer.ports.consumer_in_port)} (privacy: 0)
    | | +-pipe_in_port : [bound to] {producer_out_port (top.producer0.ports.producer_out_port)} (privacy: 0)
    | | | +-events : <top.pipe.ports.pipe_in_port.events 1 events> {builtin} (privacy: 0)
    | | | | +-pipe_in_port_forward_event : <top.pipe.ports.pipe_in_port.events.pipe_in_port_forward_event> (privacy: 0)
    | | +-pipe_go_port : [bound to] {producer0_go_port (top.consumer.ports.producer0_go_port)} (privacy: 0)
    | | | +-events : <top.pipe.ports.pipe_go_port.events 1 events> {builtin} (privacy: 0)
    | | | | +-pipe_go_port_forward_event : <top.pipe.ports.pipe_go_port.events.pipe_go_port_forward_event> (privacy: 0)
    | | +-pipe_go_producer_port : [bound to] {producer_go_port (top.producer0.ports.producer_go_port)} (privacy: 0)
    | +-events : <top.pipe.events 1 events> {builtin} (privacy: 0)
    | | +-do_work_event : <top.pipe.events.do_work_event> (privacy: 0)
    | +-stats : <top.pipe.stats 0 stats, 1 counters> {builtin} (privacy: 0)
    | | +-num_processed_ : <top.pipe.stats.num_processed_ val:101 normal vis:100000000> (privacy: 0)
    | +-? : <top.pipe:log_msg_src cat:"info" observed:false msgs:0> (_sparta_log_msg_source_[0])  (privacy: 0)
    | +-? : <top.pipe:log_msg_src cat:"warning" observed:true msgs:0> (_sparta_log_msg_source_[1])  (privacy: 0)
    | +-? : <top.pipe:log_msg_src cat:"debug" observed:false msgs:0> (_sparta_log_msg_source_[2])  (privacy: 0)
    | +-? : <top.pipe:log_msg_src cat:"info" observed:false msgs:0> (_sparta_log_msg_source_[3])  (privacy: 0)
    +-producer0 : <top.producer0 resource: "producer"> (producer[0])  (privacy: 0)
    | +-params : <top.producer0.params 3 params> {builtin} (privacy: 0)
    | | +-max_ints_to_send : [<top.producer0.params.max_ints_to_send tags:[SPARTA_Parameter]>]<param uint32_t max_ints_to_send=100, def=100, write=0 read: 1 ignored: 0> (privacy: 0)
    | | +-test_param : [<top.producer0.params.test_param tags:[SPARTA_Parameter]>]<param uint32_t test_param=1, def=0, write=1 read: 1 ignored: 0 VOLATILE> (privacy: 0)
    | | +-arch_override_test_param : [<top.producer0.params.arch_override_test_param tags:[SPARTA_Parameter]>]<param std::string arch_override_test_param=reset_in_constructor, def=arch_override_default_value, write=1 read: 0 ignored: 1> (privacy: 0)
    | +-ports : <top.producer0.ports> (privacy: 0)
    | | +-producer_out_port : [bound to] {pipe_in_port (top.pipe.ports.pipe_in_port)} (privacy: 0)
    | | +-producer_go_port : [bound to] {pipe_go_producer_port (top.pipe.ports.pipe_go_producer_port)} (privacy: 0)
    | | | +-events : <top.producer0.ports.producer_go_port.events 1 events> {builtin} (privacy: 0)
    | | | | +-producer_go_port_forward_event : <top.producer0.ports.producer_go_port.events.producer_go_port_forward_event> (privacy: 0)
    | +-events : <top.producer0.events 1 events> {builtin} (privacy: 0)
    | | +-ev_producing_event : <top.producer0.events.ev_producing_event> (privacy: 0)
    | +-stats : <top.producer0.stats 0 stats, 1 counters> {builtin} (privacy: 0)
    | | +-num_produced : <top.producer0.stats.num_produced val:100 normal vis:100000000> (privacy: 0)
    | +-? : <top.producer0:log_msg_src cat:"info" observed:false msgs:0> (_sparta_log_msg_source_[0])  (privacy: 0)
    | +-? : <top.producer0:log_msg_src cat:"warning" observed:true msgs:1> (_sparta_log_msg_source_[1])  (privacy: 0)
    | +-? : <top.producer0:log_msg_src cat:"debug" observed:false msgs:0> (_sparta_log_msg_source_[2])  (privacy: 0)
    | +-? : <top.producer0:log_msg_src cat:"info" observed:false msgs:0> (_sparta_log_msg_source_[3])  (privacy: 0)
    +-sparta_expression_trigger_fired : <top.sparta_expression_trigger_fired name:"sparta_expression_trigger_fired" datat:(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)  observers:0 posted:0> (privacy: 0)
    ```

+ To generate an html report, ```./sparta_3stage --report-all output.html html``` . The report format could also be ```yaml```.
+ Selective logging can be done. For example to view logs related to ```pipe```,
    
    ```  ./sparta_3stage -l top.pipe info pipe_info.txt ```
    This would dump the information in a log file, ```pipe_info.txt```.
+ A list of command line usage scenarios is already documented in the main (documentation](https://sparcians.github.io/map/core_example.html) of Sparta.
