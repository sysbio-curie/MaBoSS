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
     maboss_res.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2020
*/

#include "maboss_res.h"

#include <structmember.h>
#include <fstream>
#include "displayers/FixedPointDisplayer.h"
#include "displayers/ProbTrajDisplayer.h"
#include "displayers/StatDistDisplayer.h"

#ifdef __GLIBC__
#include <malloc.h>
#endif

// network/runconfig/engine are C++ pointers, not PyObject*, so they must never
// be exposed through T_OBJECT_EX: Python would incref a C++ object header
PyMemberDef cMaBoSSResult_members[] = {
    {(char*)"network", T_OBJECT, offsetof(cMaBoSSResultObject, py_network), READONLY, (char*)"network"},
    {(char*)"start_time", T_LONG, offsetof(cMaBoSSResultObject, start_time), READONLY, (char*)"start_time"},
    {(char*)"end_time", T_LONG, offsetof(cMaBoSSResultObject, end_time), READONLY, (char*)"end_time"},
    // T_OBJECT yields None while the cache is still empty
    {(char*)"probtraj", T_OBJECT, offsetof(cMaBoSSResultObject, probtraj), READONLY, (char*)"probtraj"},
    {(char*)"last_probtraj", T_OBJECT, offsetof(cMaBoSSResultObject, last_probtraj), READONLY, (char*)"last_probtraj"},
    {(char*)"observed_graph", T_OBJECT, offsetof(cMaBoSSResultObject, observed_graph), READONLY, (char*)"observed_graph"},
    {NULL}  /* Sentinel */
};

PyMethodDef cMaBoSSResult_methods[] = {
    {"get_observed_graph", (PyCFunction) cMaBoSSResult_get_observed_graph, METH_NOARGS, "gets the observed graph table"},
    {"get_observed_durations", (PyCFunction) cMaBoSSResult_get_observed_durations, METH_NOARGS, "gets the observed durations table"},
    {"get_fp_table", (PyCFunction) cMaBoSSResult_get_fp_table, METH_NOARGS, "gets the fixpoints table"},
    {"get_probtraj", (PyCFunction) cMaBoSSResult_get_probtraj, METH_NOARGS, "gets the raw states probability trajectories of the simulation"},
    {"get_last_probtraj", (PyCFunction) cMaBoSSResult_get_last_probtraj, METH_NOARGS, "gets the raw states probability trajectories of the simulation"},
    {"get_nodes_probtraj", (PyCFunction) cMaBoSSResult_get_nodes_probtraj, METH_VARARGS, "gets the raw states probability trajectories of the simulation"},
    {"get_last_nodes_probtraj", (PyCFunction) cMaBoSSResult_get_last_nodes_probtraj, METH_VARARGS, "gets the raw states probability trajectories of the simulation"},
    {"get_entropy_trajectory", (PyCFunction) cMaBoSSResult_get_entropy_trajectory, METH_NOARGS, "gets the entropies (TH, ErrorTH, H) trajectories of the simulation"},
    {"get_statdist", (PyCFunction) cMaBoSSResult_get_statdist, METH_NOARGS, "gets the stationary distribution of each trajectory"},
    {"get_statdist_clusters", (PyCFunction) cMaBoSSResult_get_statdist_clusters, METH_NOARGS, "gets the stationary distributions of the trajectories of each cluster"},
    {"get_statdist_clusters_summary", (PyCFunction) cMaBoSSResult_get_statdist_clusters_summary, METH_NOARGS, "gets the stationary distribution of each cluster, with its errors"},
    {"display_fp", (PyCFunction) cMaBoSSResult_display_fp, METH_VARARGS, "prints the fixpoints to a file"},
    {"display_probtraj", (PyCFunction) cMaBoSSResult_display_probtraj, METH_VARARGS, "prints the probtraj to a file"},
    {"display_statdist", (PyCFunction) cMaBoSSResult_display_statdist, METH_VARARGS, "prints the statdist to a file"},
    {"display_run", (PyCFunction) cMaBoSSResult_display_run, METH_VARARGS, "prints the run of the simulation to a file"},
    {NULL}  /* Sentinel */
};


PyTypeObject cMaBoSSResult = {
  PyVarObject_HEAD_INIT(NULL, 0)
  build_type_name("cMaBoSSResultObject"),               /* tp_name */
  sizeof(cMaBoSSResultObject),               /* tp_basicsize */
    0,                              /* tp_itemsize */
  (destructor) cMaBoSSResult_dealloc,      /* tp_dealloc */
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
  "cMaBoSS Result object",                   /* tp_doc */
  (traverseproc) cMaBoSSResult_traverse,      /* tp_traverse */
  (inquiry) cMaBoSSResult_clear,              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
  cMaBoSSResult_methods,                              /* tp_methods */
  cMaBoSSResult_members,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    0,                              /* tp_alloc */
  cMaBoSSResult_new,                      /* tp_new */    
};

int cMaBoSSResult_traverse(cMaBoSSResultObject *self, visitproc visit, void *arg)
{
  Py_VISIT(self->py_network);
  Py_VISIT(self->py_config);
  Py_VISIT(self->probtraj);
  Py_VISIT(self->last_probtraj);
  Py_VISIT(self->observed_graph);
  Py_VISIT(self->observed_durations);
  Py_VISIT(self->statdist);
  return 0;
}

int cMaBoSSResult_clear(cMaBoSSResultObject *self)
{
  Py_CLEAR(self->probtraj);
  Py_CLEAR(self->last_probtraj);
  Py_CLEAR(self->observed_graph);
  Py_CLEAR(self->observed_durations);
  Py_CLEAR(self->statdist);
  Py_CLEAR(self->py_network);
  Py_CLEAR(self->py_config);
  self->network = NULL;
  self->runconfig = NULL;
  return 0;
}

void cMaBoSSResult_dealloc(cMaBoSSResultObject *self)
{
  PyObject_GC_UnTrack(self);
  cMaBoSSResult_clear(self);
  delete self->engine;
  self->engine = NULL;

#ifdef __GLIBC__
  malloc_trim(0);
#endif

  Py_TYPE(self)->tp_free((PyObject *) self);
}

PyObject * cMaBoSSResult_new(PyTypeObject* type, PyObject *args, PyObject* kwargs)
{
  // tp_alloc zeroes the struct, so every cache starts out NULL ("not computed")
  return (PyObject*) type->tp_alloc(type, 0);
}

// Resolves a Python list of node names against the network. Returns false with a
// Python exception set on bad input; Network::getNode throws BNException for an
// unknown name, which must not be allowed to escape through the C boundary.
bool cMaBoSSResult_parse_node_list(Network* network, PyObject* pList, std::vector<Node*>& list_nodes)
{
  if (pList == NULL || pList == Py_None) {
    return true;
  }

  if (!PyList_Check(pList)) {
    PyErr_SetString(PyExc_TypeError, "Expected a list of node names");
    return false;
  }

  Py_ssize_t n = PyList_Size(pList);
  for (Py_ssize_t i = 0; i < n; i++) {
    PyObject* pItem = PyList_GetItem(pList, i);
    if (!PyUnicode_Check(pItem)) {
      PyErr_SetString(PyExc_TypeError, "Node names must be strings");
      return false;
    }
    try {
      list_nodes.push_back(network->getNode(std::string(PyUnicode_AsUTF8(pItem))));
    } catch (BNException& e) {
      PyErr_SetString(PyBNException, e.getMessage().c_str());
      return false;
    }
  }
  return true;
}

PyObject* cMaBoSSResult_get_fp_table(cMaBoSSResultObject* self) {

  PyObject *dict = PyDict_New();
  if (dict == NULL) {
    return NULL;
  }

  for (auto& result: self->engine->getFixPointsDists()) {
    // neither PyTuple_Pack nor PyDict_SetItem steals, so build with
    // Py_BuildValue("N") and release the key and the tuple afterwards
    PyObject *tuple = Py_BuildValue("NN",
      PyFloat_FromDouble(result.second.second),
      PyUnicode_FromString(result.second.first.getName(self->network).c_str())
    );
    PyObject *key = PyLong_FromUnsignedLong(result.first);
    if (tuple == NULL || key == NULL || PyDict_SetItem(dict, key, tuple) < 0) {
      Py_XDECREF(tuple);
      Py_XDECREF(key);
      Py_DECREF(dict);
      return NULL;
    }
    Py_DECREF(tuple);
    Py_DECREF(key);
  }

  return dict;
}

// the four getters below cache their (often very large) result on the object;
// NULL means "not computed yet"
PyObject* cMaBoSSResult_get_observed_graph(cMaBoSSResultObject* self) {

  if (self->observed_graph == NULL)
  {
    self->observed_graph = self->engine->getNumpyObservedGraph();
    if (self->observed_graph == NULL) {
      return NULL;
    }
  }

  Py_INCREF(self->observed_graph);

  return self->observed_graph;
}

PyObject* cMaBoSSResult_get_observed_durations(cMaBoSSResultObject* self) {

  if (self->observed_durations == NULL)
  {
    self->observed_durations = self->engine->getNumpyObservedDurations();
    if (self->observed_durations == NULL) {
      return NULL;
    }
  }

  Py_INCREF(self->observed_durations);

  return self->observed_durations;
}

PyObject* cMaBoSSResult_get_probtraj(cMaBoSSResultObject* self) {
  if (self->probtraj == NULL) {
    self->probtraj = self->engine->getMergedCumulator()->getNumpyStatesDists(self->network);
    if (self->probtraj == NULL) {
      return NULL;
    }
  }

  Py_INCREF(self->probtraj);

  return self->probtraj;
}

PyObject* cMaBoSSResult_get_last_probtraj(cMaBoSSResultObject* self) {
  if (self->last_probtraj == NULL) {
    self->last_probtraj = self->engine->getMergedCumulator()->getNumpyLastStatesDists(self->network);
    if (self->last_probtraj == NULL) {
      return NULL;
    }
  }

  Py_INCREF(self->last_probtraj);
  return self->last_probtraj;
}

PyObject* cMaBoSSResult_get_nodes_probtraj(cMaBoSSResultObject* self, PyObject* args) {

  std::vector<Node*> list_nodes;
  PyObject* pList = Py_None;
  
  if (!PyArg_ParseTuple(args, "|O", &pList)) {
    PyErr_SetString(PyExc_TypeError, "Error parsing arguments");
    return NULL;
  }
  
  if (!cMaBoSSResult_parse_node_list(self->network, pList, list_nodes)) {
    return NULL;
  }

  return self->engine->getMergedCumulator()->getNumpyNodesDists(self->network, list_nodes);
}

PyObject* cMaBoSSResult_get_last_nodes_probtraj(cMaBoSSResultObject* self, PyObject* args) {

  std::vector<Node*> list_nodes;
  PyObject* pList = Py_None;
  
  if (!PyArg_ParseTuple(args, "|O", &pList)) {
    PyErr_SetString(PyExc_TypeError, "Error parsing arguments");
    return NULL;
  }
  
  if (!cMaBoSSResult_parse_node_list(self->network, pList, list_nodes)) {
    return NULL;
  }

  return self->engine->getMergedCumulator()->getNumpyLastNodesDists(self->network, list_nodes);
}

PyObject* cMaBoSSResult_get_entropy_trajectory(cMaBoSSResultObject* self) {
  return self->engine->getMergedCumulator()->getNumpyEntropies();
}

// clustering the trajectories is costly, so the statdist getters share one cached
// (statdist, clusters, summary) tuple, computed on first use
static PyObject* cMaBoSSResult_get_statdist_item(cMaBoSSResultObject* self, Py_ssize_t item) {
  if (self->statdist == NULL) {
    NumpyStatDistDisplayer statdist_displayer(self->network);
    self->engine->displayStatDist(&statdist_displayer);
    self->statdist = statdist_displayer.getNumpyStatDist();
    if (self->statdist == NULL) {
      return NULL;
    }
  }

  PyObject* result = PyTuple_GET_ITEM(self->statdist, item);
  Py_INCREF(result);
  return result;
}

PyObject* cMaBoSSResult_get_statdist(cMaBoSSResultObject* self) {
  return cMaBoSSResult_get_statdist_item(self, 0);
}

PyObject* cMaBoSSResult_get_statdist_clusters(cMaBoSSResultObject* self) {
  return cMaBoSSResult_get_statdist_item(self, 1);
}

PyObject* cMaBoSSResult_get_statdist_clusters_summary(cMaBoSSResultObject* self) {
  return cMaBoSSResult_get_statdist_item(self, 2);
}

PyObject* cMaBoSSResult_display_fp(cMaBoSSResultObject* self, PyObject *args) 
{
  char * filename = NULL;
  int hexfloat = 0;
  if (!PyArg_ParseTuple(args, "s|i", &filename, &hexfloat))
    return NULL;
    
  std::ostream* output_fp = new std::ofstream(filename);
  CSVFixedPointDisplayer* fp_displayer = new CSVFixedPointDisplayer(self->network, *output_fp, (bool)hexfloat);
  self->engine->displayFixpoints(fp_displayer);
  delete(fp_displayer);
  ((std::ofstream*) output_fp)->close();
  delete output_fp;

  Py_RETURN_NONE;
}

PyObject* cMaBoSSResult_display_probtraj(cMaBoSSResultObject* self, PyObject *args) 
{
  char * filename = NULL;
  int hexfloat = 0;
  if (!PyArg_ParseTuple(args, "s|i", &filename, &hexfloat))
    return NULL;
    
  std::ostream* output_probtraj = new std::ofstream(filename);
  CSVProbTrajDisplayer<NetworkState>* probtraj_displayer = new CSVProbTrajDisplayer<NetworkState>(self->network, *output_probtraj, (bool)hexfloat);
  
  self->engine->displayProbTraj(probtraj_displayer);
  
  delete probtraj_displayer;
  ((std::ofstream*) output_probtraj)->close();
  delete output_probtraj;

  Py_RETURN_NONE;
}

PyObject* cMaBoSSResult_display_statdist(cMaBoSSResultObject* self, PyObject *args) 
{
  char * filename = NULL;
  int hexfloat = 0;
  if (!PyArg_ParseTuple(args, "s|i", &filename, &hexfloat))
    return NULL;
    
  std::ostream* output_statdist = new std::ofstream(filename);
  CSVStatDistDisplayer*  statdist_displayer = new CSVStatDistDisplayer(self->network, *output_statdist, (bool)hexfloat);
  self->engine->displayStatDist(statdist_displayer);
  delete statdist_displayer;
  ((std::ofstream*) output_statdist)->close();
  delete output_statdist;

  Py_RETURN_NONE;
}

PyObject* cMaBoSSResult_display_run(cMaBoSSResultObject* self, PyObject* args) 
{
  char * filename = NULL;
  int hexfloat = 0;
  if (!PyArg_ParseTuple(args, "s|i", &filename, &hexfloat))
    return NULL;
    
  std::ostream* output_run = new std::ofstream(filename);
  self->engine->displayRunStats(*output_run, self->start_time, self->end_time);
  ((std::ofstream*) output_run)->close();
  delete output_run;

  Py_RETURN_NONE;
}
