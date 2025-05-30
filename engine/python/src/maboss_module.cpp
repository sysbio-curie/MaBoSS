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
     maboss_module.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2020
*/

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define PY_ARRAY_UNIQUE_SYMBOL MABOSS_ARRAY_API
#include <numpy/arrayobject.h>

#include "maboss_commons.h"
#include "maboss_node.h"
#include "maboss_sim.h"
#include "popmaboss_sim.h"
#include "maboss_res.h"
#include "maboss_resfinal.h"
#include "popmaboss_res.h"
#ifdef SEDML_COMPAT
#include "sedml_sim.h"
#endif

#if !defined (MAXNODES) || MAXNODES == 0xFFFFFFF 
    const char module_name[] = "cmaboss_dn";
#elif MAXNODES <= 64 
    const char module_name[] = "cmaboss";
#else
    const char module_name[] = STR(MODULE_NAME);
#endif

const char * build_type_name(const char* object_name) {
    char * name = new char[strlen(module_name) + strlen(object_name) + 2];
    strcat(strcpy(name, module_name), ".");
    strcat(name, object_name);
    return name;
}

PyObject* PyBNException = NULL;

/*  define functions in module */
PyMethodDef cMaBoSS[] =
{ 
     {NULL, NULL, 0, NULL}
};

/* module initialization */
/* Python version 3*/
struct PyModuleDef cMaBoSSDef =
{
    PyModuleDef_HEAD_INIT,
#if !defined (MAXNODES) || MAXNODES == 0xFFFFFFF 
    "cmaboss_dn",
#elif MAXNODES <= 64 
    "cmaboss", 
#else
    STR(MODULE_NAME),
#endif
    "Some documentation",
    -1,
    cMaBoSS
};

PyMODINIT_FUNC
#if !defined (MAXNODES) || MAXNODES == 0xFFFFFFF 
PyInit_cmaboss_dn(void)
#elif MAXNODES <= 64 
PyInit_cmaboss(void)
#else
#define MODULE_INIT_NAME NAME1(PyInit_, MODULE_NAME)
MODULE_INIT_NAME(void)
#endif
{
    MaBEstEngine::init();
    import_array();
    
    PyObject *m;
    if (PyType_Ready(&cMaBoSSNetwork) < 0){
        return NULL;
    }
    if (PyType_Ready(&cMaBoSSConfig) < 0){
        return NULL;
    }
    if (PyType_Ready(&cMaBoSSSim) < 0){
        return NULL;
    }
    if (PyType_Ready(&cMaBoSSResult) < 0){
        return NULL;
    }
    if (PyType_Ready(&cMaBoSSResultFinal) < 0){
        return NULL;
    }
    if (PyType_Ready(&cPopMaBoSSSim) < 0){
        return NULL;
    }
    if (PyType_Ready(&cPopMaBoSSNetwork) < 0){
        return NULL;
    }
    if (PyType_Ready(&cPopMaBoSSResult) < 0){
        return NULL;
    }
    if (PyType_Ready(&cMaBoSSParam) < 0){
        return NULL;
    }
    if (PyType_Ready(&cMaBoSSNode) < 0){
        return NULL;
    }
#ifdef SEDML_COMPAT
    if (PyType_Ready(&sedmlSim) < 0){
        return NULL;
    }
#endif

    m = PyModule_Create(&cMaBoSSDef);

#if !defined (MAXNODES) || MAXNODES == 0xFFFFFFF 
    char exception_name[50] = "cmaboss_dn.BNException";
#elif MAXNODES <= 64 
    char exception_name[50] = "cmaboss.BNException";
#else
    char exception_name[50] = STR(MODULE_NAME);
    strcat(exception_name, ".BNException");
#endif
    PyBNException = PyErr_NewException(exception_name, NULL, NULL);
    PyModule_AddObject(m, "BNException", PyBNException);
        
    Py_INCREF(&cMaBoSSSim);
    if (PyModule_AddObject(m, "MaBoSSSim", (PyObject *) &cMaBoSSSim) < 0) {
        Py_DECREF(&cMaBoSSSim);
        Py_DECREF(m);
        return NULL;
    }
    
    Py_INCREF(&cMaBoSSParam);
    if (PyModule_AddObject(m, "MaBoSSParam", (PyObject *) &cMaBoSSParam) < 0) {
        Py_DECREF(&cMaBoSSParam);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&cPopMaBoSSSim);
    if (PyModule_AddObject(m, "PopMaBoSSSim", (PyObject *) &cPopMaBoSSSim) < 0) {
        Py_DECREF(&cPopMaBoSSSim);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&cMaBoSSNode);
    if (PyModule_AddObject(m, "MaBoSSNode", (PyObject *) &cMaBoSSNode) < 0) {
        Py_DECREF(&cMaBoSSNode);
        Py_DECREF(m);
        return NULL;
    }
    
    Py_INCREF(&cMaBoSSNetwork);
    if (PyModule_AddObject(m, "MaBoSSNet", (PyObject *) &cMaBoSSNetwork) < 0) {
        Py_DECREF(&cMaBoSSNetwork);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&cMaBoSSConfig);
    if (PyModule_AddObject(m, "MaBoSSCfg", (PyObject *) &cMaBoSSConfig) < 0) {
        Py_DECREF(&cMaBoSSConfig);
        Py_DECREF(m);
        return NULL;
    }
    
    Py_INCREF(&cPopMaBoSSNetwork);
    if (PyModule_AddObject(m, "PopMaBoSSNet", (PyObject *) &cPopMaBoSSNetwork) < 0) {
        Py_DECREF(&cPopMaBoSSNetwork);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&cMaBoSSResult);
    if (PyModule_AddObject(m, "cMaBoSSResult", (PyObject *) &cMaBoSSResult) < 0) {
        Py_DECREF(&cMaBoSSResult);
        Py_DECREF(m);
        return NULL;
    }
    
    Py_INCREF(&cMaBoSSResultFinal);
    if (PyModule_AddObject(m, "cMaBoSSResultFinal", (PyObject *) &cMaBoSSResultFinal) < 0) {
        Py_DECREF(&cMaBoSSResultFinal);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&cPopMaBoSSResult);
    if (PyModule_AddObject(m, "cPopMaBoSSResult", (PyObject *) &cPopMaBoSSResult) < 0) {
        Py_DECREF(&cPopMaBoSSResult);
        Py_DECREF(m);
        return NULL;
    }
    
#ifdef SEDML_COMPAT
    Py_INCREF(&sedmlSim);
    if (PyModule_AddObject(m, "sedmlSim", (PyObject *) &sedmlSim) < 0) {
        Py_DECREF(&sedmlSim);
        Py_DECREF(m);
        return NULL;
    }
#endif
    return m;
}