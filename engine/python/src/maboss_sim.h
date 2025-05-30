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
     maboss_sim.h

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2020
*/

#ifndef MABOSS_SIM
#define MABOSS_SIM

#include "maboss_commons.h"
#include "maboss_net.h"
#include "maboss_cfg.h"
#include "maboss_param.h"

typedef struct {
  PyObject_HEAD
  cMaBoSSNetworkObject* network;
  cMaBoSSConfigObject* config;
  cMaBoSSParamObject* param;
} cMaBoSSSimObject;

void cMaBoSSSim_dealloc(cMaBoSSSimObject *self);
int cMaBoSSSim_init(PyObject* self, PyObject *args, PyObject* kwargs);
PyObject * cMaBoSSSim_new(PyTypeObject* type, PyObject *args, PyObject* kwargs);
PyObject* cMaBoSSSim_run(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs);
PyObject* cMaBoSSSim_check(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs);
PyObject* cMaBoSSSim_get_logical_rules(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs);
PyObject* cMaBoSSSim_bnd_str(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs);
PyObject* cMaBoSSSim_cfg_str(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs);
PyObject* cMaBoSSSim_update_parameters(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs);
PyObject* cMaBoSSSim_get_nodes(cMaBoSSSimObject* self);
PyObject* cMaBoSSSim_mutate(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs);
PyObject* cMaBoSSSim_copy(cMaBoSSSimObject* self);

#endif