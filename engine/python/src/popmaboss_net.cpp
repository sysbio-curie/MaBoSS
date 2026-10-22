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
#include "popmaboss_net.h"
#include "maboss_node.h"
#include "maboss_net.h"


PyMethodDef cPopMaBoSSNetwork_methods[] = {
    // {"getNetwork", (PyCFunction) cPopMaBoSSNetwork_getNetwork, METH_NOARGS, "returns the network object"},
    // {"getNodes", (PyCFunction) cPopMaBoSSNetwork_getDictNodes, METH_NOARGS, "returns the dict of nodes"},
    {"set_output", (PyCFunction) cPopMaBoSSNetwork_setOutput, METH_VARARGS, "set the output nodes"},
    {"get_output", (PyCFunction) cPopMaBoSSNetwork_getOutput, METH_NOARGS, "returns the output nodes"},
    {"set_death_rate", (PyCFunction) cPopMaBoSSNetwork_setDeathRate, METH_VARARGS, "sets the death rate"},
    {"get_death_rate", (PyCFunction) cPopMaBoSSNetwork_getDeathRate, METH_NOARGS, "gets the death rate"},
    {"add_division_rule", (PyCFunction) cPopMaBoSSNetwork_addDivisionRule, METH_VARARGS, "adds a division rule"},
    {"remove_division_rule", (PyCFunction) cPopMaBoSSNetwork_removeDivisionRule, METH_VARARGS, "removes a division rule"},
    {"get_division_rules", (PyCFunction) cPopMaBoSSNetwork_getDivisionRules, METH_NOARGS, "gets the division rules"},
    {"set_istate", (PyCFunction) cPopMaBoSSNetwork_setIstate, METH_VARARGS, "sets the initial state"},
    {"set_pop_istate", (PyCFunction) cPopMaBoSSNetwork_setPopIstate, METH_VARARGS, "sets the initial population state"},
    {"clear_pop_istate", (PyCFunction) cPopMaBoSSNetwork_clearPopIstate, METH_NOARGS, "clears the initial population state"},
    {"keys", (PyCFunction) cPopMaBoSSNetwork_Keys, METH_NOARGS, "returns the keys"},
    {"values", (PyCFunction) cPopMaBoSSNetwork_Values, METH_NOARGS, "returns the values"},
    {"items", (PyCFunction) cPopMaBoSSNetwork_Items, METH_NOARGS, "returns the items"},
    {NULL}  /* Sentinel */
};

PyMappingMethods cPopMaBoSSNetwork_mapping = {
	(lenfunc)cPopMaBoSSNetwork_NodesLength,		
	(binaryfunc)cPopMaBoSSNetwork_NodesGetItem,
	(objobjargproc)cPopMaBoSSNetwork_NodesSetItem,
};

PyTypeObject cPopMaBoSSNetwork = {
  PyVarObject_HEAD_INIT(NULL, 0)
  build_type_name("cPopMaBoSSNetworkObject"),               /* tp_name */
  sizeof(cPopMaBoSSNetworkObject),               /* tp_basicsize */
    0,                              /* tp_itemsize */
  (destructor) cPopMaBoSSNetwork_dealloc,      /* tp_dealloc */
    0,                              /* tp_vectorcall_offset */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_as_async */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
  &cPopMaBoSSNetwork_mapping,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
  cPopMaBoSSNetwork_str,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,         /* tp_flags */
  "cPopMaBoSS Network object",                   /* tp_doc */
  (traverseproc) cPopMaBoSSNetwork_traverse,   /* tp_traverse */
  (inquiry) cPopMaBoSSNetwork_clear,           /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
  cPopMaBoSSNetwork_methods,                              /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
  cPopMaBoSSNetwork_init,                              /* tp_init */
    0,                              /* tp_alloc */
  cPopMaBoSSNetwork_new,                      /* tp_new */   
};

int cPopMaBoSSNetwork_traverse(cPopMaBoSSNetworkObject *self, visitproc visit, void *arg)
{
  Py_VISIT(self->nodes);
  return 0;
}

int cPopMaBoSSNetwork_clear(cPopMaBoSSNetworkObject *self)
{
  Py_CLEAR(self->nodes);
  return 0;
}

void cPopMaBoSSNetwork_dealloc(cPopMaBoSSNetworkObject *self)
{
    PyObject_GC_UnTrack(self);
    Py_CLEAR(self->nodes);
    delete self->network;
    self->network = NULL;
    Py_TYPE(self)->tp_free((PyObject *) self);
}

PyObject *cPopMaBoSSNetwork_str(PyObject *self) {
  return PyUnicode_FromString(((cPopMaBoSSNetworkObject* )self)->network->toString().c_str());
}

int cPopMaBoSSNetwork_NodesSetItem(cPopMaBoSSNetworkObject* self, PyObject *key, PyObject* value)
{
  // value is NULL when Python is deleting the entry (del net[key])
  if (value == NULL) {
    return PyDict_DelItem(self->nodes, key);
  }
  return PyDict_SetItem(self->nodes, key, value);
}

PyObject * cPopMaBoSSNetwork_NodesGetItem(cPopMaBoSSNetworkObject* self, PyObject *key)
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

Py_ssize_t cPopMaBoSSNetwork_NodesLength(cPopMaBoSSNetworkObject* self)
{
  return PyObject_Length(self->nodes);
}

PyObject* cPopMaBoSSNetwork_Keys(cPopMaBoSSNetworkObject* self)
{
  return PyDict_Keys(self->nodes);
}

PyObject* cPopMaBoSSNetwork_Values(cPopMaBoSSNetworkObject* self)
{
  return PyDict_Values(self->nodes);
}

PyObject* cPopMaBoSSNetwork_Items(cPopMaBoSSNetworkObject* self)
{
  return PyDict_Items(self->nodes);
}

PyObject* cPopMaBoSSNetwork_getDeathRate(cPopMaBoSSNetworkObject* self) 
{
  const Expression* death_rate = self->network->getDeathRate();
  if (death_rate == NULL) {
    Py_RETURN_NONE;
  }
  
  return PyUnicode_FromString(death_rate->toString().c_str());
}

PyObject* cPopMaBoSSNetwork_setDeathRate(cPopMaBoSSNetworkObject* self, PyObject *args) 
{

  PyObject * death_rate = NULL;
  if (!PyArg_ParseTuple(args, "|O", &death_rate))
    return NULL;
  
  try
  {
    if (death_rate != NULL) {
      std::string death_rate_str = std::string("death {\nrate=") + std::string(PyUnicode_AsUTF8(death_rate)) + std::string(";\n}");
      self->network->parseExpression(death_rate_str.c_str());
    }
    else 
    {
      self->network->setDeathRate(NULL);
    }
  } catch (BNException& e) {
    PyErr_SetString(PyBNException, (std::string(PyUnicode_AsUTF8(death_rate)) + std::string(" is not a valid expression")).c_str());
    return NULL;
  }
  
  Py_RETURN_NONE;
}

PyObject* cPopMaBoSSNetwork_setOutput(cPopMaBoSSNetworkObject* self, PyObject *args) 
{
  PyObject* list;
  if (!PyArg_ParseTuple(args, "O", &list))
    return NULL;

  return cMaBoSSNetwork_setOutputNodes(self->network, list);
}

PyObject* cPopMaBoSSNetwork_getOutput(cPopMaBoSSNetworkObject* self)
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

PyObject* cPopMaBoSSNetwork_addDivisionRule(cPopMaBoSSNetworkObject* self, PyObject *args) 
{
  char* rule = NULL;
  PyObject* daugther_1 = NULL;
  PyObject* daugther_2 = NULL;
  if (!PyArg_ParseTuple(args, "s|OO", &rule,&daugther_1,&daugther_2))
    return NULL;
  
  try{
    std::string division_rule = std::string("division {\nrate=") + std::string(rule) + ";\n";
    // PyDict_Next avoids rebuilding (and leaking) a keys list on every turn
    if (daugther_1 != NULL && daugther_1 != Py_None){
      if (!PyDict_Check(daugther_1)) {
        PyErr_SetString(PyExc_TypeError, "daughter 1 must be a dict");
        return NULL;
      }
      PyObject *key, *value;
      Py_ssize_t pos = 0;
      while (PyDict_Next(daugther_1, &pos, &key, &value))
      {
        if (!PyUnicode_Check(key)) {
          PyErr_SetString(PyExc_TypeError, "Node names must be strings");
          return NULL;
        }
        std::string key_str = PyUnicode_AsUTF8(key);
        std::string value_str = std::to_string(PyLong_AsLong(value));
        division_rule += key_str + std::string(".DAUGHTER1=") + value_str + ";\n";
      }
    }
    if (daugther_2 != NULL && daugther_2 != Py_None){
      if (!PyDict_Check(daugther_2)) {
        PyErr_SetString(PyExc_TypeError, "daughter 2 must be a dict");
        return NULL;
      }
      PyObject *key, *value;
      Py_ssize_t pos = 0;
      while (PyDict_Next(daugther_2, &pos, &key, &value))
      {
        if (!PyUnicode_Check(key)) {
          PyErr_SetString(PyExc_TypeError, "Node names must be strings");
          return NULL;
        }
        std::string key_str = PyUnicode_AsUTF8(key);
        std::string value_str = std::to_string(PyLong_AsLong(value));
        division_rule += key_str + std::string(".DAUGHTER2=") + value_str + ";\n";
      }
    }
    
    division_rule += std::string("}");
    self->network->parseExpression(division_rule.c_str());
  } catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  }
  
  Py_RETURN_NONE;
}

PyObject* cPopMaBoSSNetwork_getDivisionRules(cPopMaBoSSNetworkObject* self) 
{
  PyObject* rules = PyDict_New();
  size_t index = 0;
  for (auto rule: self->network->getDivisionRules()) 
  {
    PyObject* rate = PyUnicode_FromString(rule->rate->toString().c_str());

    // PyDict_SetItemString does not steal, so each value must be released
    PyObject* daugther_1 = PyDict_New();
    for (auto map: rule->daughters[DivisionRule::DAUGHTER_1]) {
      PyObject* v = PyUnicode_FromString(map.second->toString().c_str());
      PyDict_SetItemString(daugther_1, map.first->getLabel().c_str(), v);
      Py_XDECREF(v);
    }

    PyObject* daugther_2 = PyDict_New();
    for (auto map: rule->daughters[DivisionRule::DAUGHTER_2]) {
      PyObject* v = PyUnicode_FromString(map.second->toString().c_str());
      PyDict_SetItemString(daugther_2, map.first->getLabel().c_str(), v);
      Py_XDECREF(v);
    }

    // "N" steals rate/daugther_1/daugther_2 into the tuple; the tuple and the
    // key are then released once the dict holds its own reference
    PyObject* tuple = Py_BuildValue("NNN", rate, daugther_1, daugther_2);
    PyObject* key = PyLong_FromUnsignedLong(index);
    if (tuple == NULL || key == NULL || PyDict_SetItem(rules, key, tuple) < 0) {
      Py_XDECREF(tuple);
      Py_XDECREF(key);
      Py_DECREF(rules);
      return NULL;
    }
    Py_DECREF(tuple);
    Py_DECREF(key);

    index++;
  }

  return rules;
}

PyObject* cPopMaBoSSNetwork_removeDivisionRule(cPopMaBoSSNetworkObject* self, PyObject* args)
{
  PyObject* index = NULL;
  if (!PyArg_ParseTuple(args, "O", &index))
    return NULL;
  
  try
  {
    if (index != NULL && PyObject_IsInstance(index, (PyObject*) &PyLong_Type)) {
      self->network->removeDivisionRule(PyLong_AsLong(index));
    } else {
      PyErr_SetString(PyBNException, "Bad index");
      return NULL;
    }
  } catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  }
  
  Py_RETURN_NONE;
}

PyObject * cPopMaBoSSNetwork_new(PyTypeObject* type, PyObject *args, PyObject* kwargs) 
{
  cPopMaBoSSNetworkObject* py_network = (cPopMaBoSSNetworkObject *) type->tp_alloc(type, 0);
  if (py_network == NULL) {
    return NULL;
  }
  py_network->nodes = PyDict_New();
  if (py_network->nodes == NULL) {
    Py_DECREF(py_network);
    return NULL;
  }
  py_network->network = new PopNetwork();
  return (PyObject*) py_network;
}

int cPopMaBoSSNetwork_init(PyObject* self, PyObject *args, PyObject* kwargs) 
{
  PyObject * network_file = Py_None;
  PyObject * network_str = Py_None;
  const char *kwargs_list[] = {"network", "network_str", NULL};
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "|OO", const_cast<char **>(kwargs_list), 
    &network_file, &network_str
  ))
    return -1;
  
  cPopMaBoSSNetworkObject* py_network = (cPopMaBoSSNetworkObject *) self;
  
  try
  {
    if (network_file != Py_None) 
    {
      py_network->network->parse(PyUnicode_AsUTF8(network_file));  
      
    } else if (network_str != Py_None)
    {
      py_network->network->parseExpression(PyUnicode_AsUTF8(network_str));
      
    } else {
      PyErr_SetString(PyBNException, "No network file or string provided");
      return -1;
    }

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

PyObject* cPopMaBoSSNetwork_setIstate(cPopMaBoSSNetworkObject* self, PyObject *args)
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
    // [A,B,C] : { (0,1,1): 0.5, (1,0,1): 0.5 }
    // [A,B,C] = { ((0,1,1),5):10, ((1,0,0),2): 0.5}
    // [TumorCell,DC].pop_istate = 1.0 [{[1,0]:18} , {[0,1]:2}];

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

      // PyDict_Next avoids rebuilding (and leaking) a keys list on every turn
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

PyObject* cPopMaBoSSNetwork_setPopIstate(cPopMaBoSSNetworkObject* self, PyObject *args)
{
  PyObject* node = NULL;
  PyObject* istate = NULL;
  if (!PyArg_ParseTuple(args, "OO", &node, &istate))
    return NULL;
  
  try {
    if (PyObject_IsInstance(node, (PyObject*)&PyList_Type) && PyObject_IsInstance(istate, (PyObject *)&PyDict_Type))
    {
    
      // ['TumorCell','DC'] = { (((1,0),18), ((0,1),2)):1.0}
      // [TumorCell,DC].pop_istate = 1.0 [{[1,0]:18} , {[0,1]:2}];

      std::vector<const Node*>* istate_nodes = new std::vector<const Node*>();
      std::vector<PopIStateGroup::PopProbaIState*>* istate_map = new std::vector<PopIStateGroup::PopProbaIState*>();
      
      for (Py_ssize_t i = 0; i < PyList_Size(node); i++) {
        PyObject* name = PyList_GetItem(node, i);
        if (!PyUnicode_Check(name)) {
          PyErr_SetString(PyExc_TypeError, "Node names must be strings");
          return NULL;
        }
        istate_nodes->push_back(self->network->getNode(PyUnicode_AsUTF8(name)));
      }

      //Parsing each key : each pop state
      // PyDict_Next avoids rebuilding (and leaking) a keys list on every turn
      PyObject *py_pop_state, *py_proba;
      Py_ssize_t pos = 0;
      while (PyDict_Next(istate, &pos, &py_pop_state, &py_proba))
      {
        if (!PyTuple_Check(py_pop_state)) {
          PyErr_SetString(PyExc_TypeError, "Population state keys must be tuples");
          return NULL;
        }

        // if (PyObject_IsInstance(PyTuple_GetItem(py_pop_state, 0), (PyObject *)&PyTuple_Type) && PyObject_IsInstance(PyTuple_GetItem(py_pop_state, 1), (PyObject *)&PyLong_Type)) {
        //   PyErr_SetString(PyBNException, "Keys should be tuples of tuples, integers");
        //   return NULL;
        // }
        
        std::vector<PopIStateGroup::PopProbaIState::PopIStateGroupIndividual*>* individual_pop_istates = new std::vector<PopIStateGroup::PopProbaIState::PopIStateGroupIndividual*>();
        
        // Parsing each cell state
        for (Py_ssize_t j=0; j < PyTuple_Size(py_pop_state); j++) 
        {  
          PyObject* py_boolean_state = PyTuple_GetItem(PyTuple_GetItem(py_pop_state, j), 0);
          
          std::vector<double> boolean_state;
          for (Py_ssize_t k=0; k < PyTuple_Size(py_boolean_state); k++) {
            boolean_state.push_back((double) (PyLong_AsLong(PyTuple_GetItem(py_boolean_state, k)) == 1));
          }
          unsigned int pop = PyLong_AsLong(PyTuple_GetItem(PyTuple_GetItem(py_pop_state, j), 1));
          
          individual_pop_istates->push_back(new PopIStateGroup::PopProbaIState::PopIStateGroupIndividual(boolean_state, pop));
        }
        
        double proba = PyFloat_AsDouble(py_proba);
        istate_map->push_back(new PopIStateGroup::PopProbaIState(proba, individual_pop_istates));
      }
      
      new PopIStateGroup(self->network, istate_nodes, istate_map);
    } 
  } catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  }
  
  Py_RETURN_NONE;
  
}

PyObject* cPopMaBoSSNetwork_clearPopIstate(cPopMaBoSSNetworkObject* self)
{
  self->network->clearPopIstates();
  Py_RETURN_NONE;
}
