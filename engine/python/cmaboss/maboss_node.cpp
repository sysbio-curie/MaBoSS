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
     maboss_node.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     September 2022
*/

#ifndef MABOSS_NODE
#define MABOSS_NODE

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <set>
#include "src/BooleanNetwork.h"

typedef struct {
  PyObject_HEAD
  Node* __node;
} cMaBoSSNodeObject;

static void cMaBoSSNode_dealloc(cMaBoSSNodeObject *self)
{
    free(self->__node);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * cMaBoSSNode_new(PyTypeObject* type, PyObject *args, PyObject* kwargs) 
{
  char * node_label;
  static const char *kwargs_list[] = {"label", NULL};
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "s", const_cast<char **>(kwargs_list), 
    &node_label
  ))
    return NULL;
  
  cMaBoSSNodeObject* pynode;
  pynode = (cMaBoSSNodeObject *) type->tp_alloc(type, 0);
  // pynode->__node = new Network();
  // pynode->__node->parse(network_file);

  return (PyObject*) pynode;
}


static PyMethodDef cMaBoSSNode_methods[] = {
    // {"getNetwork", (PyCFunction) cMaBoSSNetwork_getNetwork, METH_NOARGS, "returns the network object"},
    // {"keys", (PyCFunction) cMaBoSSNetwork_keys, METH_NOARGS, "returns the list of keys"},
    // {"values", (PyCFunction) cMaBoSSNetwork_values, METH_NOARGS, "returns the list of values"},
    // {"items", (PyCFunction) cMaBoSSNetwork_items, METH_NOARGS, "returns the list of items"},
    // {"set_output", (PyCFunction) cMaBoSSNetwork_set_output, METH_NOARGS, "set the list of output nodes"},
    // {"get_output", (PyCFunction) cMaBoSSNetwork_get_output, METH_NOARGS, "get the list of output nodes"},
    // // {"__set_item__", (PyCFunction) cMaBoSSNetwork_set_item, METH_NOARGS, "sets a node"},
    // {"__get_item__", (PyCFunction) cMaBoSSNetwork_get_item, METH_NOARGS, "gets a node"},

    {NULL}  /* Sentinel */
};

static PyTypeObject cMaBoSSNode = []{
    PyTypeObject node{PyVarObject_HEAD_INIT(NULL, 0)};

    node.tp_name = "cmaboss.cMaBoSSNodeObject";
    node.tp_basicsize = sizeof(cMaBoSSNodeObject);
    node.tp_itemsize = 0;
    node.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    node.tp_doc = "cMaBoSS Node object";
    node.tp_new = cMaBoSSNode_new;
    node.tp_dealloc = (destructor) cMaBoSSNode_dealloc;
    node.tp_methods = cMaBoSSNode_methods;
    return node;
}();
#endif