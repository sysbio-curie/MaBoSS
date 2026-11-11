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
     popmaboss_sim.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     March 2021
*/

#include "popmaboss_sim.h"
#include "engines/PopMaBEstEngine.h"
#include "popmaboss_res.h"

#ifdef __GLIBC__
#include <malloc.h>
#endif

using namespace MaBoSS;

PyMethodDef cPopMaBoSSSim_methods[] = {

    {"get_nodes", (PyCFunction) cPopMaBoSSSim_get_nodes, METH_NOARGS, "gets the list of nodes"},
    {"copy", (PyCFunction) cPopMaBoSSSim_copy, METH_NOARGS, "copy the simulation"},
    {"str_bnd", (PyCFunction) cPopMaBoSSSim_bnd_str, METH_VARARGS | METH_KEYWORDS, "returns the contents of the pbnd file"},
    {"str_cfg", (PyCFunction) cPopMaBoSSSim_cfg_str, METH_VARARGS | METH_KEYWORDS, "checks the contents of the cfg file"},
    {"run", (PyCFunction) cPopMaBoSSSim_run, METH_VARARGS | METH_KEYWORDS, "runs the simulation"},
    {"update_parameters", (PyCFunction) cPopMaBoSSSim_update_parameters, METH_VARARGS | METH_KEYWORDS, "changes the parameters of the simulation"},
    {"set_custom_pop_output", (PyCFunction) cPopMaBoSSSim_setCustomPopOutput, METH_VARARGS, "changes the custom pop output"},
    {NULL}  /* Sentinel */
};

PyMemberDef cPopMaBoSSSim_members[] = {
    {"network", T_OBJECT_EX, offsetof(cPopMaBoSSSimObject, network), READONLY},
    {"param", T_OBJECT_EX, offsetof(cPopMaBoSSSimObject, param), READONLY},
    {NULL}  /* Sentinel */
};

PyTypeObject cPopMaBoSSSim = {
  PyVarObject_HEAD_INIT(NULL, 0)
  build_type_name("cPopMaBoSSSimObject"),               /* tp_name */
  sizeof(cPopMaBoSSSimObject),               /* tp_basicsize */
    0,                              /* tp_itemsize */
  (destructor) cPopMaBoSSSim_dealloc,      /* tp_dealloc */
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
  "cPopMaBoSS Simulation object",                   /* tp_doc */
  (traverseproc) cPopMaBoSSSim_traverse,        /* tp_traverse */
  (inquiry) cPopMaBoSSSim_clear,                /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
  cPopMaBoSSSim_methods,                              /* tp_methods */
  cPopMaBoSSSim_members,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
  cPopMaBoSSSim_init,                              /* tp_init */
    0,                              /* tp_alloc */
  cPopMaBoSSSim_new,                      /* tp_new */ 
};

// the members are typed pointers, so go through PyObject* explicitly rather
// than letting the Py_VISIT / Py_CLEAR macros cast them
int cPopMaBoSSSim_traverse(cPopMaBoSSSimObject *self, visitproc visit, void *arg)
{
  Py_VISIT((PyObject *) self->network);
  Py_VISIT((PyObject *) self->config);
  Py_VISIT((PyObject *) self->param);
  return 0;
}

int cPopMaBoSSSim_clear(cPopMaBoSSSimObject *self)
{
  PyObject* tmp;
  tmp = (PyObject *) self->network; self->network = NULL; Py_XDECREF(tmp);
  tmp = (PyObject *) self->config;  self->config  = NULL; Py_XDECREF(tmp);
  tmp = (PyObject *) self->param;   self->param   = NULL; Py_XDECREF(tmp);
  return 0;
}

void cPopMaBoSSSim_dealloc(cPopMaBoSSSimObject *self)
{
    PyObject_GC_UnTrack(self);
    cPopMaBoSSSim_clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

PyObject * cPopMaBoSSSim_new(PyTypeObject* type, PyObject *args, PyObject* kwargs)
{
  cPopMaBoSSSimObject* py_simulation = (cPopMaBoSSSimObject *) type->tp_alloc(type, 0);
  if (py_simulation == NULL) {
    return NULL;
  }
  py_simulation->network = NULL;
  py_simulation->config = NULL;
  py_simulation->param = NULL;
  return (PyObject *) py_simulation;
}


int cPopMaBoSSSim_init(PyObject* self, PyObject *args, PyObject* kwargs) 
{
  try {
    PyObject * network_file = Py_None;
    PyObject * config_file = Py_None;
    PyObject * config_files = Py_None;
    PyObject * network_str = Py_None;
    PyObject * config_str = Py_None;
    PyObject* net = Py_None;
    PyObject* cfg = Py_None;
    const char *kwargs_list[] = {"network", "config", "configs", "network_str", "config_str", "net", "cfg", NULL};
    if (!PyArg_ParseTupleAndKeywords(
      args, kwargs, "|OOOOOOO", const_cast<char **>(kwargs_list), 
      &network_file, &config_file, &config_files, &network_str, &config_str, &net, &cfg
    ))
      return -1;
      
    cPopMaBoSSSimObject* py_simulation = (cPopMaBoSSSimObject *) self;

    PyObject* new_network = NULL;
    if (net != Py_None)
    {
      // borrowed from the caller: take our own reference
      if (!PyObject_IsInstance(net, (PyObject*) &cPopMaBoSSNetwork)) {
        PyErr_SetString(PyExc_TypeError, "net must be a PopMaBoSSNet object");
        return -1;
      }
      Py_INCREF(net);
      new_network = net;

    } else {
      new_network = PyObject_CallFunction((PyObject *) &cPopMaBoSSNetwork,
        "OO", network_file, network_str
      );
    }

    if (new_network == NULL)
    {
      return -1;
    }
    Py_XSETREF(py_simulation->network, (cPopMaBoSSNetworkObject*) new_network);

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

    if (new_config == NULL)
    {
      return -1;
    }
    Py_XSETREF(py_simulation->config, (cMaBoSSConfigObject*) new_config);

    PyObject* new_param = PyObject_CallFunction((PyObject *) &cMaBoSSParam,
      "OO", (PyObject *) py_simulation->network, (PyObject *) py_simulation->config
    );

    if (new_param == NULL)
    {
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
PyObject* cPopMaBoSSSim_update_parameters(cPopMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) 
{
  return cMaBoSSParam_update_parameters(self->param, args, kwargs);
}

PyObject* cPopMaBoSSSim_run(cPopMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  
  time_t start_time, end_time;

  RandomGenerator::resetGeneratedNumberCount();
  
  // go through tp_new so the object is GC-tracked and zero-initialised
  cPopMaBoSSResultObject* res = (cPopMaBoSSResultObject*) cPopMaBoSSResult_new(&cPopMaBoSSResult, NULL, NULL);
  if (res == NULL) {
    return NULL;
  }

  // run without the GIL, so that simulations started from several Python
  // threads run in parallel
  PopMaBEstEngine* simulation = NULL;
  bool failed = false;
  std::string error;

  Py_BEGIN_ALLOW_THREADS
  try {
    simulation = new PopMaBEstEngine(self->network->network, self->config->config);
    time(&start_time);
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
    Py_DECREF(res);
    PyErr_SetString(PyBNException, error.c_str());
    return NULL;
  }

  time(&end_time);

  res->network = self->network->network;
  res->config = self->config->config;
  res->engine = simulation;
  res->start_time = start_time;
  res->end_time = end_time;
  // keep the network and the config alive for as long as the result is
  Py_INCREF(self->network);
  res->py_network = (PyObject *) self->network;
  Py_INCREF(self->config);
  res->py_config = (PyObject *) self->config;

  return (PyObject*) res;
}

PyObject* cPopMaBoSSSim_get_nodes(cPopMaBoSSSimObject* self) {

  PyObject *list = PyList_New(self->network->network->getNodes().size());

  size_t index = 0;
  for (auto* node: self->network->network->getNodes()) {
    PyList_SetItem(list, index, PyUnicode_FromString(node->getLabel().c_str()));
    index++;
  }

  return list;
}

PyObject* cPopMaBoSSSim_setCustomPopOutput(cPopMaBoSSSimObject* self, PyObject *args) {
  PyObject* custom_output;
  if (!PyArg_ParseTuple(args, "O", &custom_output))
    return NULL;
  
  std::string str_custom_output("custom_pop_output = ");
  str_custom_output.append(PyUnicode_AsUTF8(custom_output));
  str_custom_output.append(";");  
  try{
    self->config->config->parseExpression(self->network->network, str_custom_output.c_str());  
  } catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  }
  
  Py_RETURN_NONE;
}


PyObject* cPopMaBoSSSim_bnd_str(cPopMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  std::ostringstream bnd;
  self->network->network->display(bnd);
  return PyUnicode_FromString(bnd.str().c_str());
}


PyObject* cPopMaBoSSSim_cfg_str(cPopMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  std::ostringstream cfg;
  self->config->config->dump(self->network->network, cfg, PopMaBEstEngine::VERSION, false);
  return PyUnicode_FromString(cfg.str().c_str());
}

PyObject* cPopMaBoSSSim_copy(cPopMaBoSSSimObject* self) {
  std::ostringstream bnd;
  self->network->network->display(bnd);
  
  std::ostringstream cfg;
  self->config->config->dump(self->network->network, cfg, PopMaBEstEngine::VERSION, false);
  
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

  PyObject* simulation = PyObject_Call((PyObject *) &cPopMaBoSSSim, args, kwargs);

  Py_DECREF(args);
  Py_DECREF(kwargs);

  return simulation;
}
