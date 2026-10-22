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

#include "maboss_net.h"
#include "IStates.h"
#include <set>

PyMethodDef cMaBoSSNetwork_methods[] = {
    {"set_output", (PyCFunction) cMaBoSSNetwork_setOutput, METH_VARARGS, "sets the output nodes"},
    {"get_output", (PyCFunction) cMaBoSSNetwork_getOutput, METH_NOARGS, "gets the output nodes"},
    {"set_observed_graph_nodes", (PyCFunction) cMaBoSSNetwork_setObservedGraphNode, METH_VARARGS, "sets the observed graph nodes"},
    {"get_observed_graph_nodes", (PyCFunction) cMaBoSSNetwork_getObservedGraphNode, METH_VARARGS, "gets the observed graph nodes"},
    {"add_node", (PyCFunction) cMaBoSSNetwork_addNode, METH_VARARGS, "adds a node to the network"},
    {"set_istate", (PyCFunction) cMaBoSSNetwork_setIState, METH_VARARGS, "sets the initial state of the network"},
    {"get_istate", (PyCFunction) cMaBoSSNetwork_getIState, METH_NOARGS, "gets the initial state of the network"},
    {"keys", (PyCFunction) cMaBoSSNetwork_Keys, METH_NOARGS, "returns the keys of the nodes"},
    {"values", (PyCFunction) cMaBoSSNetwork_Values, METH_NOARGS, "returns the values of the nodes"},
    {"items", (PyCFunction) cMaBoSSNetwork_Items, METH_NOARGS, "returns the items of the nodes"},
    {NULL}  /* Sentinel */
};

PyMappingMethods cMaBoSSNetwork_mapping = {
	(lenfunc)cMaBoSSNetwork_NodesLength,		
	(binaryfunc)cMaBoSSNetwork_NodesGetItem,
	(objobjargproc)cMaBoSSNetwork_NodesSetItem,
};

PyTypeObject cMaBoSSNetwork = {
  PyVarObject_HEAD_INIT(NULL, 0)
  build_type_name("cMaBoSSNetworkObject"),               /* tp_name */
  sizeof(cMaBoSSNetworkObject),               /* tp_basicsize */
    0,                              /* tp_itemsize */
  (destructor) cMaBoSSNetwork_dealloc,      /* tp_dealloc */
    0,                              /* tp_vectorcall_offset */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_as_async */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
  &cMaBoSSNetwork_mapping,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
  cMaBoSSNetwork_str,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,         /* tp_flags */
  "cMaBoSS Network object",                   /* tp_doc */
  (traverseproc) cMaBoSSNetwork_traverse,     /* tp_traverse */
  (inquiry) cMaBoSSNetwork_clear,             /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
  cMaBoSSNetwork_methods,                              /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
  cMaBoSSNetwork_init,                              /* tp_init */
    0,                              /* tp_alloc */
  cMaBoSSNetwork_new,                      /* tp_new */
};

int cMaBoSSNetwork_traverse(cMaBoSSNetworkObject *self, visitproc visit, void *arg)
{
  Py_VISIT(self->nodes);
  return 0;
}

int cMaBoSSNetwork_clear(cMaBoSSNetworkObject *self)
{
  Py_CLEAR(self->nodes);
  return 0;
}

void cMaBoSSNetwork_dealloc(cMaBoSSNetworkObject *self)
{
    PyObject_GC_UnTrack(self);
    Py_CLEAR(self->nodes);
    delete self->network;
    self->network = NULL;
    Py_TYPE(self)->tp_free((PyObject *) self);
}

PyObject *cMaBoSSNetwork_str(PyObject *self) {
  return PyUnicode_FromString(((cMaBoSSNetworkObject* )self)->network->toString().c_str());
}

int cMaBoSSNetwork_NodesSetItem(cMaBoSSNetworkObject* self, PyObject *key, PyObject* value)
{
  // value is NULL when Python is deleting the entry (del net[key])
  if (value == NULL) {
    return PyDict_DelItem(self->nodes, key);
  }
  return PyDict_SetItem(self->nodes, key, value);
}

PyObject * cMaBoSSNetwork_NodesGetItem(cMaBoSSNetworkObject* self, PyObject *key)
{
  // PyDict_GetItem returns a borrowed reference, and NULL without setting an
  // exception when the key is missing
  PyObject* item = PyDict_GetItemWithError(self->nodes, key);
  if (item == NULL) {
    if (!PyErr_Occurred()) {
      PyErr_SetObject(PyExc_KeyError, key);
    }
    return NULL;
  }
  Py_INCREF(item);
  return item;
}

Py_ssize_t cMaBoSSNetwork_NodesLength(cMaBoSSNetworkObject* self)
{
  return PyObject_Length(self->nodes);
}

PyObject * cMaBoSSNetwork_Keys(cMaBoSSNetworkObject* self)
{
  return PyDict_Keys(self->nodes);
}

PyObject * cMaBoSSNetwork_Values(cMaBoSSNetworkObject* self)
{
  return PyDict_Values(self->nodes);
}

PyObject * cMaBoSSNetwork_Items(cMaBoSSNetworkObject* self)
{
  return PyDict_Items(self->nodes);
}

// Marks the nodes named in labels (any iterable of strings) as outputs, and all the others
// as internal. Nothing changes if a label is not a string or not a node of the network.
PyObject* cMaBoSSNetwork_setOutputNodes(Network* network, PyObject* labels)
{
  if (PyUnicode_Check(labels)) {
    PyErr_SetString(PyExc_TypeError, "Expected a list of node names");
    return NULL;
  }
  PyObject* iterator = PyObject_GetIter(labels);
  if (iterator == NULL) {
    PyErr_SetString(PyExc_TypeError, "Expected a list of node names");
    return NULL;
  }

  std::set<Node*> output_nodes;
  PyObject* item;
  while ((item = PyIter_Next(iterator)) != NULL) {
    if (!PyUnicode_Check(item)) {
      Py_DECREF(item);
      Py_DECREF(iterator);
      PyErr_SetString(PyExc_TypeError, "Node names must be strings");
      return NULL;
    }
    try {
      output_nodes.insert(network->getNode(std::string(PyUnicode_AsUTF8(item))));
    } catch (BNException& e) {
      Py_DECREF(item);
      Py_DECREF(iterator);
      PyErr_SetString(PyBNException, e.getMessage().c_str());
      return NULL;
    }
    Py_DECREF(item);
  }
  Py_DECREF(iterator);
  if (PyErr_Occurred()) {
    return NULL;
  }

  for (auto* node: network->getNodes()) {
    node->isInternal(output_nodes.find(node) == output_nodes.end());
  }
  Py_RETURN_NONE;
}

PyObject* cMaBoSSNetwork_setOutput(cMaBoSSNetworkObject* self, PyObject *args) 
{
  PyObject* list;
  if (!PyArg_ParseTuple(args, "O", &list))
    return NULL;

  return cMaBoSSNetwork_setOutputNodes(self->network, list);
}

PyObject* cMaBoSSNetwork_getOutput(cMaBoSSNetworkObject* self)
{
  PyObject* output = PyList_New(0);
  if (output == NULL) {
    return NULL;
  }
  for (auto* node: self->network->getNodes())
  {
    if (!node->isInternal()) {
      PyObject* label = PyUnicode_FromString(node->getLabel().c_str());
      if (label == NULL || PyList_Append(output, label) < 0) {
        Py_XDECREF(label);
        Py_DECREF(output);
        return NULL;
      }
      Py_DECREF(label);
    }
  }
  return output;
}

PyObject* cMaBoSSNetwork_setObservedGraphNode(cMaBoSSNetworkObject* self, PyObject *args) 
{
  PyObject* list;
  if (!PyArg_ParseTuple(args, "O", &list))
    return NULL;
  
  for (auto* node: self->network->getNodes())
  {
    PyObject* label = PyUnicode_FromString(node->getLabel().c_str());
    if (label == NULL) {
      return NULL;
    }
    int contains = PySequence_Contains(list, label);
    Py_DECREF(label);
    if (contains < 0) {
      return NULL;
    }
    node->inGraph(contains == 1);
  }
  Py_RETURN_NONE;
}

PyObject* cMaBoSSNetwork_getObservedGraphNode(cMaBoSSNetworkObject* self, PyObject *args)
{
  PyObject* output = PyList_New(0);
  if (output == NULL) {
    return NULL;
  }
  for (auto* node: self->network->getNodes())
  {
    if (node->inGraph()) {
      PyObject* label = PyUnicode_FromString(node->getLabel().c_str());
      if (label == NULL || PyList_Append(output, label) < 0) {
        Py_XDECREF(label);
        Py_DECREF(output);
        return NULL;
      }
      Py_DECREF(label);
    }
  }
  return output;
}

PyObject* cMaBoSSNetwork_addNode(cMaBoSSNetworkObject* self, PyObject *args) 
{
  char * name;
  if (!PyArg_ParseTuple(args, "s", &name))
    return NULL;
  
  try{
    // the node takes a reference on this network, so pass the Python object
    PyObject * py_node = PyObject_CallFunction((PyObject *) &cMaBoSSNode, "sO", name, (PyObject *) self);
    if (py_node == NULL)
    {
      return NULL;
    }

    int rc = PyDict_SetItemString(self->nodes, name, py_node);
    Py_DECREF(py_node);
    if (rc < 0) {
      return NULL;
    }

  } catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  }
  
  Py_RETURN_NONE;
}

PyObject* cMaBoSSNetwork_setIState(cMaBoSSNetworkObject* self, PyObject *args) 
{
  PyObject* node = NULL;
  PyObject* istate = NULL;
  if (!PyArg_ParseTuple(args, "OO", &node, &istate))
    return NULL;
  
  try {
    if (PyObject_IsInstance(node, (PyObject*)&PyUnicode_Type) && (
      PyObject_IsInstance(istate, (PyObject *)&PyFloat_Type) || PyObject_IsInstance(istate, (PyObject *)&PyLong_Type)
    ))
    {
    
      Node* maboss_node = self->network->getNode(PyUnicode_AsUTF8(node));
      if (PyObject_IsInstance(istate, (PyObject *)&PyFloat_Type)) {
        IStateGroup::setNodeProba(self->network, maboss_node, PyFloat_AsDouble(istate));
      } else {
        IStateGroup::setNodeProba(self->network, maboss_node, PyLong_AsDouble(istate));
      }
    
    } else if (PyObject_IsInstance(node, (PyObject*)&PyUnicode_Type) && PyObject_IsInstance(istate, (PyObject *)&PyList_Type))
    {
      Node* maboss_node = self->network->getNode(PyUnicode_AsUTF8(node));
      double proba = PyFloat_AsDouble(PyList_GetItem(istate, 1))/(PyFloat_AsDouble(PyList_GetItem(istate, 0)) + PyFloat_AsDouble(PyList_GetItem(istate, 1)));
      if (PyObject_IsInstance(istate, (PyObject *)&PyFloat_Type)) {
        IStateGroup::setNodeProba(self->network, maboss_node, proba);
      } else {
        IStateGroup::setNodeProba(self->network, maboss_node, proba);
      }
  
  } else if (PyObject_IsInstance(node, (PyObject*)&PyList_Type) && PyObject_IsInstance(istate, (PyObject *)&PyDict_Type))
  {
      std::vector<const Node*>* istate_nodes = new std::vector<const Node*>();
      std::map<std::vector<bool>, double> istate_map;

      for (Py_ssize_t i = 0; i < PyList_Size(node); i++) {
        PyObject* name = PyList_GetItem(node, i);
        if (!PyUnicode_Check(name)) {
          delete istate_nodes;
          PyErr_SetString(PyExc_TypeError, "Node names must be strings");
          return NULL;
        }
        istate_nodes->push_back(self->network->getNode(PyUnicode_AsUTF8(name)));
      }

      // PyDict_Next walks the dict without building a keys list on every turn
      PyObject *boolean_state, *proba;
      Py_ssize_t pos = 0;
      while (PyDict_Next(istate, &pos, &boolean_state, &proba)) {

        std::vector<bool> istate_state;

        if (!PyTuple_Check(boolean_state)) {
          delete istate_nodes;
          PyErr_SetString(PyExc_TypeError, "Initial state keys must be tuples");
          return NULL;
        }

        if (PyTuple_Size(boolean_state) != PyList_Size(node)) {
          delete istate_nodes;
          PyErr_SetString(PyBNException, "The number of nodes and the number of boolean values do not match");
          return NULL;
        }

        for (Py_ssize_t j=0; j < PyTuple_Size(boolean_state); j++) {
          istate_state.push_back(PyLong_AsLong(PyTuple_GetItem(boolean_state, j)) == 1);
        }
        istate_map.insert(std::pair<std::vector<bool>, double>(istate_state, PyFloat_AsDouble(proba)));
      }

      if (PyErr_Occurred()) {
        delete istate_nodes;
        return NULL;
      }

      // setStatesProbas takes ownership of istate_nodes
      IStateGroup::setStatesProbas(self->network, istate_nodes, istate_map);
    }
  } catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  }
  
  Py_RETURN_NONE;
}

PyObject* cMaBoSSNetwork_getIState(cMaBoSSNetworkObject* self) 
{
  Py_RETURN_NONE;
}

PyObject * cMaBoSSNetwork_new(PyTypeObject* type, PyObject *args, PyObject* kwargs) 
{
  cMaBoSSNetworkObject* py_network = (cMaBoSSNetworkObject *) type->tp_alloc(type, 0);
  if (py_network == NULL) {
    return NULL;
  }
  py_network->nodes = PyDict_New();
  if (py_network->nodes == NULL) {
    Py_DECREF(py_network);
    return NULL;
  }
  py_network->network = new Network();
  return (PyObject*) py_network;
}

int cMaBoSSNetwork_init(PyObject* self, PyObject *args, PyObject* kwargs) 
{
  PyObject * network_file = Py_None;
  PyObject * network_str = Py_None;
  PyObject * use_sbml_names = Py_False;
  const char *kwargs_list[] = {"network", "network_str", "use_sbml_names", NULL};
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "|OOO", const_cast<char **>(kwargs_list), 
    &network_file, &network_str, &use_sbml_names
  ))
    return -1;
  
  cMaBoSSNetworkObject * py_network = (cMaBoSSNetworkObject *) self;
  
  try
  {
    if (network_file != Py_None) 
    {
      std::string network_file_str = std::string(PyUnicode_AsUTF8(network_file));
#ifdef SBML_COMPAT
      if (network_file_str.substr(network_file_str.find_last_of(".") + 1) == "sbml" || network_file_str.substr(network_file_str.find_last_of(".") + 1) == "xml" ) {
        py_network->network->parseSBML(network_file_str.c_str(), NULL, (use_sbml_names == Py_True)); 
      } else {
#endif
        py_network->network->parse(network_file_str.c_str());
#ifdef SBML_COMPAT
      }
#endif
    } else if (network_str != Py_None) 
    {
      py_network->network->parseExpression(PyUnicode_AsUTF8(network_str));
      
    } else {
      PyErr_SetString(PyBNException, "No network file or string provided");
      return -1;
    }
      
    IStateGroup::checkAndComplete(py_network->network);
    // Building dictionary of nodes
    for (auto* node: py_network->network->getNodes())
    {
      PyObject * py_node = PyObject_CallFunction((PyObject *) &cMaBoSSNode, "sO", node->getLabel().c_str(), (PyObject *) py_network);
      if (py_node == NULL)
      {
        return -1;
      }
      int rc = PyDict_SetItemString(py_network->nodes, node->getLabel().c_str(), py_node);
      Py_DECREF(py_node);
      if (rc < 0) {
        return -1;
      }
    }

  } catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return -1;
  }
  
  return 0;
  
}
