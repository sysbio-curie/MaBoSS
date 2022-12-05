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
     maboss_net.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2020
*/

#ifndef MABOSS_NETWORK
#define MABOSS_NETWORK

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <set>
#include "src/BooleanNetwork.h"
#include "src/MaBEstEngine.h"
#include "maboss_node.cpp"

typedef struct {
  PyObject_HEAD
  Network* network;
  PyDictObject* __dict;
} cMaBoSSNetworkObject;

static void cMaBoSSNetwork_dealloc(cMaBoSSNetworkObject *self)
{
    free(self->network);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static Network* cMaBoSSNetwork_getNetwork(cMaBoSSNetworkObject* self) 
{
  return self->network;
}


static PyDictObject* cMaBoSSNetwork_getDict(cMaBoSSNetworkObject* self) 
{
  return self->__dict;
}

static PyObject* cMaBoSSNetwork_keys(cMaBoSSNetworkObject* self) 
{
  return PyDict_Keys((PyObject*) self->__dict);
}
static PyObject* cMaBoSSNetwork_values(cMaBoSSNetworkObject* self) 
{
  return PyDict_Values((PyObject*) self->__dict);
}
static PyObject* cMaBoSSNetwork_get_output(cMaBoSSNetworkObject* self) 
{
    PyObject* list = PyList_New(0);

  for (auto node : self->network->getNodes()) {
    if (node->isInternal())
      PyList_Append(list, PyUnicode_FromString(node->getLabel().c_str()));
  }
  return list;
}
static PyObject* cMaBoSSNetwork_set_output(cMaBoSSNetworkObject* self, PyObject *args, PyObject* kwargs)
{

  PyObject * list;
  static const char *kwargs_list[] = {"list", NULL};
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "O", const_cast<char **>(kwargs_list), 
    &list
  ))
    return NULL;
  for (auto node: self->network->getNodes()) {
    node->isInternal(false);
  }


  for (int i=0; i < PyList_Size(list); i++)
  {
    self->network->getNode(PyUnicode_AsUTF8(PyList_GetItem(list, i)))->isInternal(true);
  }
  return Py_None;
}


static PyObject* cMaBoSSNetwork_items(cMaBoSSNetworkObject* self) 
{
  return PyDict_Items((PyObject*) self->__dict);
}
// static PyDictObject* cMaBoSSNetwork_set_item(cMaBoSSNetworkObject* self) 
// {
//   return PyDict_Items(self->__dict);
// }
static PyObject* cMaBoSSNetwork_get_item(PyObject* self, PyObject *args) 
{
  // char * key = NULL;
  // if (!PyArg_ParseTuple(args, "s", &key))
  //   return NULL;
    

  return PyDict_GetItem((PyObject*) ((cMaBoSSNetworkObject*)self)->__dict, args);
}
static PyObject * cMaBoSSNetwork_new(PyTypeObject* type, PyObject *args, PyObject* kwargs) 
{
  char * network_file;
  static const char *kwargs_list[] = {"network", NULL};
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "s", const_cast<char **>(kwargs_list), 
    &network_file
  ))
    return NULL;
  
  cMaBoSSNetworkObject* pynetwork;
  pynetwork = (cMaBoSSNetworkObject *) type->tp_alloc(type, 0);
  pynetwork->network = new Network();
  pynetwork->network->parse(network_file);

  pynetwork->__dict = (PyDictObject*) PyDict_New();
  for (auto& node : pynetwork->network->getNodes()) {
    PyObject* pylabel = PyUnicode_FromString(node->getLabel().c_str());
    cMaBoSSNodeObject* pynode = (cMaBoSSNodeObject*) PyObject_New(cMaBoSSNodeObject, &cMaBoSSNode);
    pynode->__node = node;
    PyDict_SetItem((PyObject*) pynetwork->__dict, pylabel, (PyObject*) pynode);
  }
  return (PyObject*) pynetwork;
}


static PyMethodDef cMaBoSSNetwork_methods[] = {
    {"getNetwork", (PyCFunction) cMaBoSSNetwork_getNetwork, METH_NOARGS, "returns the network object"},
    {"getDict", (PyCFunction) cMaBoSSNetwork_getDict, METH_NOARGS, "returns the dict object"},
    {"keys", (PyCFunction) cMaBoSSNetwork_keys, METH_NOARGS, "returns the list of keys"},
    {"values", (PyCFunction) cMaBoSSNetwork_values, METH_NOARGS, "returns the list of values"},
    {"items", (PyCFunction) cMaBoSSNetwork_items, METH_NOARGS, "returns the list of items"},
    {"set_output", (PyCFunction) cMaBoSSNetwork_set_output, METH_VARARGS, "set the list of output nodes"},
    {"get_output", (PyCFunction) cMaBoSSNetwork_get_output, METH_NOARGS, "get the list of output nodes"},
    // {"__set_item__", (PyCFunction) cMaBoSSNetwork_set_item, METH_NOARGS, "sets a node"},
    {"__get_item__", (PyCFunction) cMaBoSSNetwork_get_item, METH_VARARGS, "gets a node"},
    {"__getitem__", (PyCFunction) cMaBoSSNetwork_get_item, METH_VARARGS, "gets a node"},

    {NULL}  /* Sentinel */
};


static PyMappingMethods cMaBoSSNetworkMapping = {
	(lenfunc)NULL,		// lenfunc PyMappingMethods.mp_length
	(binaryfunc)cMaBoSSNetwork_get_item,		// binaryfunc PyMappingMethods.mp_subscript
	(objobjargproc)NULL,		// objobjargproc PyMappingMethods.mp_ass_subscript
};
static PyTypeObject cMaBoSSNetwork = []{
    PyTypeObject net{PyVarObject_HEAD_INIT(NULL, 0)};




    net.tp_name = "cmaboss.cMaBoSSNetworkObject";
    net.tp_basicsize = sizeof(cMaBoSSNetworkObject);
    net.tp_itemsize = 0;
    net.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    net.tp_doc = "cMaBoSS Network object";
    net.tp_new = cMaBoSSNetwork_new;
    net.tp_dealloc = (destructor) cMaBoSSNetwork_dealloc;
    net.tp_methods = cMaBoSSNetwork_methods;
    net.tp_as_mapping = &cMaBoSSNetworkMapping;
    return net;
}();
#endif