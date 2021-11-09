// <SkeletonSimulation.hpp> -*- C++ -*-

#pragma once

#include <cinttypes>

#include "sparta/app/Simulation.hpp"

/*!
 * \brief StageSim which builds the model and configures it
 */
class StageSim : public sparta::app::Simulation
{
public:

    /*!
     * \brief Construct StageSim
     * \param be_noisy Be verbose -- not necessary, just an skeleton
     */
    StageSim(sparta::Scheduler & scheduler, bool be_noisy);

    // Tear it down
    virtual ~StageSim();

private:

    //////////////////////////////////////////////////////////////////////
    // Setup

    //! Build the tree with tree nodes, but does not instantiate the
    //! unit yet
    void buildTree_() override;

    //! Configure the tree and apply any last minute parameter changes
    void configureTree_() override;

    //! The tree is now configured, built, and instantiated.  We need
    //! to bind things together.
    void bindTree_() override;

    //! Verbosity
    const bool be_noisy_ = false;
};

