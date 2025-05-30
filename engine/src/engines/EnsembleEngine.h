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
     EnsembleEngine.h

   Authors:
     Vincent Noel <contact@vincent-noel.fr>
 
   Date:
     March 2019
*/

#ifndef _ENSEMBLEENGINE_H_
#define _ENSEMBLEENGINE_H_

#include <string>
#include <map>
#include <vector>
#include <assert.h>

#include "ProbTrajEngine.h"
#include "../Network.h"
#include "../Cumulator.h"
#include "../RandomGenerator.h"
#include "../RunConfig.h"
#include "../ObservedGraph.h"

struct EnsembleArgWrapper;

class EnsembleEngine : public ProbTrajEngine {

  std::vector<Network*> networks;
  std::vector<Cumulator<NetworkState>*> cumulators_per_model; // The final Cumulators for each model
  std::vector<FixedPoints* > fixpoints_per_model; // The final fixpoints for each model
  std::vector<ObservedGraph* > observed_graph_per_model; // The final observed graph for each model
  
  bool save_individual_result; // Do we want to save individual model simulation result
  bool random_sampling; // Randomly select the number of simulation per model

  std::vector<std::vector<unsigned int> > simulation_indices_v; // The list of indices of models to simulate for each thread
  std::vector<std::vector<Cumulator<NetworkState>*> > cumulator_models_v; // The results for each model, by thread
  std::vector<std::vector<Cumulator<NetworkState>*> > cumulators_thread_v; // The results for each model, by model
  std::vector<std::vector<FixedPoints*> > fixpoints_models_v; // The fixpoints for each model, by thread
  std::vector<std::vector<FixedPoints*> > fixpoints_threads_v; // The fixpoints for each model, by thread
  std::vector<std::vector<ObservedGraph* > > observed_graph_models_v; // The observed graph for each model, by thread
  std::vector<std::vector<ObservedGraph* > > observed_graph_threads_v; // The observed graph for each model, by thread
  
  std::vector<EnsembleArgWrapper*> arg_wrapper_v;
  void epilogue();
  static void* threadWrapper(void *arg);
  void runThread(Cumulator<NetworkState>* cumulator, unsigned int start_count_thread, unsigned int sample_count_thread, RandomGeneratorFactory* randgen_factory, int seed, FixedPoints* fixpoint_map, ObservedGraph* observed_graph, std::ostream* output_traj, std::vector<unsigned int> simulation_ind, std::vector<Cumulator<NetworkState>*> t_models_cumulators, std::vector<FixedPoints* > t_models_fixpoints, std::vector<ObservedGraph*> t_models_observed_graphs);
  void displayIndividualFixpoints(unsigned int model_id, FixedPointDisplayer* fp_displayer) const;
  void mergeIndividual();

#ifdef MPI_COMPAT
  void mergeMPIIndividual(bool pack=true);
#endif

public:
  static const std::string VERSION;
  
#ifdef MPI_COMPAT
  EnsembleEngine(std::vector<Network*> network, RunConfig* runconfig, int world_size, int world_rank, bool save_individual_result=false, bool random_sampling=false);
#else
  EnsembleEngine(std::vector<Network*> network, RunConfig* runconfig, bool save_individual_result=false, bool random_sampling=false);
#endif

  void run(std::ostream* output_traj);

  void displayIndividual(unsigned int model_id, ProbTrajDisplayer<NetworkState>* probtraj_displayer, StatDistDisplayer* statdist_displayer, FixedPointDisplayer* fp_displayer) const;
  ~EnsembleEngine();
};

#endif
