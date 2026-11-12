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
     maboss_param.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2020
*/

#include "maboss_param.h"
#include "maboss_net.h"
#include "popmaboss_net.h"
#include "maboss_cfg.h"

using namespace MaBoSS;

PyMethodDef cMaBoSSParam_methods[] = {
  {"keys", (PyCFunction) cMaBoSSParam_getKeys, METH_NOARGS, "returns the keys"},
  {"values", (PyCFunction) cMaBoSSParam_getValues, METH_NOARGS, "returns the values"},
  {"items", (PyCFunction) cMaBoSSParam_getItems, METH_NOARGS, "returns the items"},
  {"update", (PyCFunction) cMaBoSSParam_update, METH_VARARGS | METH_KEYWORDS, "updates the parameters"},
  {NULL}  /* Sentinel */
};

PyMappingMethods cMaBoSSParam_mapping = {
	(lenfunc)cMaBoSSParam_Length,		// lenfunc PyMappingMethods.mp_length
	(binaryfunc)cMaBoSSParam_GetItem,		// binaryfunc PyMappingMethods.mp_subscript
	(objobjargproc)cMaBoSSParam_SetItem,		// objobjargproc PyMappingMethods.mp_ass_subscript
};

// only sq_contains is set: on a C type, mp_subscript alone does not make
// `key in param` work, Python would fall back to iteration
PySequenceMethods cMaBoSSParam_sequence = {
  0,                                        // sq_length
  0,                                        // sq_concat
  0,                                        // sq_repeat
  0,                                        // sq_item
  0,                                        // was_sq_slice
  0,                                        // sq_ass_item
  0,                                        // was_sq_ass_slice
  (objobjproc)cMaBoSSParam_Contains,        // sq_contains
};

PyTypeObject cMaBoSSParam = {
  PyVarObject_HEAD_INIT(NULL, 0)
  build_type_name("cMaBoSSParamObject"),               /* tp_name */
  sizeof(cMaBoSSParamObject),               /* tp_basicsize */
    0,                              /* tp_itemsize */
  (destructor) cMaBoSSParam_dealloc,      /* tp_dealloc */
    0,                              /* tp_vectorcall_offset */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_as_async */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
  &cMaBoSSParam_sequence,                              /* tp_as_sequence */
  &cMaBoSSParam_mapping,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,         /* tp_flags */
  "cMaBoSS Params object",                   /* tp_doc */
  (traverseproc) cMaBoSSParam_traverse,      /* tp_traverse */
  (inquiry) cMaBoSSParam_clear,              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
  (getiterfunc) cMaBoSSParam_Iter,          /* tp_iter */
    0,                              /* tp_iternext */
  cMaBoSSParam_methods,                              /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
  cMaBoSSParam_init,                              /* tp_init */
    0,                              /* tp_alloc */
  cMaBoSSParam_new,                      /* tp_new */
};

int cMaBoSSParam_traverse(cMaBoSSParamObject *self, visitproc visit, void *arg)
{
  Py_VISIT(self->py_network);
  Py_VISIT(self->py_config);
  return 0;
}

int cMaBoSSParam_clear(cMaBoSSParamObject *self)
{
  Py_CLEAR(self->py_network);
  Py_CLEAR(self->py_config);
  self->network = NULL;
  self->config = NULL;
  return 0;
}

void cMaBoSSParam_dealloc(PyObject *self)
{
  PyObject_GC_UnTrack(self);
  cMaBoSSParam_clear((cMaBoSSParamObject *) self);
  Py_TYPE(self)->tp_free(self);
}

PyObject* cMaBoSSParam_new(PyTypeObject* type, PyObject *args, PyObject* kwargs)
{
  // tp_alloc zeroes the struct
  return (PyObject*) type->tp_alloc(type, 0);
}

int cMaBoSSParam_init(PyObject* self, PyObject *args, PyObject* kwargs) 
{
  PyObject * py_network = Py_None;
  PyObject * py_config = Py_None;
  
  const char *kwargs_list[] = {"network", "config", NULL};
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "OO", const_cast<char **>(kwargs_list), 
    &py_network, &py_config
  ))
    return -1;
  
  cMaBoSSParamObject* py_param = (cMaBoSSParamObject *) self;

  if (PyObject_IsInstance(py_network, (PyObject*)&cMaBoSSNetwork))
  {
    py_param->network = ((cMaBoSSNetworkObject*) py_network)->network;

  } else if (PyObject_IsInstance(py_network, (PyObject*)&cPopMaBoSSNetwork))
  {
    py_param->network = ((cPopMaBoSSNetworkObject*) py_network)->network;

  } else {
    PyErr_SetString(PyBNException, "Invalid network object");
    return -1;
  }

  if (!PyObject_IsInstance(py_config, (PyObject*)&cMaBoSSConfig)) {
    py_param->network = NULL;
    PyErr_SetString(PyBNException, "Invalid config object");
    return -1;
  }
  py_param->config = ((cMaBoSSConfigObject *) py_config)->config;

  // keep both wrappers alive for as long as this param view is
  Py_INCREF(py_network);
  Py_XSETREF(py_param->py_network, py_network);
  Py_INCREF(py_config);
  Py_XSETREF(py_param->py_config, py_config);

  return 0;
}

PyObject* cMaBoSSParam_update_parameters(cMaBoSSParamObject* self, PyObject *args, PyObject* kwargs) 
{
  // METH_KEYWORDS passes NULL when the call carried no keyword arguments
  if (kwargs == NULL || kwargs == Py_None) {
    Py_RETURN_NONE;
  }
  if (!PyDict_Check(kwargs)) {
    PyErr_SetString(PyExc_TypeError, "parameters must be given as keyword arguments");
    return NULL;
  }

  PyObject* key, *value;
  Py_ssize_t pos = 0;
  while (PyDict_Next(kwargs, &pos, &key, &value))
  {
    if (!PyUnicode_Check(key)) {
      PyErr_SetString(PyExc_TypeError, "Parameter names must be strings");
      return NULL;
    }
    if (PyUnicode_CompareWithASCIIString(key, "time_tick") == 0) {
      self->config->setParameter("time_tick", PyFloat_AsDouble(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "max_time") == 0) {
      self->config->setParameter("max_time", PyFloat_AsDouble(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "sample_count") == 0) {
      self->config->setParameter("sample_count", PyLong_AsLong(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "init_pop") == 0) {
      self->config->setParameter("init_pop", PyLong_AsLong(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "discrete_time") == 0) {
      self->config->setParameter("discrete_time", PyLong_AsLong(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "use_physrandgen") == 0) {
      self->config->setParameter("use_physrandgen", PyLong_AsLong(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "use_mtrandgen") == 0) {
      self->config->setParameter("use_mtrandgen", PyLong_AsLong(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "use_glibcrandgen") == 0) {
      self->config->setParameter("use_glibcrandgen", PyLong_AsLong(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "seed_pseudorandom") == 0) {
      self->config->setParameter("seed_pseudorandom", PyFloat_AsDouble(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "thread_count") == 0) {
      self->config->setParameter("thread_count", PyLong_AsLong(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "display_traj") == 0) {
      self->config->setParameter("display_traj", PyLong_AsLong(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "statdist_traj_count") == 0) {
      self->config->setParameter("statdist_traj_count", PyLong_AsLong(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "statdist_cluster_threshold") == 0) {
      self->config->setParameter("statdist_cluster_threshold", PyFloat_AsDouble(value));
    } else if (PyUnicode_CompareWithASCIIString(key, "statdist_similarity_cache_max_size") == 0) {
      self->config->setParameter("statdist_similarity_cache_max_size", PyLong_AsLong(value));
    } else {
      const char * key_str = PyUnicode_AsUTF8(key);
      if (key_str != NULL && key_str[0] == '$') {
        try {
          SymbolTable* st = self->network->getSymbolTable();
          st->setSymbolValue(st->getOrMakeSymbol(key_str), PyFloat_AsDouble(value));
        } catch (BNException& e) {
          PyErr_SetString(PyBNException, e.getMessage().c_str());
          return NULL;
        }
      } else {
        PyErr_SetString(PyExc_KeyError, "Unknown parameter");
        return NULL;
      }
    }
    if (PyErr_Occurred()) {
      return NULL;
    }
  }

  Py_RETURN_NONE;
}

PyObject* cMaBoSSParam_update(cMaBoSSParamObject* self, PyObject *args, PyObject* kwargs)
{
  PyObject * params = PyDict_New();
  if (params == NULL) {
    return NULL;
  }

  if (args != NULL && args != Py_None && PyTuple_Size(args) > 0 && PyDict_Check(PyTuple_GetItem(args, 0)))
  {
    if (PyDict_Update(params, PyTuple_GetItem(args, 0)) < 0) {
      Py_DECREF(params);
      return NULL;
    }
  }

  if (kwargs != NULL && kwargs != Py_None && PyDict_Size(kwargs) > 0) {
    PyObject* key, *value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(kwargs, &pos, &key, &value))
    {
      if (PyDict_SetItem(params, key, value) < 0) {
        Py_DECREF(params);
        return NULL;
      }
    }
  }

  PyObject* result = cMaBoSSParam_update_parameters(self, Py_None, params);
  Py_DECREF(params);
  return result;
}

int cMaBoSSParam_SetItem(cMaBoSSParamObject* self, PyObject *key, PyObject* value) 
{
  if (value == NULL) {
    PyErr_SetString(PyExc_TypeError, "Parameters cannot be deleted");
    return -1;
  }
  if (!PyUnicode_Check(key)) {
    PyErr_SetString(PyExc_TypeError, "Parameter names must be strings");
    return -1;
  }

  PyObject* params = Py_BuildValue("{s:O}", PyUnicode_AsUTF8(key), value);
  if (params == NULL) {
    return -1;
  }

  PyObject* result = cMaBoSSParam_update_parameters(self, Py_None, params);
  Py_DECREF(params);

  if (result == NULL) {
    return -1;   // propagate the failure instead of silently reporting success
  }
  Py_DECREF(result);
  return 0;
}

PyObject * cMaBoSSParam_GetItem(cMaBoSSParamObject* self, PyObject *key)
{
  if (!PyUnicode_Check(key)) {
    PyErr_SetString(PyExc_TypeError, "Parameter names must be strings");
    return NULL;
  }

  if (PyUnicode_CompareWithASCIIString(key, "time_tick") == 0) {
    return PyFloat_FromDouble(self->config->getTimeTick());
    
  } else if (PyUnicode_CompareWithASCIIString(key, "max_time") == 0) {
    return PyFloat_FromDouble(self->config->getMaxTime());
    
  } else if (PyUnicode_CompareWithASCIIString(key, "sample_count") == 0) {
    return PyLong_FromUnsignedLong(self->config->getSampleCount());
    
  } else if (PyUnicode_CompareWithASCIIString(key, "discrete_time") == 0) {
    PyObject* discrete_time = self->config->isDiscreteTime() ? Py_True : Py_False;
    Py_INCREF(discrete_time);
    return discrete_time;
  
  } else if (PyUnicode_CompareWithASCIIString(key, "use_physrandgen") == 0) {
    PyObject* use_physrandgen = self->config->usePhysRandGen() ? Py_True : Py_False;
    Py_INCREF(use_physrandgen);
    return use_physrandgen;
  
  } else if (PyUnicode_CompareWithASCIIString(key, "use_mtrandgen") == 0) {
    PyObject* use_mtrandgen = self->config->useMTRandGen() ? Py_True : Py_False;
    Py_INCREF(use_mtrandgen);
    return use_mtrandgen;
  
  } else if (PyUnicode_CompareWithASCIIString(key, "use_glibcrandgen") == 0) {
    PyObject* use_glibcrandgen = self->config->useGlibcRandGen() ? Py_True : Py_False;
    Py_INCREF(use_glibcrandgen);
    return use_glibcrandgen;
  
  } else if (PyUnicode_CompareWithASCIIString(key, "seed_pseudorandom") == 0) {
    return PyLong_FromLong(self->config->getSeedPseudoRandom());
  
  } else if (PyUnicode_CompareWithASCIIString(key, "thread_count") == 0) {
    return PyLong_FromUnsignedLong(self->config->getThreadCount());
  
  } else if (PyUnicode_CompareWithASCIIString(key, "display_traj") == 0) {
    return PyLong_FromUnsignedLong(self->config->getDisplayTrajectories());
    
  } else if (PyUnicode_CompareWithASCIIString(key, "statdist_traj_count") == 0) {
    return PyLong_FromUnsignedLong(self->config->getStatDistTrajCount());
  
  } else if (PyUnicode_CompareWithASCIIString(key, "statdist_cluster_threshold") == 0) {
    return PyFloat_FromDouble(self->config->getStatdistClusterThreshold());
    
  } else if (PyUnicode_CompareWithASCIIString(key, "statdist_similarity_cache_max_size") == 0) {
    return PyLong_FromUnsignedLong(self->config->getStatDistSimilarityCacheMaxSize());
    
  } else if (PyUnicode_CompareWithASCIIString(key, "init_pop") == 0) {
    return PyLong_FromUnsignedLong(self->config->getInitPop());
    
  } else {
    
    const char * key_str = PyUnicode_AsUTF8(key);
    if (key_str != NULL && key_str[0] == '$') {
      // getSymbol returns NULL for an unknown symbol, getSymbolValue throws
      // BNException for a known but undefined one
      try {
        SymbolTable* st = self->network->getSymbolTable();
        const Symbol* symbol = st->getSymbol(key_str);
        if (symbol == NULL) {
          PyErr_SetObject(PyExc_KeyError, key);
          return NULL;
        }
        return PyFloat_FromDouble(st->getSymbolValue(symbol));
      } catch (BNException& e) {
        PyErr_SetString(PyExc_KeyError, e.getMessage().c_str());
        return NULL;
      }
    } else {
      PyErr_SetString(PyExc_KeyError, "Unknown parameter");
      return NULL;
    }
  }
  
  return NULL;
}

int cMaBoSSParam_Contains(cMaBoSSParamObject* self, PyObject *key)
{
  // like a dict, a key of the wrong type is simply not there
  if (!PyUnicode_Check(key)) {
    return 0;
  }
  PyObject* value = cMaBoSSParam_GetItem(self, key);
  if (value != NULL) {
    Py_DECREF(value);
    return 1;
  }
  if (PyErr_ExceptionMatches(PyExc_KeyError)) {
    PyErr_Clear();
    return 0;
  }
  return -1;
}

PyObject* cMaBoSSParam_Iter(cMaBoSSParamObject* self)
{
  // iterating a mapping yields its keys
  PyObject* keys = cMaBoSSParam_getKeys(self);
  if (keys == NULL) {
    return NULL;
  }
  PyObject* iter = PyObject_GetIter(keys);
  Py_DECREF(keys);
  return iter;
}

// number of run parameters listed by keys(), values() and items() before the
// network symbols; keep it in sync with the slots filled in those functions
static const Py_ssize_t N_BUILTIN_PARAMS = 14;

Py_ssize_t cMaBoSSParam_Length(cMaBoSSParamObject* self)
{
  return N_BUILTIN_PARAMS + self->network->getSymbolTable()->getSymbolsNames().size();
}

PyObject* cMaBoSSParam_getKeys(cMaBoSSParamObject* self)
{
  SymbolTable* st = self->network->getSymbolTable();
  PyObject* keys = PyList_New(N_BUILTIN_PARAMS + st->getSymbolsNames().size());
  PyList_SetItem(keys, 0, PyUnicode_FromString("time_tick"));
  PyList_SetItem(keys, 1, PyUnicode_FromString("max_time"));
  PyList_SetItem(keys, 2, PyUnicode_FromString("sample_count"));
  PyList_SetItem(keys, 3, PyUnicode_FromString("init_pop"));
  PyList_SetItem(keys, 4, PyUnicode_FromString("discrete_time"));
  PyList_SetItem(keys, 5, PyUnicode_FromString("use_physrandgen"));
  PyList_SetItem(keys, 6, PyUnicode_FromString("use_glibcrandgen"));
  PyList_SetItem(keys, 7, PyUnicode_FromString("use_mtrandgen"));
  PyList_SetItem(keys, 8, PyUnicode_FromString("seed_pseudorandom"));
  PyList_SetItem(keys, 9, PyUnicode_FromString("display_traj"));
  PyList_SetItem(keys, 10, PyUnicode_FromString("statdist_traj_count"));
  PyList_SetItem(keys, 11, PyUnicode_FromString("statdist_cluster_threshold"));
  PyList_SetItem(keys, 12, PyUnicode_FromString("thread_count"));
  PyList_SetItem(keys, 13, PyUnicode_FromString("statdist_similarity_cache_max_size"));
  int i = 0;
  for (auto const& item_name : st->getSymbolsNames()) {
    PyList_SetItem(keys, N_BUILTIN_PARAMS + i, PyUnicode_FromString(item_name.c_str()));
    i++;
  }
  return keys;
}

PyObject* cMaBoSSParam_getValues(cMaBoSSParamObject* self)
{
  SymbolTable* st = self->network->getSymbolTable();
  PyObject* values = PyList_New(N_BUILTIN_PARAMS + st->getSymbolsNames().size());
  PyList_SetItem(values, 0, PyFloat_FromDouble(self->config->getTimeTick()));
  PyList_SetItem(values, 1, PyFloat_FromDouble(self->config->getMaxTime()));
  PyList_SetItem(values, 2, PyLong_FromUnsignedLong(self->config->getSampleCount()));
  PyList_SetItem(values, 3, PyLong_FromUnsignedLong(self->config->getInitPop()));
  PyList_SetItem(values, 4, Py_NewRef(self->config->isDiscreteTime() ? Py_True : Py_False));
  PyList_SetItem(values, 5, Py_NewRef(self->config->usePhysRandGen() ? Py_True : Py_False));
  PyList_SetItem(values, 6, Py_NewRef(self->config->useGlibcRandGen() ? Py_True : Py_False));
  PyList_SetItem(values, 7, Py_NewRef(self->config->useMTRandGen() ? Py_True : Py_False));
  PyList_SetItem(values, 8, PyLong_FromLong(self->config->getSeedPseudoRandom()));
  PyList_SetItem(values, 9, PyLong_FromUnsignedLong(self->config->getDisplayTrajectories()));
  PyList_SetItem(values, 10, PyLong_FromUnsignedLong(self->config->getStatDistTrajCount()));
  PyList_SetItem(values, 11, PyFloat_FromDouble(self->config->getStatdistClusterThreshold()));
  PyList_SetItem(values, 12, PyLong_FromUnsignedLong(self->config->getThreadCount()));
  PyList_SetItem(values, 13, PyLong_FromUnsignedLong(self->config->getStatDistSimilarityCacheMaxSize()));
  int i = 0;
  for (auto const& item_name : st->getSymbolsNames()) {
    PyList_SetItem(values, N_BUILTIN_PARAMS + i, PyFloat_FromDouble(st->getSymbolValue(st->getSymbol(item_name))));
    i++;
  }
  return values;
}

PyObject* cMaBoSSParam_getItems(cMaBoSSParamObject* self)
{
  SymbolTable* st = self->network->getSymbolTable();
  PyObject* items = PyList_New(N_BUILTIN_PARAMS + st->getSymbolsNames().size());
  PyList_SetItem(items, 0, Py_BuildValue("(sN)", "time_tick", PyFloat_FromDouble(self->config->getTimeTick())));
  PyList_SetItem(items, 1, Py_BuildValue("(sN)", "max_time", PyFloat_FromDouble(self->config->getMaxTime())));
  PyList_SetItem(items, 2, Py_BuildValue("(sN)", "sample_count", PyLong_FromUnsignedLong(self->config->getSampleCount())));
  PyList_SetItem(items, 3, Py_BuildValue("(sN)", "init_pop", PyLong_FromUnsignedLong(self->config->getInitPop())));
  PyList_SetItem(items, 4, Py_BuildValue("(sO)", "discrete_time", self->config->isDiscreteTime() ? Py_True : Py_False));
  PyList_SetItem(items, 5, Py_BuildValue("(sO)", "use_physrandgen", self->config->usePhysRandGen() ? Py_True : Py_False));
  PyList_SetItem(items, 6, Py_BuildValue("(sO)", "use_glibcrandgen", self->config->useGlibcRandGen() ? Py_True : Py_False));
  PyList_SetItem(items, 7, Py_BuildValue("(sO)", "use_mtrandgen", self->config->useMTRandGen() ? Py_True : Py_False));
  PyList_SetItem(items, 8, Py_BuildValue("(sN)", "seed_pseudorandom", PyLong_FromLong(self->config->getSeedPseudoRandom())));
  PyList_SetItem(items, 9, Py_BuildValue("(sN)", "display_traj", PyLong_FromUnsignedLong(self->config->getDisplayTrajectories())));
  PyList_SetItem(items, 10, Py_BuildValue("(sN)", "statdist_traj_count", PyLong_FromUnsignedLong(self->config->getStatDistTrajCount())));
  PyList_SetItem(items, 11, Py_BuildValue("(sN)", "statdist_cluster_threshold", PyFloat_FromDouble(self-> config->getStatdistClusterThreshold())));
  PyList_SetItem(items, 12, Py_BuildValue("(sN)", "thread_count", PyLong_FromUnsignedLong(self->config->getThreadCount())));
  PyList_SetItem(items, 13, Py_BuildValue("(sN)", "statdist_similarity_cache_max_size", PyLong_FromUnsignedLong(self->config->getStatDistSimilarityCacheMaxSize())));
  int i = 0;
  for (auto const& item_name : st->getSymbolsNames()) {
    // "N" steals both temporaries into the tuple; PyTuple_Pack would incref
    // them instead and leak the caller's references
    PyList_SetItem(items, N_BUILTIN_PARAMS + i, Py_BuildValue("(NN)",
      PyUnicode_FromString(item_name.c_str()),
      PyFloat_FromDouble(st->getSymbolValue(st->getSymbol(item_name)))
    ));
    i++;
  }
  return items;
}
