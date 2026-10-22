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
     maboss_res.h

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2020
*/

#ifndef MABOSS_RES
#define MABOSS_RES

#include "maboss_commons.h"

#include "Network.h"
#include "RunConfig.h"
#include "engines/MaBEstEngine.h"

typedef struct {
  PyObject_HEAD
  // py_network / py_config keep the wrappers alive; network and runconfig are
  // cached raw pointers borrowed from them and are never freed here
  PyObject* py_network;
  PyObject* py_config;
  Network* network;
  RunConfig* runconfig;
  MaBEstEngine* engine;
  time_t start_time;
  time_t end_time;
  PyObject* probtraj;
  PyObject* last_probtraj;
  PyObject* observed_graph;
  PyObject* observed_durations;
  PyObject* statdist;
} cMaBoSSResultObject;

void cMaBoSSResult_dealloc(cMaBoSSResultObject *self);
int cMaBoSSResult_traverse(cMaBoSSResultObject *self, visitproc visit, void *arg);
int cMaBoSSResult_clear(cMaBoSSResultObject *self);
PyObject * cMaBoSSResult_new(PyTypeObject* type, PyObject *args, PyObject* kwargs);
bool cMaBoSSResult_parse_node_list(Network* network, PyObject* pList, std::vector<Node*>& list_nodes);
PyObject* cMaBoSSResult_get_fp_table(cMaBoSSResultObject* self);
PyObject* cMaBoSSResult_get_observed_graph(cMaBoSSResultObject* self);
PyObject* cMaBoSSResult_get_observed_durations(cMaBoSSResultObject* self);
PyObject* cMaBoSSResult_get_probtraj(cMaBoSSResultObject* self);
PyObject* cMaBoSSResult_get_last_probtraj(cMaBoSSResultObject* self);
PyObject* cMaBoSSResult_get_nodes_probtraj(cMaBoSSResultObject* self, PyObject* args);
PyObject* cMaBoSSResult_get_last_nodes_probtraj(cMaBoSSResultObject* self, PyObject* args);
PyObject* cMaBoSSResult_get_entropy_trajectory(cMaBoSSResultObject* self);
PyObject* cMaBoSSResult_get_statdist(cMaBoSSResultObject* self);
PyObject* cMaBoSSResult_get_statdist_clusters(cMaBoSSResultObject* self);
PyObject* cMaBoSSResult_get_statdist_clusters_summary(cMaBoSSResultObject* self);
PyObject* cMaBoSSResult_display_fp(cMaBoSSResultObject* self, PyObject *args);
PyObject* cMaBoSSResult_display_probtraj(cMaBoSSResultObject* self, PyObject *args);
PyObject* cMaBoSSResult_display_statdist(cMaBoSSResultObject* self, PyObject *args);
PyObject* cMaBoSSResult_display_run(cMaBoSSResultObject* self, PyObject* args);

#endif