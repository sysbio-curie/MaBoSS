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
     FinalStateSimulationEngine.h

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2011
*/

#ifndef _FINAL_STATE_SIMULATION_ENGINE_H_
#define _FINAL_STATE_SIMULATION_ENGINE_H_

#include <string>
#include <map>
#include <vector>
#include <assert.h>

#ifdef PYTHON_API
#define NO_IMPORT_ARRAY
#define PY_ARRAY_UNIQUE_SYMBOL MABOSS_ARRAY_API
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <numpy/arrayobject.h>
#endif

#include "MetaEngine.h"
#include "FixedPointEngine.h"
#include "../Network.h"
#include "../RandomGenerator.h"
#include "../RunConfig.h"
#include "../displayers/FinalStateDisplayer.h"

struct FinalStateArgWrapper;

class FinalStateSimulationEngine : public MetaEngine {
  
  bool has_internal = false;
  NetworkState internal_state;

  std::vector<unsigned int> sample_count_per_thread;

  std::vector<FinalStateArgWrapper*> arg_wrapper_v;
  NodeIndex getTargetNode(RandomGenerator* random_generator, const std::vector<double>& nodeTransitionRates, double total_rate) const;
  void epilogue();
  static void* threadWrapper(void *arg);
  void runThread(unsigned int start_count_thread, unsigned int sample_count_thread, RandomGeneratorFactory* randgen_factory, int seed, FixedPoints* final_state_map, std::ostream* output_traj);
  
  FixedPoints* mergeFinalStateMaps();
  STATE_MAP<NetworkState_Impl, double> final_states;
  std::vector<FixedPoints*> final_states_map_v;

public:
  static const std::string VERSION;
  
#ifdef MPI_COMPAT
  FinalStateSimulationEngine(Network* network, RunConfig* runconfig, int world_size, int world_rank);
#else
  FinalStateSimulationEngine(Network* network, RunConfig* runconfig);
#endif

  void run(std::ostream* output_traj);
  ~FinalStateSimulationEngine();

  const STATE_MAP<Node*, double> getFinalNodes() const;
  double getFinalTime() const { return max_time; }

#ifdef PYTHON_API
  PyObject* getNumpyLastStatesDists() const;
  std::vector<Node*> getNodes() const;
  PyObject* getNumpyLastNodesDists(std::vector<Node*> output_nodes) const;
#endif

  void displayFinal(FinalStateDisplayer* displayer) const;
  
  void displayRunStats(std::ostream& os, time_t start_time, time_t end_time) const;

};

#endif
