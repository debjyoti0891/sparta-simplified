// <StageSimn.cpp> -*- C++ -*-


#include <iostream>

#include "StageSim.hpp"

#include "sparta/simulation/Clock.hpp"
#include "sparta/utils/TimeManager.hpp"
#include "sparta/simulation/TreeNode.hpp"
#include "sparta/utils/StringUtils.hpp"

#include "Producer.hpp"
#include "Consumer.hpp"
#include "Pipe.hpp"

StageSim::StageSim(sparta::Scheduler & scheduler, bool be_noisy) :
    sparta::app::Simulation("stage_simulator", &scheduler),
    be_noisy_(be_noisy)
{
    // Using the macro SPARTA_EXPECT_FALSE will tell the compiler that
    // most of the time, this if statement is falses -- it's an
    // optimization.  There is also a SPARTA_EXPECT_TRUE
    if(SPARTA_EXPECT_FALSE(be_noisy_)) {
        std::cout << "NOISE: " << __PRETTY_FUNCTION__ << ": Constructing" << std::endl;
    }

    // Set up all resources to be available through ResourceTreeNode.
    // These factories will be instantiated during the buildTree_
    // phase (after their registration -- see below).
    getResourceSet()->addResourceFactory<sparta::ResourceFactory<Producer, ProducerParameterSet>>();
    getResourceSet()->addResourceFactory<sparta::ResourceFactory<Consumer, Consumer::ConsumerParameterSet>>();
    getResourceSet()->addResourceFactory<sparta::ResourceFactory<Pipe, Pipe::PipeParameterSet>>();
}


StageSim::~StageSim()
{
    if(SPARTA_EXPECT_FALSE(be_noisy_)) {
        std::cout << __PRETTY_FUNCTION__ << ": Tearing down" << std::endl;
    }
    getRoot()->enterTeardown(); // Allow deletion of nodes without error now
}

void StageSim::buildTree_()
{
    if(SPARTA_EXPECT_FALSE(be_noisy_)) {
        std::cout << "NOISE: " << __PRETTY_FUNCTION__ << ": Building the ResourceTreeNodes -- not instantiated yet" << std::endl;
    }

    // TREE_BUILDING Phase.  See sparta::PhasedObject::TreePhase

    // Create a single consumer
    sparta::ResourceTreeNode* rtn =
        new sparta::ResourceTreeNode(getRoot(), "consumer", // Could use Consumer::name here...
                                   sparta::TreeNode::GROUP_NAME_NONE,  // Do not allow consumer[n] -- there's only one!
                                   sparta::TreeNode::GROUP_IDX_NONE,
                                   "Consumer Object",
                                   getResourceSet()->getResourceFactory(Consumer::name));
    to_delete_.emplace_back(rtn);


    // Create a single pipe
    sparta::ResourceTreeNode* pipe_rtn =
        new sparta::ResourceTreeNode(getRoot(), "pipe", 
                                   sparta::TreeNode::GROUP_NAME_NONE,  
                                   sparta::TreeNode::GROUP_IDX_NONE,
                                   "Pipe Object",
                                   getResourceSet()->getResourceFactory(Pipe::name));
    to_delete_.emplace_back(pipe_rtn);

    // Get the producer count from the created parameter in the
    // created ParameterSet.  Note that you get the ParameterSet, but
    // not the Consumer resource/unit -- that has not been created yet.
    sparta::ParameterSet * param_set = rtn->getParameterSet();
    const uint32_t num_producers = 1;

    // Create the producers
    for (uint32_t i = 0; i < num_producers; ++i)
    {
        std::stringstream nodeName, humanName;
        nodeName << "producer" << i;
        humanName << "Producer " << i;

        // We create resource tree nodes because each component of the
        // core requires parameters and a clock.  TreeNode does not
        // provide this.
        sparta::ResourceTreeNode* prod_tn =
            new sparta::ResourceTreeNode(getRoot(),
                                       nodeName.str(),
                                       "producer", i,  // Grouping, i.e. producer[n]
                                       humanName.str(),
                                       getResourceSet()->getResourceFactory(Producer::name));
        to_delete_.emplace_back(prod_tn);
    }

}

void StageSim::configureTree_()
{
    if(SPARTA_EXPECT_FALSE(be_noisy_)) {
        std::cout << "NOISE: " << __PRETTY_FUNCTION__
                  << ": Configuring the parameters in the ResourceTreeNodes, "
                  << "but not the simulated objects are still not instantiated yet!" << std::endl;
    }

    // In TREE_CONFIGURING phase
    // Configuration from command line is already applied
}

void StageSim::bindTree_()
{
    if(SPARTA_EXPECT_FALSE(be_noisy_)) {
        std::cout << "NOISE: " << __PRETTY_FUNCTION__
                  << ": The simulated objects are instantiated.  Can be bound now." << std::endl;
    }
    // In TREE_FINALIZED phase
    // Tree is finalized. Taps placed. No new nodes at this point
    // Bind appropriate ports

    sparta::TreeNode* root_tree_node = getRoot();
    sparta_assert(root_tree_node != nullptr);

    sparta::ParameterSet * param_set = root_tree_node->getChildAs<sparta::ParameterSet>("consumer.params");
    const uint32_t num_producers = 1;

    for (uint32_t i = 0; i < num_producers; ++i)
    {
        std::stringstream nodeName, humanName;
        nodeName << "producer" << i;

        // Bind the consumer to the pipe 
        sparta::bind(root_tree_node->getChildAs<sparta::Port>("pipe.ports.pipe_out_port"),
                   root_tree_node->getChildAs<sparta::Port>("consumer.ports.consumer_in_port"));

        sparta::bind(root_tree_node->getChildAs<sparta::Port>("pipe.ports.pipe_go_port"),
                   root_tree_node->getChildAs<sparta::Port>("consumer.ports." + nodeName.str() + "_go_port"));
        
        // bind pipe to the producer
        sparta::bind(root_tree_node->getChildAs<sparta::Port>(nodeName.str() + ".ports.producer_out_port"),
                   root_tree_node->getChildAs<sparta::Port>("pipe.ports.pipe_in_port"));

         sparta::bind(root_tree_node->getChildAs<sparta::Port>("pipe.ports.pipe_go_producer_port"),
                   root_tree_node->getChildAs<sparta::Port>(nodeName.str() + ".ports.producer_go_port"));

    }
}
