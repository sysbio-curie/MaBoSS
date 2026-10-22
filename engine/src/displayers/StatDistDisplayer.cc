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
     ProjTrajDisplayer.cc

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     December 2020
*/

#include "StatDistDisplayer.h"
#include "../Utils.h"
#include <iomanip>
#include <map>
#include <set>

void JSONStatDistDisplayer::beginDisplay() {
  //os_statdist << "{";
}

void JSONStatDistDisplayer::beginStatDistDisplay() {
  //os_statdist << "\"statdist\":[";
  os_statdist << "[";
}

void JSONStatDistDisplayer::endStatDistDisplay() {
  os_statdist << "]";
}

void JSONStatDistDisplayer::beginStateProbaDisplay() {
  std::ostream&os = cluster_mode ? os_statdist_cluster : os_statdist;
  if (current_line > 0) {
    os << ",";
  }
  os << "{\"num\":" << num << ",\"state_probas\":[";
  current_state_proba = 0;
}

void JSONStatDistDisplayer::addStateProba(const NetworkState_Impl& state, double proba) {
  std::ostream&os = cluster_mode ? os_statdist_cluster : os_statdist;
  if (current_state_proba > 0) {
    os << ",";
  }
  os << "{";
#ifdef USE_DYNAMIC_BITSET
  NetworkState network_state(state, 1);
#else
  NetworkState network_state(state);
#endif
  os << "\"state\":\"";
  network_state.displayOneLine(os, network);
  os << "\",\"proba\":";
  if (hexfloat) {
    os << fmthexdouble(proba, true);
  } else {
    os << proba;
  }
  os << "}";
  current_state_proba++;
}

void JSONStatDistDisplayer::endStateProbaDisplay() {
  std::ostream&os = cluster_mode ? os_statdist_cluster : os_statdist;
  os << "]}";
}

void JSONStatDistDisplayer::endDisplay() {
  //os_statdist << "}";
}

void JSONStatDistDisplayer::beginFactoryCluster() {
    os_statdist_cluster << "[";
    cluster_mode = true;
}

void JSONStatDistDisplayer::endFactoryCluster() {
    os_statdist_cluster << "]";
    cluster_mode = false;
}

void JSONStatDistDisplayer::beginCluster(size_t _num, size_t size) {
  if (_num > 1) {
    os_statdist_cluster << ",";
  }
  os_statdist_cluster << "{\"num\":" << _num << ",\"size\":" << size << ",\"cluster\":[";
  current_state_proba = 0;
  current_line = 0;
}

void JSONStatDistDisplayer::endCluster() {
  os_statdist_cluster << "]}";
}

void JSONStatDistDisplayer::beginClusterFactoryStationaryDistribution() {
  os_statdist_distrib << "[";
}

void JSONStatDistDisplayer::endClusterFactoryStationaryDistribution() {
  os_statdist_distrib << "]";
}

void JSONStatDistDisplayer::beginClusterStationaryDistribution(size_t _num) {
  if (_num > 1) {
    os_statdist_distrib << ",";
  }
  os_statdist_distrib << "{\"num\":" << _num << ",\"proba_variances\":[";
  current_state_proba = 0;
}

void JSONStatDistDisplayer::endClusterStationaryDistribution() {
  os_statdist_distrib << "]}";
}

void JSONStatDistDisplayer::addProbaVariance(const NetworkState_Impl& state, double proba, double variance) {
  if (current_state_proba > 0) {
    os_statdist_distrib << ",";
  }
  os_statdist_distrib << "{\"state\":\"";
#ifdef USE_DYNAMIC_BITSET
  NetworkState network_state(state, 1);
#else
  NetworkState network_state(state);
#endif
  network_state.displayOneLine(os_statdist_distrib, network);
  os_statdist_distrib << "\"";
  if (hexfloat) {
    os_statdist_distrib << ",\"proba\":" << fmthexdouble(proba, true);
    os_statdist_distrib << ",\"variance\":" << fmthexdouble(variance, true);
  } else {
    os_statdist_distrib << ",\"proba\":" << proba;
    os_statdist_distrib << ",\"variance\":" << variance;
  }
  os_statdist_distrib << "}";
  current_state_proba++;
}

void CSVStatDistDisplayer::beginDisplay() {
  os_statdist << "Trajectory";
  for (unsigned int nn = 0; nn < max_size; ++nn) {
    os_statdist << "\tState\tProba";
  }
  os_statdist << '\n';
}

void CSVStatDistDisplayer::beginStatDistDisplay() {
}

void CSVStatDistDisplayer::beginStateProbaDisplay() {
  os_statdist << "#" << num;
  os_statdist << std::setprecision(10);
}

void CSVStatDistDisplayer::addStateProba(const NetworkState_Impl& state, double proba) {
  
#ifdef USE_DYNAMIC_BITSET 
  NetworkState network_state(state, 1);
#else
  NetworkState network_state(state);
#endif
  os_statdist << '\t';
  network_state.displayOneLine(os_statdist, network);
  if (hexfloat) {
    os_statdist << '\t' << fmthexdouble(proba);
  } else {
    os_statdist << '\t' << proba;
  }
}

void CSVStatDistDisplayer::endStateProbaDisplay() {
  os_statdist << '\n';
}

void CSVStatDistDisplayer::endStatDistDisplay() {
}

void CSVStatDistDisplayer::endDisplay() {
}

void CSVStatDistDisplayer::beginCluster(size_t _num, size_t size) {
  os_statdist << "\nTrajectory[cluster=#" << _num << ",size=" << size << "]\tState\tProba\tState\tProba\tState\tProba\tState\tProba ...\n";
}

void CSVStatDistDisplayer::endCluster() {
}

void CSVStatDistDisplayer::beginClusterFactoryStationaryDistribution() {
  os_statdist << "\nCluster\tState\tProba\tErrorProba\tState\tProba\tErrorProba\tState\tProba\tErrorProba\tState\tProba\tErrorProba...\n";
}

void CSVStatDistDisplayer::endClusterFactoryStationaryDistribution() {
}

void CSVStatDistDisplayer::beginClusterStationaryDistribution(size_t _num) {
  os_statdist << "#" << _num;
}

void CSVStatDistDisplayer::endClusterStationaryDistribution() {
  os_statdist << '\n';
}

void CSVStatDistDisplayer::addProbaVariance(const NetworkState_Impl& state, double proba, double variance) {
#ifdef USE_DYNAMIC_BITSET
  NetworkState network_state(state, 1);
#else
  NetworkState network_state(state);
#endif
  os_statdist << '\t';
  network_state.displayOneLine(os_statdist, network);
  if (hexfloat) {
    os_statdist << '\t' << fmthexdouble(proba) << '\t';
    os_statdist << fmthexdouble(variance);
  } else {
    os_statdist << '\t' << proba << '\t';
    os_statdist << variance;
  }
}

#ifdef HDF5_COMPAT


void HDF5StatDistDisplayer::beginDisplay() {
}

void HDF5StatDistDisplayer::beginStatDistDisplay() {
}

void HDF5StatDistDisplayer::endStatDistDisplay() {
}

void HDF5StatDistDisplayer::beginStateProbaDisplay() {
}

void HDF5StatDistDisplayer::addStateProba(const NetworkState_Impl& state, double proba) {
}

void HDF5StatDistDisplayer::endStateProbaDisplay() {
}

void HDF5StatDistDisplayer::endDisplay() {
}

void HDF5StatDistDisplayer::beginFactoryCluster() {
}

void HDF5StatDistDisplayer::endFactoryCluster() {
}

void HDF5StatDistDisplayer::beginCluster(size_t _num, size_t size) {
}

void HDF5StatDistDisplayer::endCluster() {
}

void HDF5StatDistDisplayer::beginClusterFactoryStationaryDistribution() {
}

void HDF5StatDistDisplayer::endClusterFactoryStationaryDistribution() {
}

void HDF5StatDistDisplayer::beginClusterStationaryDistribution(size_t _num) {
}

void HDF5StatDistDisplayer::endClusterStationaryDistribution() {
}

void HDF5StatDistDisplayer::addProbaVariance(const NetworkState_Impl& state, double proba, double variance) {
}


#endif

#ifdef PYTHON_API
void NumpyStatDistDisplayer::beginStateProbaDisplay() {
  current_table->numbers.push_back(num);
  current_table->probas.emplace_back();
}

void NumpyStatDistDisplayer::addStateProba(const NetworkState_Impl& state, double proba) {
  current_table->probas.back().emplace_back(state, proba);
}

void NumpyStatDistDisplayer::beginCluster(size_t num, size_t size) {
  clusters.emplace_back();
  current_table = &clusters.back();
}

void NumpyStatDistDisplayer::beginClusterStationaryDistribution(size_t num) {
  current_table->numbers.push_back(num);
  current_table->probas.emplace_back();
  current_table->errors.emplace_back();
}

void NumpyStatDistDisplayer::addProbaVariance(const NetworkState_Impl& state, double proba, double variance) {
  current_table->probas.back().emplace_back(state, proba);
  current_table->errors.back().emplace_back(state, variance);
}

PyObject* NumpyStatDistDisplayer::getNumpyTable(const Table& table, bool with_errors) const {

  std::set<NetworkState_Impl> states;
  for (const Row& row: table.probas) {
    for (const auto& cell: row) {
      states.insert(cell.first);
    }
  }
  std::map<NetworkState_Impl, size_t> pos_states;
  PyObject* pylist_states = PyList_New(states.size());
  size_t i = 0;
  for (const NetworkState_Impl& state: states) {
    pos_states[state] = i;
#ifdef USE_DYNAMIC_BITSET
    NetworkState network_state(state, 1);
#else
    NetworkState network_state(state);
#endif
    PyList_SetItem(pylist_states, i, PyUnicode_FromString(network_state.getName(network).c_str()));
    i++;
  }

  npy_intp dims[2] = {(npy_intp) table.probas.size(), (npy_intp) states.size()};
  PyArrayObject* probas = (PyArrayObject *) PyArray_ZEROS(2, dims, NPY_DOUBLE, 0);
  for (size_t nn = 0; nn < table.probas.size(); nn++) {
    for (const auto& cell: table.probas[nn]) {
      *(double *) PyArray_GETPTR2(probas, nn, pos_states[cell.first]) = cell.second;
    }
  }

  PyObject* pylist_numbers = PyList_New(table.numbers.size());
  for (size_t nn = 0; nn < table.numbers.size(); nn++) {
    PyList_SetItem(pylist_numbers, nn, PyLong_FromSize_t(table.numbers[nn]));
  }

  if (!with_errors) {
    return Py_BuildValue("NNN", PyArray_Return(probas), pylist_numbers, pylist_states);
  }

  PyArrayObject* errors = (PyArrayObject *) PyArray_ZEROS(2, dims, NPY_DOUBLE, 0);
  for (size_t nn = 0; nn < table.errors.size(); nn++) {
    for (const auto& cell: table.errors[nn]) {
      *(double *) PyArray_GETPTR2(errors, nn, pos_states[cell.first]) = cell.second;
    }
  }

  return Py_BuildValue("NNNN", PyArray_Return(probas), pylist_numbers, pylist_states, PyArray_Return(errors));
}

PyObject* NumpyStatDistDisplayer::getNumpyStatDist() const {

  PyObject* pylist_clusters = PyList_New(clusters.size());
  for (size_t nn = 0; nn < clusters.size(); nn++) {
    PyList_SetItem(pylist_clusters, nn, getNumpyTable(clusters[nn], false));
  }

  return Py_BuildValue("NNN", getNumpyTable(statdist, false), pylist_clusters, getNumpyTable(summary, true));
}
#endif
