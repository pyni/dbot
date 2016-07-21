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

/*
 * This file implements a part of the algorithm published in:
 *
 * J. Issac, M. Wuthrich, C. Garcia Cifuentes, J. Bohg, S. Trimpe, S. Schaal
 * Depth-Based Object Tracking Using a Robust Gaussian Filter
 * IEEE Intl Conf on Robotics and Automation, 2016
 * http://arxiv.org/abs/1602.06157
 *
 */

/**
 * \file rms_gaussian_filter_object_tracker.hpp
 * \author Jan Issc (jan.issac@gmail.com)
 * \date December 2015
 */

#pragma once

#include <dbot/tracker/object_tracker.hpp>
#include <dbot/model/depth_pixel_model.hpp>

#include <fl/util/types.hpp>
#include <fl/model/process/linear_state_transition_model.hpp>
#include <fl/model/observation/uniform_observation_model.hpp>
#include <fl/model/observation/body_tail_observation_model.hpp>
#include <fl/filter/gaussian/robust_multi_sensor_gaussian_filter.hpp>

namespace dbot
{
class RmsGaussianFilterObjectTracker : public ObjectTracker
{
public:
    /* ---------------------------------------------------------------------- */
    /* - State Transition Model                                             - */
    /* ---------------------------------------------------------------------- */
    typedef fl::LinearTransition<State, Noise, Input> Transition;

    /* ---------------------------------------------------------------------- */
    /* - Observation Model                                                  - */
    /* ---------------------------------------------------------------------- */
    // Pixel Level: Body model
    typedef fl::DepthPixelModel<State> PixelModel;

    // Pixel Level: Tail model
    typedef fl::UniformSensor<State> TailModel;

    // Pixel Level: Body-Tail model
    typedef fl::BodyTailSensor<PixelModel, TailModel> BodyTailPixelModel;

    // Image Level: create many of BodyTailPixelModel
    typedef fl::JointSensor<
        fl::MultipleOf<BodyTailPixelModel, Eigen::Dynamic>> Sensor;

    /* ---------------------------------------------------------------------- */
    /* - Quadrature                                                         - */
    /* ---------------------------------------------------------------------- */
    typedef fl::SigmaPointQuadrature<fl::UnscentedTransform> Quadrature;

    /* ---------------------------------------------------------------------- */
    /* - Filter                                                             - */
    /* ---------------------------------------------------------------------- */
    typedef fl::RobustMultiSensorGaussianFilter<Transition,
                                                Sensor,
                                                Quadrature> Filter;

    typedef typename fl::Traits<Filter>::Belief Belief;

public:
    /**
     * \brief Creates the tracker
     *
     * \param filter
     *     Rbc particle filter instance
     * \param object_model
     *     Object model instance
     * \param camera_data
     *     Camera data container
     * \param update_rate
     *     Moving average update rate
     */
    RmsGaussianFilterObjectTracker(
        const std::shared_ptr<Filter>& filter,
        const std::shared_ptr<ObjectModel>& object_model,
        double update_rate);

    /**
     * \brief perform a single filter step
     *
     * \param image
     *     Current observation image
     */
    State on_track(const Obsrv& image);

    /**
     * \brief Initializes the particle filter with the given initial states and
     *    the number of evaluations
     * @param initial_states
     * @param evaluation_count
     */
    State on_initialize(const std::vector<State>& initial_states);

private:
    std::shared_ptr<Filter> filter_;
    Belief belief_;
};
}
