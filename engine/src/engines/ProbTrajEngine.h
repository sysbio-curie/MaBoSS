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
     MetaEngine.h

   Authors:
     Vincent Noel <contact@vincent-noel.fr>
 
   Date:
     March 2019
*/

#ifndef _PROBTRAJENGINE_H_
#define _PROBTRAJENGINE_H_

#include <string>
#include <map>
#include <vector>
#include <assert.h>

#ifdef MPI_COMPAT
#include <mpi.h>
#endif

#ifdef PYTHON_API
#include "../Python_headers.h"
#endif

#include "../Network.h"
#include "FixedPointEngine.h"
#include "../Cumulator.h"
#include "../ObservedGraph.h"
#include "../RunConfig.h"
#include "../displayers/FixedPointDisplayer.h"
#include "../displayers/ProbTrajDisplayer.h"

struct EnsembleArgWrapper;

class ProbTrajEngine : public FixedPointEngine {
protected:
    ObservedGraph *observed_graph;
    std::vector<ObservedGraph *> observed_graph_v;

    Cumulator<NetworkState> *merged_cumulator;
    std::vector<Cumulator<NetworkState> *> cumulator_v;
    std::map<double, std::map<Node *, Expression *> *> schedule;
    std::vector<double> schedule_times;

    /// Takes all the nodes' schedule to create a global schedule, sorted by time (sooner to later)
    void buildSchedule();

    /// Applies the scheduled events whose time is less than or equal to the current time; the node state is then set
    /// according to the expression result and a random dra
    /// @param random_generator pointer to the RG used
    /// @param network_state reference to the network state
    /// @param times reference to a vector with all the times
    /// @param time the time to compare the schedule to
    void applySchedule(RandomGenerator *random_generator, NetworkState &network_state, std::vector<double> &times,
                       double time);

    /// Utilitary function that merges the simulation results in a thread
    /// @param arg
    /// @return
    static void *threadMergeWrapper(void *arg);

    /// Merges the accumulated results pairwise, by levels
    /// @param cumulator_v reference to the cumulator stocking the results
    /// @param fixpoint_map_v reference to the map of fixpoints
    /// @param observed_graph_v reference to the observed graph
    static void mergeResults(std::vector<Cumulator<NetworkState> *> &cumulator_v,
                             std::vector<FixedPoints *> &fixpoint_map_v,
                             std::vector<ObservedGraph *> &observed_graph_v);

#ifdef MPI_COMPAT
    /// Merges the results of mutliple MPI process after the simulation
    /// @param runconfig pointer to the configuration of the simulation
    /// @param ret_cumul pointer to the final cumulator
    /// @param fixpoints pointer to the final fixpoints map
    /// @param graph pointer to the final states and transitions
    /// @param world_size total number of MPI processes
    /// @param world_rank MPI rank of the current process
    /// @param pack if null = true. If true, the data is sent as a packed buffer; otherwise it is sent with direct
    /// MPI calls
    static void mergeMPIResults(RunConfig *runconfig, Cumulator<NetworkState> *ret_cumul, FixedPoints *fixpoints,
                                ObservedGraph *graph, int world_size, int world_rank, bool pack = true);
#endif

public:
#ifdef MPI_COMPAT
    ProbTrajEngine(Network *network, RunConfig *runconfig, int world_size, int world_rank) : FixedPointEngine(
        network, runconfig, world_size, world_rank) {
    }
#else
    ProbTrajEngine(Network *network, RunConfig *runconfig) : FixedPointEngine(network, runconfig) {
    }
#endif

    Cumulator<NetworkState> *getMergedCumulator() {
        return merged_cumulator;
    }

    /// @return the max index of tick the simulation went to
    int getMaxTickIndex() const { return merged_cumulator->getMaxTickIndex(); }

    /// @return the final time of simulation associated to the final cumulator
    double getFinalTime() const { return merged_cumulator->getFinalTime(); }

    /// Displays the final statistical distribution computed by the simulation. Only used on the main process in MPI mode
    /// @param output_statdist pointer to the object in charge of writing or formating the statistical distribution
    void displayStatDist(StatDistDisplayer *output_statdist) const;

    /// Display the probabilities as a function of time. Only used on the main process in MPI mode
    /// @param displayer pointer to the object in charge of displaying
    void displayProbTraj(ProbTrajDisplayer<NetworkState> *displayer) const;

    /// Calls \link displayProbTraj \endlink ,\link displayStatDist \endlink , \link displayFixPoints \endlink
    /// @param probtraj_displayer object for the probabilities' evolution
    /// @param statdist_displayer object for the statistical distribution
    /// @param fp_displayer object for the fixpoints
    void display(ProbTrajDisplayer<NetworkState> *probtraj_displayer, StatDistDisplayer *statdist_displayer,
                 FixedPointDisplayer *fp_displayer) const;

    /// Displays the graph of the observed states and eventually the associated durations
    /// @param output_observed_graph
    /// @param output_observed_durations
    void displayObservedGraph(std::ostream *output_observed_graph, std::ostream *output_observed_durations);

#ifdef PYTHON_API
    PyObject *getNumpyObservedGraph();
    PyObject *getNumpyObservedDurations();
#endif
};

#endif
