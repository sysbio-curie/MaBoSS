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
     popmaboss_res.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     March 2021
*/


#include "popmaboss_res.h"

#include "displayers/PopProbTrajDisplayer.h"

#include <fstream>
#ifdef __GLIBC__
#include <malloc.h>
#endif

PyMethodDef cPopMaBoSSResult_methods[] = {
    {"get_fp_table", (PyCFunction) cPopMaBoSSResult_get_fp_table, METH_NOARGS, "gets the fixpoints table"},
    {"get_probtraj", (PyCFunction) cPopMaBoSSResult_get_probtraj, METH_NOARGS, "gets the raw states probability trajectories of the simulation"},
    {"get_last_probtraj", (PyCFunction) cPopMaBoSSResult_get_last_probtraj, METH_NOARGS, "gets the last raw states probability of the simulation"},
    {"get_simple_probtraj", (PyCFunction) cPopMaBoSSResult_get_simple_probtraj, METH_NOARGS, "gets the raw simple states probability trajectories of the simulation"},
    {"get_simple_last_probtraj", (PyCFunction) cPopMaBoSSResult_get_simple_last_probtraj, METH_NOARGS, "gets the last raw simple states probability of the simulation"},
    {"get_custom_probtraj", (PyCFunction) cPopMaBoSSResult_get_custom_probtraj, METH_NOARGS, "gets the raw custom states probability trajectories of the simulation"},
    {"get_custom_last_probtraj", (PyCFunction) cPopMaBoSSResult_get_custom_last_probtraj, METH_NOARGS, "gets the last raw custom states probability of the simulation"},
    {"get_entropy_trajectory", (PyCFunction) cPopMaBoSSResult_get_entropy_trajectory, METH_NOARGS, "gets the entropies (TH, ErrorTH, H) trajectories of the simulation"},
    {"display_fp", (PyCFunction) cPopMaBoSSResult_display_fp, METH_VARARGS, "prints the fixpoints to a file"},
    {"display_probtraj", (PyCFunction) cPopMaBoSSResult_display_probtraj, METH_VARARGS, "prints the probtraj to a file"},
    // {"display_statdist", (PyCFunction) cMaBoSSResult_display_statdist, METH_VARARGS, "prints the statdist to a file"},
    {"display_run", (PyCFunction) cPopMaBoSSResult_display_run, METH_VARARGS, "prints the run of the simulation to a file"},
    {NULL}  /* Sentinel */
};

PyTypeObject cPopMaBoSSResult = {
  PyVarObject_HEAD_INIT(NULL, 0)
  build_type_name("cPopMaBoSSResultObject"),               /* tp_name */
  sizeof(cPopMaBoSSResultObject),               /* tp_basicsize */
    0,                              /* tp_itemsize */
  (destructor) cPopMaBoSSResult_dealloc,      /* tp_dealloc */
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
  "cPopMaBoSS Result object",                   /* tp_doc */
  (traverseproc) cPopMaBoSSResult_traverse,     /* tp_traverse */
  (inquiry) cPopMaBoSSResult_clear,             /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
  cPopMaBoSSResult_methods,                              /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    0,                              /* tp_alloc */
  cPopMaBoSSResult_new,                      /* tp_new */   
};

int cPopMaBoSSResult_traverse(cPopMaBoSSResultObject *self, visitproc visit, void *arg)
{
  Py_VISIT(self->py_network);
  Py_VISIT(self->py_config);
  return 0;
}

int cPopMaBoSSResult_clear(cPopMaBoSSResultObject *self)
{
  Py_CLEAR(self->py_network);
  Py_CLEAR(self->py_config);
  self->network = NULL;
  self->config = NULL;
  return 0;
}

void cPopMaBoSSResult_dealloc(cPopMaBoSSResultObject *self)
{
  PyObject_GC_UnTrack(self);
  cPopMaBoSSResult_clear(self);
  delete self->engine;
  self->engine = NULL;

#ifdef __GLIBC__
  malloc_trim(0);
#endif

  Py_TYPE(self)->tp_free((PyObject *) self);
}

PyObject * cPopMaBoSSResult_new(PyTypeObject* type, PyObject *args, PyObject* kwargs)
{
  // tp_alloc zeroes the struct
  return (PyObject*) type->tp_alloc(type, 0);
}

PyObject* cPopMaBoSSResult_get_fp_table(cPopMaBoSSResultObject* self) {

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

PyObject* cPopMaBoSSResult_get_probtraj(cPopMaBoSSResultObject* self) {
  return self->engine->getMergedCumulator()->getNumpyStatesDists(self->network);
}

PyObject* cPopMaBoSSResult_get_last_probtraj(cPopMaBoSSResultObject* self) {
  return self->engine->getMergedCumulator()->getNumpyLastStatesDists(self->network);
}

PyObject* cPopMaBoSSResult_get_simple_probtraj(cPopMaBoSSResultObject* self) {
  return self->engine->getMergedCumulator()->getNumpySimpleStatesDists(self->network);
}

PyObject* cPopMaBoSSResult_get_simple_last_probtraj(cPopMaBoSSResultObject* self) {
  return self->engine->getMergedCumulator()->getNumpySimpleLastStatesDists(self->network);
}

PyObject* cPopMaBoSSResult_get_custom_probtraj(cPopMaBoSSResultObject* self) {
  if (self->config->hasCustomPopOutput()){
    return self->engine->getCustomPopCumulator()->getNumpyStatesDists(self->network);
  } else 
  Py_RETURN_NONE;
}

PyObject* cPopMaBoSSResult_get_custom_last_probtraj(cPopMaBoSSResultObject* self) {
  if (self->config->hasCustomPopOutput())
    return self->engine->getCustomPopCumulator()->getNumpyLastStatesDists(self->network);
  else 
    Py_RETURN_NONE;
}

PyObject* cPopMaBoSSResult_get_entropy_trajectory(cPopMaBoSSResultObject* self) {
  return self->engine->getMergedCumulator()->getNumpyEntropies();
}

PyObject* cPopMaBoSSResult_display_fp(cPopMaBoSSResultObject* self, PyObject *args) 
{
  char * filename = NULL;
  int hexfloat = 0;
  if (!PyArg_ParseTuple(args, "s|i", &filename, &hexfloat))
    return NULL;
    
  std::ostream* output_fp = new std::ofstream(filename);
  CSVFixedPointDisplayer * fp_displayer = new CSVFixedPointDisplayer(self->network, *output_fp, hexfloat);

  self->engine->displayFixpoints(fp_displayer);
  ((std::ofstream*) output_fp)->close();
  
  delete fp_displayer;
  delete output_fp;

  Py_RETURN_NONE;
}

PyObject* cPopMaBoSSResult_display_probtraj(cPopMaBoSSResultObject* self, PyObject *args) 
{
  char * filename = NULL;
  char * simple_filename = NULL;
  int hexfloat = 0;
  if (!PyArg_ParseTuple(args, "ss|i", &filename, &simple_filename, &hexfloat))
    return NULL;
    
  std::ostream* output_probtraj = new std::ofstream(filename);
  std::ostream* output_simple_probtraj = new std::ofstream(simple_filename);
  
  CSVSimplePopProbTrajDisplayer * pop_probtraj_displayer = new CSVSimplePopProbTrajDisplayer(self->network, *output_probtraj, *output_simple_probtraj, hexfloat);
  self->engine->displayPopProbTraj(pop_probtraj_displayer);
  
  ((std::ofstream*) output_probtraj)->close();
  ((std::ofstream*) output_simple_probtraj)->close();
  
  delete pop_probtraj_displayer;
  delete output_probtraj;
  delete output_simple_probtraj;
  
  Py_RETURN_NONE;
}

// PyObject* cPopMaBoSSResult_display_statdist(cPopMaBoSSResultObject* self, PyObject *args) 
// {
//   char * filename = NULL;
//   int hexfloat = 0;
//   if (!PyArg_ParseTuple(args, "s|i", &filename, &hexfloat))
//     return NULL;
    
//   std::ostream* output_statdist = new std::ofstream(filename);
//   self->engine->displayStatDist(*output_statdist, (bool) hexfloat);
//   ((std::ofstream*) output_statdist)->close();
//   delete output_statdist;

//   Py_RETURN_NONE;
// }

PyObject* cPopMaBoSSResult_display_run(cPopMaBoSSResultObject* self, PyObject* args) 
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
