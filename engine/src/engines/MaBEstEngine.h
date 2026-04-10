/*
#############################################################################
#                                                                           #
# BSD 3-Clause License (see https://opensource.org/licenses/BSD-3-Clause)   #
#                                                                           #
# Copyright (c) 2011-2020 Institut Curie, 26 rue d'Ulm, Paris, France       #
# All rights reserved.                                                      #
#                                                                           #
# Redistribution and use in source and binary forms, with or without        #
# modification, are permitted provided that the following conditions are    #
# met:                                                                      #
#                                                                           #
# 1. Redistributions of source code must retain the above copyright notice, #
# this list of conditions and the following disclaimer.                     #
#                                                                           #
# 2. Redistributions in binary form must reproduce the above copyright      #
# notice, this list of conditions and the following disclaimer in the       #
# documentation and/or other materials provided with the distribution.      #
#                                                                           #
# 3. Neither the name of the copyright holder nor the names of its          #
# contributors may be used to endorse or promote products derived from this #
# software without specific prior written permission.                       #
#                                                                           #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED #
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           #
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER #
# OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  #
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       #
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        #
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    #
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      #
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        #
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              #
#                                                                           #
#############################################################################

   Module:
     MaBEstEngine.h

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>

   Date:
     January-March 2011
*/

#ifndef _MABESTENGINE_H_
#define _MABESTENGINE_H_

#include <string>
#include <vector>
#include <assert.h>

#include "ProbTrajEngine.h"
#include "../Cumulator.h"
#include "../RandomGenerator.h"
#include "../RunConfig.h"

struct ArgWrapper;

class MaBEstEngine : public ProbTrajEngine {
    std::vector<ArgWrapper *> arg_wrapper_v;

    /// Starts \link runThread \endlink in a thread, catches exceptions
    /// @param arg pointer to the data required in the thread
    static void *threadWrapper(void *arg);

    /// Finalizes the computation by merging all the results
    void epilogue();

    /// Runs the simulation in a thread started by \link run \endlink. It simulates the evolution across time, computes
    /// possible transitions, updates statistics, and optionally saves trajectories and builds the graph of
    /// observed states
    /// @param cumulator where to put the results. Will be used to compute the final results.
    /// @param start_count_thread Starting index of the trajectories/simulations of the thread.
    /// @param sample_count_thread number of trajectories to evaluate
    /// @param randgen_factory the factory to "create" the random aspect
    /// @param elapsed_time pointer to the time took by the thread (ms)
    /// @param seed Seed used by the random generator. Useful for reproducibility
    /// @param fixpoint_map Structure used to count fixed points (states where no transition is possible anymore)
    /// @param observed_map Pointer to the graph of observed states and transitions
    /// @param output_traj optional output stream that prints trajectory information
    /// (trajectory number, initial state, successive states, times, and transition-related value)
    void runThread(Cumulator<NetworkState> *cumulator, unsigned int start_count_thread,
                   unsigned int sample_count_thread, RandomGeneratorFactory *randgen_factory,
                   long long int *elapsed_time, int seed, FixedPoints *fixpoint_map, ObservedGraph *observed_map,
                   std::ostream *output_traj);

public:
    static const std::string VERSION;

#ifdef MPI_COMPAT
    MaBEstEngine(Network *network, RunConfig *runconfig, int world_size, int world_rank);
#else
    MaBEstEngine(Network *network, RunConfig *runconfig);
#endif

    /// Main function that launches the whole execution of the simulation. It distributes the trajectories between the
    /// threads and executes the final phase while recording execution times.
    /// @param output_traj (optional) prints info about trajectories (see \link runThread \endlink )
    void run(std::ostream *output_traj = NULL);

    /// Displays various info about the simulation
    /// @param os reference to the output stream
    /// @param start_time start time of the simulation
    /// @param end_time end time of the simulation
    void displayRunStats(std::ostream &os, time_t start_time, time_t end_time) const;

    ~MaBEstEngine();
};

/**
 * @class MaBEstEngine
 *
 */

#endif
