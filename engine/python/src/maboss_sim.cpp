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
     maboss_sim.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2020
*/

#include "maboss_sim.h"
#include "maboss_commons.h"
#include "maboss_res.h"
#include "maboss_resfinal.h"

#include "engines/MaBEstEngine.h"
#include "engines/FinalStateSimulationEngine.h"
#include <sstream>

#ifdef __GLIBC__
#include <malloc.h>
#endif

using namespace MaBoSS;

PyMethodDef cMaBoSSSim_methods[] = {
    {"run", (PyCFunction) cMaBoSSSim_run, METH_VARARGS | METH_KEYWORDS, "runs the simulation"},
    {"check", (PyCFunction) cMaBoSSSim_check, METH_VARARGS | METH_KEYWORDS, "checks the model"},
    {"copy", (PyCFunction) cMaBoSSSim_copy, METH_NOARGS, "returns a copy of the simulation"},
    {"str_bnd", (PyCFunction) cMaBoSSSim_bnd_str, METH_VARARGS | METH_KEYWORDS, "returns the contents of the bnd file"},
    {"str_cfg", (PyCFunction) cMaBoSSSim_cfg_str, METH_VARARGS | METH_KEYWORDS, "checks the contents of the cfg file"},
    {"get_logical_rules", (PyCFunction) cMaBoSSSim_get_logical_rules, METH_VARARGS | METH_KEYWORDS, "returns logical formulas"},
    {"update_parameters", (PyCFunction) cMaBoSSSim_update_parameters, METH_VARARGS | METH_KEYWORDS, "changes the parameters of the simulation"},
    {"get_nodes", (PyCFunction) cMaBoSSSim_get_nodes, METH_NOARGS, "returns the list of nodes"},
    {"mutate", (PyCFunction) cMaBoSSSim_mutate, METH_VARARGS | METH_KEYWORDS, "mutates a node of the network"},
    {NULL}  /* Sentinel */
};

PyMemberDef cMaBoSSSim_members[] = {
    {"network", T_OBJECT_EX, offsetof(cMaBoSSSimObject, network), READONLY},
    {"config", T_OBJECT_EX, offsetof(cMaBoSSSimObject, config), READONLY},
    {"param", T_OBJECT_EX, offsetof(cMaBoSSSimObject, param), READONLY},
    {NULL}  /* Sentinel */
};

PyTypeObject cMaBoSSSim = {
  PyVarObject_HEAD_INIT(NULL, 0)
  build_type_name("cMaBoSSSimObject"),               /* tp_name */
  sizeof(cMaBoSSSimObject),               /* tp_basicsize */
    0,                              /* tp_itemsize */
  (destructor) cMaBoSSSim_dealloc,      /* tp_dealloc */
    0,                              /* tp_vectorcall_offset */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_as_async */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,         /* tp_flags */
  "cMaBoSS Simulation object",                   /* tp_doc */
  (traverseproc) cMaBoSSSim_traverse,           /* tp_traverse */
  (inquiry) cMaBoSSSim_clear,                   /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
  cMaBoSSSim_methods,                              /* tp_methods */
  cMaBoSSSim_members,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
  cMaBoSSSim_init,                              /* tp_init */
    0,                              /* tp_alloc */
  cMaBoSSSim_new,                      /* tp_new */    
};

// the members are typed pointers, so go through PyObject* explicitly rather
// than letting the Py_VISIT / Py_CLEAR macros cast them
int cMaBoSSSim_traverse(cMaBoSSSimObject *self, visitproc visit, void *arg)
{
  Py_VISIT((PyObject *) self->network);
  Py_VISIT((PyObject *) self->config);
  Py_VISIT((PyObject *) self->param);
  return 0;
}

int cMaBoSSSim_clear(cMaBoSSSimObject *self)
{
  PyObject* tmp;
  tmp = (PyObject *) self->network; self->network = NULL; Py_XDECREF(tmp);
  tmp = (PyObject *) self->config;  self->config  = NULL; Py_XDECREF(tmp);
  tmp = (PyObject *) self->param;   self->param   = NULL; Py_XDECREF(tmp);
  return 0;
}

void cMaBoSSSim_dealloc(cMaBoSSSimObject *self)
{
    PyObject_GC_UnTrack(self);
    cMaBoSSSim_clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

int cMaBoSSSim_init(PyObject* self, PyObject *args, PyObject* kwargs)  
{
  PyObject * network_file = Py_None;
  PyObject * config_file = Py_None;
  PyObject * config_files = Py_None;
  PyObject * network_str = Py_None;
  PyObject * config_str = Py_None;
  PyObject * net = Py_None;
  PyObject * cfg = Py_None;
  PyObject * use_sbml_names = Py_False;
  const char *kwargs_list[] = {"network", "config", "configs", "network_str", "config_str", "net", "cfg", "use_sbml_names", NULL};
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "|OOOOOOOO", const_cast<char **>(kwargs_list), 
    &network_file, &config_file, &config_files, &network_str, &config_str, &net, &cfg, &use_sbml_names
  )) {
    return -1;
  }
  cMaBoSSSimObject* py_simulation = (cMaBoSSSimObject *) self;

  try {
    PyObject* new_network = NULL;
    if (net != Py_None)
    {
      // borrowed from the caller: take our own reference
      if (!PyObject_IsInstance(net, (PyObject*) &cMaBoSSNetwork)) {
        PyErr_SetString(PyExc_TypeError, "net must be a MaBoSSNet object");
        return -1;
      }
      Py_INCREF(net);
      new_network = net;

    } else {
      new_network = PyObject_CallFunction((PyObject *) &cMaBoSSNetwork,
        "OOO", network_file, network_str, use_sbml_names
      );
    }

    if (new_network == NULL) {
      return -1;
    }
    Py_XSETREF(py_simulation->network, (cMaBoSSNetworkObject*) new_network);

    PyObject* new_config = NULL;
    if (cfg != Py_None)
    {
      if (!PyObject_IsInstance(cfg, (PyObject*) &cMaBoSSConfig)) {
        PyErr_SetString(PyExc_TypeError, "cfg must be a MaBoSSCfg object");
        return -1;
      }
      Py_INCREF(cfg);
      new_config = cfg;

    } else {
      new_config = PyObject_CallFunction((PyObject *) &cMaBoSSConfig,
        "OOOO", (PyObject *) py_simulation->network, config_file, config_files, config_str
      );
    }

    if (new_config == NULL) {
      return -1;
    }
    Py_XSETREF(py_simulation->config, (cMaBoSSConfigObject*) new_config);

    PyObject* new_param = PyObject_CallFunction((PyObject *) &cMaBoSSParam,
      "OO", (PyObject *) py_simulation->network, (PyObject *) py_simulation->config
    );

    if (new_param == NULL) {
      return -1;
    }
    Py_XSETREF(py_simulation->param, (cMaBoSSParamObject*) new_param);

    // Error checking
    IStateGroup::checkAndComplete(py_simulation->network->network);
    py_simulation->network->network->getSymbolTable()->checkSymbols();
  }
  catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return -1;
  }
  
  return 0;
}

PyObject * cMaBoSSSim_new(PyTypeObject* type, PyObject *args, PyObject* kwargs) 
{
  cMaBoSSSimObject* py_simulation = (cMaBoSSSimObject *) type->tp_alloc(type, 0);
  if (py_simulation == NULL) {
    return NULL;
  }
  py_simulation->network = NULL;
  py_simulation->config = NULL;
  py_simulation->param = NULL;

  return (PyObject *) py_simulation;
}

// Builds and runs the engine without the GIL, so that simulations started from
// several Python threads run in parallel. Returns NULL with a Python exception set
// on failure.
template <typename Engine>
static Engine* cMaBoSSSim_run_engine(Network* network, RunConfig* config, time_t* start_time) {
  Engine* simulation = NULL;
  bool failed = false;
  std::string error;

  Py_BEGIN_ALLOW_THREADS
  try {
    simulation = new Engine(network, config);
    time(start_time);
    simulation->run(NULL);
  } catch (BNException& e) {
    failed = true;
    error = e.getMessage();
  }

#ifdef __GLIBC__
  malloc_trim(0);
#endif
  Py_END_ALLOW_THREADS

  if (failed) {
    delete simulation;
    PyErr_SetString(PyBNException, error.c_str());
    return NULL;
  }
  return simulation;
}

PyObject* cMaBoSSSim_run(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  
  int only_last_state = 0;
  const char *kwargs_list[] = {"only_last_state", NULL};
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "|i", const_cast<char **>(kwargs_list), 
    &only_last_state
  ))
    return NULL;
    
  bool b_only_last_state = (only_last_state != 0);
  time_t start_time, end_time;

  RandomGenerator::resetGeneratedNumberCount();
  if (b_only_last_state) {

    // go through tp_new so the object is GC-tracked and zero-initialised
    cMaBoSSResultFinalObject* res = (cMaBoSSResultFinalObject*) cMaBoSSResultFinal_new(&cMaBoSSResultFinal, NULL, NULL);
    if (res == NULL) {
      return NULL;
    }

    FinalStateSimulationEngine* simulation = cMaBoSSSim_run_engine<FinalStateSimulationEngine>(self->network->network, self->config->config, &start_time);
    if (simulation == NULL) {
      Py_DECREF(res);
      return NULL;
    }

    time(&end_time);
    res->network = self->network->network;
    res->runconfig = self->config->config;
    res->engine = simulation;
    res->start_time = start_time;
    res->end_time = end_time;
    // keep the network and the config alive for as long as the result is
    Py_INCREF(self->network);
    res->py_network = (PyObject *) self->network;
    Py_INCREF(self->config);
    res->py_config = (PyObject *) self->config;
    return (PyObject*) res;
  } else {

    cMaBoSSResultObject* res = (cMaBoSSResultObject*) cMaBoSSResult_new(&cMaBoSSResult, NULL, NULL);
    if (res == NULL) {
      return NULL;
    }

    MaBEstEngine* simulation = cMaBoSSSim_run_engine<MaBEstEngine>(self->network->network, self->config->config, &start_time);
    if (simulation == NULL) {
      Py_DECREF(res);
      return NULL;
    }

    time(&end_time);

    res->network = self->network->network;
    res->runconfig = self->config->config;
    res->engine = simulation;
    res->start_time = start_time;
    res->end_time = end_time;
    Py_INCREF(self->network);
    res->py_network = (PyObject *) self->network;
    Py_INCREF(self->config);
    res->py_config = (PyObject *) self->config;
    return (PyObject*) res;
  }
}


PyObject* cMaBoSSSim_check(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  try {
    IStateGroup::checkAndComplete(self->network->network);
    self->network->network->getSymbolTable()->checkSymbols();
  } catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  }
  Py_RETURN_NONE;
}

PyObject* cMaBoSSSim_get_logical_rules(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  
  std::ostringstream ss;
  try {
    self->network->network->generateLogicalExpressions(ss);
  } catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  } catch (std::exception& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
  return PyUnicode_FromString(ss.str().c_str());
}

PyObject* cMaBoSSSim_bnd_str(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  std::ostringstream bnd;
  self->network->network->display(bnd);
  return PyUnicode_FromString(bnd.str().c_str());
}


PyObject* cMaBoSSSim_cfg_str(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  std::ostringstream cfg;
  self->config->config->dump(self->network->network, cfg, MaBEstEngine::VERSION, false);
  return PyUnicode_FromString(cfg.str().c_str());
}

PyObject* cMaBoSSSim_update_parameters(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) 
{
  return cMaBoSSParam_update_parameters(self->param, args, kwargs);
}

PyObject* cMaBoSSSim_get_nodes(cMaBoSSSimObject* self) {

  PyObject *list = PyList_New(self->network->network->getNodes().size());

  size_t index = 0;
  for (auto* node: self->network->network->getNodes()) {
    PyList_SetItem(list, index, PyUnicode_FromString(node->getLabel().c_str()));
    index++;
  }

  return list;
}

PyObject* cMaBoSSSim_mutate(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs)
{
  PyObject *node_name = Py_None;
  PyObject *mutation = Py_None;
  PyObject* simple = Py_True;
  const char *kwargs_list[] = {"node_name", "mutation", "simple", NULL};
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "|OOO", const_cast<char **>(kwargs_list), 
    &node_name, &mutation, &simple
  ))
    return NULL;

  if (!PyUnicode_Check(node_name)) {
    PyErr_SetString(PyExc_TypeError, "node_name must be a string");
    return NULL;
  }
  if (!PyUnicode_Check(mutation)) {
    PyErr_SetString(PyExc_TypeError, "mutation must be a string");
    return NULL;
  }

  std::string node_name_str = PyUnicode_AsUTF8(node_name);
  std::string mutation_str = PyUnicode_AsUTF8(mutation);

  try {
    Node* node = self->network->network->getNode(node_name_str);
  
    bool is_activation = (mutation_str.compare("ON") == 0 || mutation_str.compare("on") == 0);
    if (simple == Py_True) {
      node->mutate((is_activation ? 1.0 : 0.0));
    } else {
      
      node->makeMutable(self->network->network);
      SymbolTable* symbol_table = self->network->network->getSymbolTable();
      const Symbol* lowvar = symbol_table->getSymbol("$Low_" + node_name_str);
      const Symbol* highvar = symbol_table->getSymbol("$High_" + node_name_str);
      symbol_table->setSymbolValue(lowvar, (is_activation ? 0.0 : 1.0));
      symbol_table->setSymbolValue(highvar, (is_activation ? 1.0 : 0.0));
    }
  }
  catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  }
  
  
  Py_RETURN_NONE;
}

PyObject* cMaBoSSSim_copy(cMaBoSSSimObject* self) {
  std::ostringstream bnd;
  self->network->network->display(bnd);
  
  std::ostringstream cfg;
  self->config->config->dump(self->network->network, cfg, MaBEstEngine::VERSION, false);
   
  PyObject *args = PyTuple_New(0);
  // "N" steals, so the two strings are owned by the dict rather than leaked
  PyObject *kwargs = Py_BuildValue("{s:N,s:N}",
    "network_str", PyUnicode_FromString(bnd.str().c_str()),
    "config_str", PyUnicode_FromString(cfg.str().c_str())
  );
  if (args == NULL || kwargs == NULL) {
    Py_XDECREF(args);
    Py_XDECREF(kwargs);
    return NULL;
  }

  PyObject* simulation = PyObject_Call((PyObject *) &cMaBoSSSim, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);

  return simulation;
}
