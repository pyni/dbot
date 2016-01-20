/*
 * This is part of the Bayesian Object Tracking (bot),
 * (https://github.com/bayesian-object-tracking)
 *
 * Copyright (c) 2015 Max Planck Society,
 * 				 Autonomous Motion Department,
 * 			     Institute for Intelligent Systems
 *
 * This Source Code Form is subject to the terms of the GNU General Public
 * License License (GNU GPL). A copy of the license can be found in the LICENSE
 * file distributed with this source code.
 */

/**
 * \file rbc_particle_filter_tracker_builder.hpp
 * \date November 2015
 * \author Jan Issac (jan.issac@gmail.com)
 */

#pragma once

#include <exception>

#include <dbot/util/object_model_loader.hpp>
#include <dbot/util/object_resource_identifier.hpp>
#include <dbot/tracker/rbc_particle_filter_object_tracker.hpp>
#include <dbot/tracker/builder/object_transition_model_builder.hpp>
#include <dbot/tracker/builder/rb_observation_model_builder.h>

namespace dbot
{

/**
 * \brief Represents an Rbc Particle filter based tracker builder
 */
template <typename Tracker>
class RbcParticleFilterTrackerBuilder
{
public:
    typedef typename Tracker::State State;
    typedef typename Tracker::Noise Noise;
    typedef typename Tracker::Input Input;

    /* == Model Builder Interfaces ========================================== */
    typedef StateTransitionFunctionBuilder<State, Noise, Input>
        StateTransitionBuilder;
    typedef RbObservationModelBuilder<State> ObservationModelBuilder;

    /* == Model Interfaces ================================================== */
    typedef fl::StateTransitionFunction<State, Noise, Input> StateTransition;
    typedef RbObservationModel<State> ObservationModel;
    typedef typename ObservationModel::Observation Obsrv;

    /* == Filter algorithm ================================================== */
    typedef RaoBlackwellCoordinateParticleFilter<StateTransition,
                                                 ObservationModel> Filter;

    /* == Tracker parameters ================================================ */
    struct Parameters
    {
        int evaluation_count;
        double moving_average_update_rate;
        double max_kl_divergence;
    };

public:
    /**
     * \brief Creates a RbcParticleFilterTrackerBuilder
     * \param param			Builder and sub-builder parameters
     * \param camera_data	Tracker camera data object
     */
    RbcParticleFilterTrackerBuilder(
        const std::shared_ptr<StateTransitionBuilder>& state_transition_builder,
        const std::shared_ptr<ObservationModelBuilder>& obsrv_model_builder,
        const std::shared_ptr<ObjectModel>& object_model,
        const std::shared_ptr<CameraData>& camera_data,
        const Parameters& params)
        : state_transition_builder_(state_transition_builder),
          obsrv_model_builder_(obsrv_model_builder),
          object_model_(object_model),
          camera_data_(camera_data),
          params_(params)
    {
    }

    /**
     * \brief Builds the Rbc PF tracker
     */
    virtual std::shared_ptr<RbcParticleFilterObjectTracker> build()
    {
        auto filter = create_filter(object_model_, params_.max_kl_divergence);

        auto tracker = std::make_shared<RbcParticleFilterObjectTracker>(
            filter,
            object_model_,
            camera_data_,
            params_.evaluation_count,
            params_.moving_average_update_rate);

        return tracker;
    }

    /**
     * \brief Creates an instance of the Rbc particle filter
     *
     * \throws NoGpuSupportException if compile with DBOT_BUILD_GPU=OFF and
     *         attempting to build a tracker with GPU support
     */
    virtual std::shared_ptr<Filter> create_filter(
        const std::shared_ptr<ObjectModel>& object_model,
        double max_kl_divergence)
    {
        auto state_transition_model = state_transition_builder_->build();
        auto obsrv_model = obsrv_model_builder_->build();

        auto sampling_blocks =
            create_sampling_blocks(object_model->count_parts(),
                                   state_transition_model->noise_dimension() /
                                       object_model->count_parts());

        auto filter = std::shared_ptr<Filter>(new Filter(state_transition_model,
                                                         obsrv_model,
                                                         sampling_blocks,
                                                         max_kl_divergence));

        return filter;
    }

    /**
     * \brief Creates a sampling block definition used by the coordinate
     *        particle filter
     *
     * \param blocks		Number of objects or object parts
     * \param block_size	State dimension of each part
     */
    virtual std::vector<std::vector<size_t>> create_sampling_blocks(
        int blocks,
        int block_size) const
    {
        std::vector<std::vector<size_t>> sampling_blocks(blocks);
        for (int i = 0; i < blocks; ++i)
        {
            for (int k = 0; k < block_size; ++k)
            {
                sampling_blocks[i].push_back(i * block_size + k);
            }
        }

        return sampling_blocks;
    }

private:
    std::shared_ptr<StateTransitionBuilder> state_transition_builder_;
    std::shared_ptr<ObservationModelBuilder> obsrv_model_builder_;
    std::shared_ptr<ObjectModel> object_model_;
    std::shared_ptr<CameraData> camera_data_;
    Parameters params_;
};
}
