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
     Cumulator.h

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2011
*/

#ifndef _CUMULATOR_H_
#define _CUMULATOR_H_

#include <string>
#include <map>
#include <set>
#include <vector>
#include <assert.h>
#include <cfloat>

#ifdef PYTHON_API
#include "Python_headers.h"
#endif
#ifdef MPI_COMPAT
#include "MPI_headers.h"
#endif

static bool COMPUTE_ERRORS = true;

#include "RunConfig.h"
#include "ProbaDist.h"
#include "RunConfig.h"
#include "displayers/StatDistDisplayer.h"
#include "displayers/ProbTrajDisplayer.h"

class Network;
template <typename S> class ProbTrajDisplayer;

template <typename S>
class Cumulator {

  struct TickValue {
    double tm_slice;
    double TH;
    double tm_slice_square;
    TickValue() {tm_slice = 0.; TH = 0.; tm_slice_square = 0.;}
    TickValue(double tm_slice, double TH) : tm_slice(tm_slice), TH(TH), tm_slice_square(0.0) { }
  };

  struct LastTickValue {
    double tm_slice;
    double TH;
    LastTickValue() : tm_slice(0.0), TH(0.0) {}
    LastTickValue(double tm_slice, double TH) : tm_slice(tm_slice), TH(TH) {}
  };

  class CumulMap {
    STATE_MAP<S, TickValue> mp;

  public:
    size_t size() const {
      return mp.size();
    }

    void incr(const S& state, double tm_slice, double TH) {
      auto iter = mp.find(state);
      if (iter == mp.end()) {
#ifdef USE_DYNAMIC_BITSET
  S state_copy(state, 1);
	mp[state_copy] = TickValue(tm_slice, tm_slice * TH);
#else
  mp[state] = TickValue(tm_slice, tm_slice * TH);
#endif
      } else {
	(*iter).second.tm_slice += tm_slice;
	(*iter).second.TH += tm_slice * TH;
      }
    }

    void cumulTMSliceSquare(const S& state, double tm_slice) {
      auto iter = mp.find(state);
      assert(iter != mp.end());
      (*iter).second.tm_slice_square += tm_slice * tm_slice;
    }
    
    void add(const S& state, const TickValue& tick_value) {
      auto iter = mp.find(state);
      if (iter == mp.end()) {
#ifdef USE_DYNAMIC_BITSET
  S state_copy(state, 1);    
  mp[state_copy] = tick_value;
#else
	mp[state] = tick_value;
#endif
      } else {
	TickValue& to_tick_value = (*iter).second;
	to_tick_value.tm_slice += tick_value.tm_slice;
	if (COMPUTE_ERRORS) {
	  to_tick_value.tm_slice_square += tick_value.tm_slice_square;
	}
	to_tick_value.TH += tick_value.TH;
      }
    }
    
#ifdef MPI_COMPAT
    size_t my_MPI_Size() const {
      size_t res = sizeof(size_t);
      for (auto& elem: mp) {
        res += elem.first.my_MPI_Pack_Size() + 2*sizeof(double);
      }
      return res;
    }

    void my_MPI_Pack(void* buff, unsigned int size_pack, int* position) {
      size_t s_cumulMap = size();
      MPI_Pack(&s_cumulMap, 1, my_MPI_SIZE_T, buff, size_pack, position, MPI_COMM_WORLD);

      CumulMap::Iterator t_iterator = iterator();
      
      TickValue t_tick_value;
      while ( t_iterator.hasNext()) {

        const S& state = t_iterator.next2(t_tick_value);        
        
        state.my_MPI_Pack(buff, size_pack, position);

        MPI_Pack(&(t_tick_value.tm_slice), 1, MPI_DOUBLE, buff, size_pack, position, MPI_COMM_WORLD);
        MPI_Pack(&(t_tick_value.TH), 1, MPI_DOUBLE, buff, size_pack, position, MPI_COMM_WORLD);

      }   
    }

    void my_MPI_Unpack(void* buff, unsigned int buff_size, int* position) 
    {
      size_t s_cumulMap;
      MPI_Unpack(buff, buff_size, position, &s_cumulMap, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);

      for (size_t j=0; j < s_cumulMap; j++) {
        
        S t_state;
        t_state.my_MPI_Unpack(buff, buff_size, position);
     
        double t_tm_slice;
        MPI_Unpack(buff, buff_size, position, &t_tm_slice, 1, MPI_DOUBLE, MPI_COMM_WORLD);
     
        double t_TH;
        MPI_Unpack(buff, buff_size, position, &t_TH, 1, MPI_DOUBLE, MPI_COMM_WORLD);

        TickValue t_tick_value(t_tm_slice, t_TH);
        add(t_state, t_tick_value); 
      }

    }
    
    void my_MPI_Recv(int source) {
      
      size_t s_cumulMap;
      MPI_Recv(&s_cumulMap, 1, my_MPI_SIZE_T, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      for (size_t j=0; j < s_cumulMap; j++) {
        
        S t_state;
        t_state.my_MPI_Recv(source);
     
        double t_tm_slice;
        double t_TH;
        MPI_Recv(&t_tm_slice, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&t_TH, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        TickValue t_tick_value(t_tm_slice, t_TH);
        add(t_state, t_tick_value); 
      }
    }
    
    void my_MPI_Send(int dest) {
      
      size_t s_cumulMap = size();
      MPI_Send(&s_cumulMap, 1, my_MPI_SIZE_T, dest, 0, MPI_COMM_WORLD);

      CumulMap::Iterator t_iterator = iterator();
      
      TickValue t_tick_value;
      while ( t_iterator.hasNext()) {

        const S& state = t_iterator.next2(t_tick_value);
        state.my_MPI_Send(dest);
        MPI_Send(&(t_tick_value.tm_slice), 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
        MPI_Send(&(t_tick_value.TH), 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);

      }   
    }
#endif

    class Iterator {
    
      const CumulMap& cumul_map;
      typename STATE_MAP<S, TickValue>::const_iterator iter, end;

    public:
      Iterator(const CumulMap& cumul_map) : cumul_map(cumul_map) {
	iter = cumul_map.mp.begin();
	end = cumul_map.mp.end();
      }
	
      void rewind() {
	iter = cumul_map.mp.begin();
      }

      bool hasNext() const {
	return iter != end;
      }

      void next(S& state, TickValue& tick_value) {
	state = (*iter).first;
	tick_value = (*iter).second;
	++iter;
      }
	
      const S& next2(TickValue& tick_value) {
	tick_value = (*iter).second;
	return (*iter++).first;
      }
	
      void next(TickValue& tick_value) {
	tick_value = (*iter).second;
	++iter;
      }
    };

    Iterator iterator() {return Iterator(*this);}
    Iterator iterator() const {return Iterator(*this);}
  };
  class HDCumulMap {
    typename STATE_MAP<S, double> mp;

  public:
    size_t size() const {
      return mp.size();
    }

    void incr(const S& fullstate, double tm_slice) {
      auto iter = mp.find(fullstate);
      if (iter == mp.end()) {
#ifdef USE_DYNAMIC_BITSET
	S fullstate_copy(fullstate, 1);
  mp[fullstate_copy] = tm_slice;
#else
  mp[fullstate] = tm_slice;
#endif
      } else {
	(*iter).second += tm_slice;
      }
    }

    void add(const S& fullstate, double tm_slice) {
      auto iter = mp.find(fullstate);
      if (iter == mp.end()) {
#ifdef USE_DYNAMIC_BITSET
        S fullstate_copy(fullstate, 1);
        mp[fullstate_copy] = tm_slice;
#else
      	mp[fullstate] = tm_slice;
#endif
      } else {
	(*iter).second += tm_slice;
      }
    }

#ifdef MPI_COMPAT
    size_t my_MPI_Size() const {
      size_t res = sizeof(size_t);
      for (auto& elem: mp) {
        res += elem.first.my_MPI_Pack_Size() + sizeof(double);
      }
      return res;
    }
    
    void my_MPI_Pack(void* buff, unsigned int size_pack, int* position) 
    {
      size_t s_HDCumulMap = size();
      MPI_Pack(&s_HDCumulMap, 1, my_MPI_SIZE_T, buff, size_pack, position, MPI_COMM_WORLD);

      HDCumulMap::Iterator t_hd_iterator = iterator();
      
      double tm_slice;
      while ( t_hd_iterator.hasNext()) {

        const S& state = t_hd_iterator.next2(tm_slice);
        
        state.my_MPI_Pack(buff, size_pack, position);
        MPI_Pack(&tm_slice, 1, MPI_DOUBLE, buff, size_pack, position, MPI_COMM_WORLD);
      }
    }
    
    void my_MPI_Unpack(void* buff, unsigned int buff_size, int* position) 
    {
      // First we need the size
      size_t s_HDCumulMap;
      MPI_Unpack(buff, buff_size, position, &s_HDCumulMap, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
      
      for (size_t j=0; j < s_HDCumulMap; j++) {
        
        S t_state;
        t_state.my_MPI_Unpack(buff, buff_size, position);
      
        double t_tm_slice;
        MPI_Unpack(buff, buff_size, position, &t_tm_slice, 1, MPI_DOUBLE, MPI_COMM_WORLD);
      
        add(t_state, t_tm_slice);  
      }
    }
    
    void my_MPI_Recv(int source) {
      // First we need the size
      size_t s_HDCumulMap;
      MPI_Recv(&s_HDCumulMap, 1, my_MPI_SIZE_T, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      for (size_t j=0; j < s_HDCumulMap; j++) {
        
        S t_state;
        t_state.my_MPI_Recv(source);

        double t_tm_slice;
        MPI_Recv(&t_tm_slice, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        add(t_state, t_tm_slice);  
      }
    }
    
    void my_MPI_Send(int dest) {
      size_t s_HDCumulMap = size();
      MPI_Send(&s_HDCumulMap, 1, my_MPI_SIZE_T, dest, 0, MPI_COMM_WORLD);

      HDCumulMap::Iterator t_hd_iterator = iterator();
      
      // NetworkState t_state;
      double tm_slice;
      while ( t_hd_iterator.hasNext()) {

        const S& state = t_hd_iterator.next2(tm_slice);
        state.my_MPI_Send(dest);
        MPI_Send(&tm_slice, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);

      }
    }
    
#endif

    class Iterator {
    
      const HDCumulMap& hd_cumul_map;
      typename STATE_MAP<S, double>::const_iterator iter, end;

    public:
      Iterator(const HDCumulMap& hd_cumul_map) : hd_cumul_map(hd_cumul_map) {
	iter = hd_cumul_map.mp.begin();
	end = hd_cumul_map.mp.end();
      }
	
      void rewind() {
	iter = hd_cumul_map.mp.begin();
      }

      bool hasNext() const {
	return iter != end;
      }

      void next(S& state, double& tm_slice) {
	state = (*iter).first;
	tm_slice = (*iter).second;
	++iter;
      }
	
      const S& next2(double& tm_slice) {
	tm_slice = (*iter).second;
	return (*iter++).first;
      }

      void next(double& tm_slice) {
	tm_slice = (*iter).second;
	++iter;
      }
    };

    Iterator iterator() {return Iterator(*this);}
    Iterator iterator() const {return Iterator(*this);}
  };

  RunConfig* runconfig;
  double time_tick;
  unsigned int sample_count;
  unsigned int sample_num;
  double last_tm;
  int tick_index;
  std::vector<double> H_v;
  std::vector<double> TH_v;
  std::vector<MAP<unsigned int, double> > HD_v;
  std::vector<double> TH_square_v;
  unsigned int maxcols;
  bool isPopCumulator;
  unsigned int max_simplecols;
  int max_size;
  int max_tick_index;
  S output_mask;
  std::vector<CumulMap> cumul_map_v;
  std::vector<HDCumulMap> hd_cumul_map_v;
  unsigned int statdist_trajcount;
  unsigned int refnode_count;
  NetworkState refnode_mask;
  std::vector<ProbaDist<S> > proba_dist_v;
  ProbaDist<S> curtraj_proba_dist;
  STATE_MAP<S, LastTickValue> last_tick_map;
  std::map<unsigned int, unsigned int> output_scale;
  bool tick_completed;

  CumulMap& get_map() {
    assert((size_t)tick_index < cumul_map_v.size());
    return cumul_map_v[tick_index];
  }

  CumulMap& get_map(unsigned int nn) {
    assert(nn < cumul_map_v.size());
    return cumul_map_v[nn];
  }

  const CumulMap& get_map(unsigned int nn) const {
    assert(nn < cumul_map_v.size());
    return cumul_map_v[nn];
  }

  HDCumulMap& get_hd_map() {
    assert((size_t)tick_index < hd_cumul_map_v.size());
    return hd_cumul_map_v[tick_index];
  }

  HDCumulMap& get_hd_map(unsigned int nn) {
    assert(nn < hd_cumul_map_v.size());
    return hd_cumul_map_v[nn];
  }

  const HDCumulMap& get_hd_map(unsigned int nn) const {
    assert(nn < hd_cumul_map_v.size());
    return hd_cumul_map_v[nn];
  }

// #ifdef MPI_COMPAT

// static void MPI_Send_Cumulator(Cumulator* ret_cumul, int dest);
// static void MPI_Recv_Cumulator(Cumulator* mpi_ret_cumul, int origin);
// #endif
  double cumultime(int at_tick_index = -1) {
    if (at_tick_index < 0) {
      at_tick_index = tick_index;
    }
    return at_tick_index * time_tick;
  }

  bool incr(const S& state, double tm_slice, double TH, const S& fullstate) {
    if (tm_slice == 0.0) {
      return true;
    }

    if (sample_num < statdist_trajcount) {
      curtraj_proba_dist.incr(S(fullstate), tm_slice);
    }
    if (tick_index >= max_size) {
      return false;
    }
    tick_completed = false;
    CumulMap& mp = get_map();
    mp.incr(state, tm_slice, TH);

    HDCumulMap& hd_mp = get_hd_map();
    hd_mp.incr(fullstate, tm_slice);

    auto last_tick_iter = last_tick_map.find(state);
    if (last_tick_iter == last_tick_map.end()) {
#ifdef USE_DYNAMIC_BITSET
      S t_state(state, 1);
      last_tick_map[t_state] = LastTickValue(tm_slice, tm_slice * TH);
#else
      last_tick_map[state] = LastTickValue(tm_slice, tm_slice * TH);
#endif
    } else {
      (*last_tick_iter).second.tm_slice += tm_slice;
      (*last_tick_iter).second.TH += tm_slice * TH;
    }

    return true;
  }

  void check() const {
    // check that for each tick (except the last one), the sigma of each map == 1.
    for (int nn = 0; nn < max_tick_index; ++nn) {
    const CumulMap& mp = get_map(nn);
    auto iter = mp.iterator();
    double sum = 0.;
    while (iter.hasNext()) {
      TickValue tick_value;
      iter.next(tick_value);
      sum += tick_value.tm_slice;
    }
    sum /= time_tick*sample_count;
    assert(sum >= 1. - 0.0001 && sum <= 1. + 0.0001);
  }
  }

  void add(unsigned int where, const CumulMap& add_cumul_map) 
  {
    CumulMap& to_cumul_map = get_map(where);

    auto iter = add_cumul_map.iterator();
    while (iter.hasNext()) {
      TickValue tick_value;
      const S& state = iter.next2(tick_value);
      to_cumul_map.add(state, tick_value);
    }

  }
  void add(unsigned int where, const HDCumulMap& add_hd_cumul_map) 
  {
    HDCumulMap& to_hd_cumul_map = get_hd_map(where);

    auto iter = add_hd_cumul_map.iterator();
    while (iter.hasNext()) {
      double tm_slice;
      const S& state = iter.next2(tm_slice);
      to_hd_cumul_map.add(state, tm_slice);
    }

  }
  
public:

  Cumulator(RunConfig* runconfig, double time_tick, double max_time, unsigned int sample_count, unsigned int statdist_trajcount) :
    runconfig(runconfig), time_tick(time_tick), sample_count(sample_count), sample_num(0), last_tm(0.), tick_index(0), statdist_trajcount(statdist_trajcount) {
    output_mask.set();
    refnode_mask.reset();
    max_size = (int)(max_time/time_tick);
    max_tick_index = max_size;
    cumul_map_v.resize(max_size);
    hd_cumul_map_v.resize(max_size);
    if (COMPUTE_ERRORS) {
      TH_square_v.resize(max_size);
      for (int nn = 0; nn < max_size; ++nn) {
	TH_square_v[nn] = 0.;
      }
    }
    proba_dist_v.resize(statdist_trajcount);
    tick_completed = false;
    isPopCumulator = S::isPopState();
    if (isPopCumulator) {
      std::map<unsigned int, std::vector<int>> scale_values;
      double base = runconfig->getPopBase();
      for (unsigned int i = 1; i < 1024; i++) {
        int level;
        // std::cout << "base = " << base << std::endl;
        if (base > 1.001)
          level = (int) pow(base, (round(log(i)/log(base))));
        else  
          level = i;  
        // int level = pow(2, (ceil(log2(i))));
        output_scale[i] = level;
        if (scale_values.find(level) == scale_values.end()) {
          scale_values[level] = std::vector<int>();
        }
        scale_values[level].push_back(i);
        // std::cout << "output_scale[" << i << "] = " << output_scale[i] << std::endl;
      }
      std::map<unsigned int, unsigned int> replace;
      for (auto& scale_value: scale_values) {
        double mean = 0;
        // std::cout << "Values for " << scale_value.first << ": " << std::endl;
        for (auto& value: scale_value.second) {
          // std::cout << value << ", ";
          mean += value;
        }
        unsigned int imean = (unsigned int) round(mean/scale_value.second.size());
        // std::cout << "Mean: " << imean << std::endl;
        replace[scale_value.first] = imean;
      }
      for (auto& scale_value: output_scale) {
        // std::cout << "output_scale[" << scale_value.first << "] = " << scale_value.second << " => " << replace[scale_value.second] << std::endl;
        output_scale[scale_value.first] = replace[scale_value.second];
      }
    }
  }

  void rewind() {
    tick_index = 0;
    last_tm = 0.;
    last_tick_map.clear();
    curtraj_proba_dist.clear();
    tick_completed = false;
  }

  void next() {
    if (tick_index < max_size) {
      CumulMap& mp = get_map();
      double TH = 0.0;
      for (const auto & last_tick: last_tick_map)
      {
        const S& state = last_tick.first;
        double tm_slice = last_tick.second.tm_slice;
        TH += last_tick.second.TH;
        if (COMPUTE_ERRORS) {
          mp.cumulTMSliceSquare(state, tm_slice);
        }
      }
      if (COMPUTE_ERRORS) {
	      TH_square_v[tick_index] += TH * TH;
      }
    }
    ++tick_index;
    tick_completed = true;
    last_tick_map.clear();
  }

  void cumulEmpty(double tm) {
    double time_1 = cumultime(tick_index+1);
    if (tm < time_1) {
      last_tm = tm;
      return;
    }

    if (!tick_completed) {
      next();
    }

    for (; cumultime(tick_index+1) <= tm; next()) {
    }
    last_tm = tm;
  }

  void cumul(const S& network_state, double tm, double TH) { 
// #ifdef USE_DYNAMIC_BITSET
//     S fullstate(network_state & refnode_mask, 1);
// #else
//     S fullstate(network_state & refnode_mask);
// #endif    
    
    S fullstate = network_state & refnode_mask;
    S state = network_state.applyMask(output_mask, output_scale);

    double time_1 = cumultime(tick_index+1);
    if (tm < time_1) {
      incr(state, tm - last_tm, TH, fullstate);
      last_tm = tm;
      return;
    }

    if (!incr(state, time_1 - last_tm, TH, fullstate)) {
      last_tm = tm;
      return;
    }
    next();

    for (; cumultime(tick_index+1) <= tm; next()) {
      if (!incr(state, time_tick, TH, fullstate)) {
	last_tm = tm;
	return;
      }
    }
      
    incr(state, tm - cumultime(), TH, fullstate);
    last_tm = tm;
  }

  void setOutputMask(const S& _output_mask) {
    this->output_mask = _output_mask;
  }
  
  S getOutputMask() const {
    return output_mask;
  }
  
  void setRefnodeMask(const NetworkState_Impl& _refnode_mask) {
#ifdef USE_DYNAMIC_BITSET
    this->refnode_mask = NetworkState(_refnode_mask, 1);
#else
    this->refnode_mask = NetworkState(_refnode_mask);
#endif
  }

  void displayProbTraj(unsigned int _refnode_count, ProbTrajDisplayer<S>* displayer) const 
  {
    std::set<S> result_states = getStates();
    std::vector<S> list_states(result_states.begin(), result_states.end());
    
    std::set<NetworkState_Impl> result_simple_states = getSimpleStates();
    std::vector<NetworkState_Impl> list_simple_states(result_simple_states.begin(), result_simple_states.end());
    
    displayer->begin(COMPUTE_ERRORS, max_tick_index, maxcols, max_simplecols, _refnode_count, list_states, list_simple_states);

    double time_tick2 = time_tick * time_tick;
    double ratio = time_tick*sample_count;
    for (int nn = 0; nn < max_tick_index; ++nn) {
      displayer->beginTimeTick(nn*time_tick);
      // TH
      const CumulMap& mp = get_map(nn);
      auto iter = mp.iterator();
      displayer->setTH(TH_v[nn]);

      // ErrorTH
      //assert((size_t)nn < TH_square_v.size());
      if (COMPUTE_ERRORS) {
        double TH_square = TH_square_v[nn];
        double TH = TH_v[nn]; // == TH
        double variance_TH = (TH_square / ((sample_count-1) * time_tick2)) - (TH*TH*sample_count/(sample_count-1));
        double err_TH;
        double variance_TH_sample_count = variance_TH/sample_count;
        //assert(variance_TH > 0.0);
        if (variance_TH_sample_count >= 0.0) {
    err_TH = sqrt(variance_TH_sample_count);
        } else {
    err_TH = 0.;
        }
        displayer->setErrorTH(err_TH);
      }

      // H
      displayer->setH(H_v[nn]);

      std::string zero_hexfloat = fmthexdouble(0.0);
      // HD
      const MAP<unsigned int, double>& hd_m = HD_v[nn];
      for (unsigned int hd = 0; hd <= _refnode_count; ++hd) { 
        auto hd_m_iter = hd_m.find(hd);
        if (hd_m_iter != hd_m.end()) {
    displayer->setHD(hd, hd_m_iter->second);
        } else {
    displayer->setHD(hd, 0.);
        }
      }

      // Proba, ErrorProba
      while (iter.hasNext()) {
        TickValue tick_value;
        const S& state = iter.next2(tick_value); 
        double proba = tick_value.tm_slice / ratio;      
        if (COMPUTE_ERRORS) {
    double tm_slice_square = tick_value.tm_slice_square;
    double variance_proba = (tm_slice_square / ((sample_count-1) * time_tick2)) - (proba*proba*sample_count/(sample_count-1));
    double err_proba;
    double variance_proba_sample_count = variance_proba/sample_count;
    if (variance_proba_sample_count >= DBL_MIN) {
      err_proba = sqrt(variance_proba_sample_count);
    } else {
      err_proba = 0.;
    }
    displayer->addProba(state, proba, err_proba);
        } else {
    displayer->addProba(state, proba, 0.);
        }
      }
      displayer->endTimeTick();
    }
    displayer->end();
  }
  void displayStatDist(StatDistDisplayer* displayer) const {
    // should not be in cumulator, but somehwere in ProbaDist*

    // Probability distribution
    unsigned int statdist_traj_count = runconfig->getStatDistTrajCount();
    if (statdist_traj_count == 0) {
      return;
    }

    unsigned int t_max_size = 0;
    unsigned int cnt = 0;
    unsigned int proba_dist_size = (unsigned int) proba_dist_v.size();
    for (unsigned int nn = 0; nn < proba_dist_size; ++nn) {
      const ProbaDist<S>& proba_dist = proba_dist_v[nn];
      if (proba_dist.size() > t_max_size) {
        t_max_size = (unsigned int) proba_dist.size();
      }
      cnt++;
      if (cnt > statdist_traj_count) {
        break;
      }
    }

    displayer->begin(t_max_size, statdist_traj_count);
    cnt = 0;
    displayer->beginStatDistDisplay();
    for (unsigned int nn = 0; nn < proba_dist_size; ++nn) {
      const ProbaDist<S>& proba_dist = proba_dist_v[nn];
      displayer->beginStateProba(cnt+1);
      cnt++;

      proba_dist.display(displayer);
      displayer->endStateProba();
      if (cnt >= statdist_traj_count) {
        break;
      }
    }
    displayer->endStatDistDisplay();

    // should not be called from here, but from MaBestEngine
    ProbaDistClusterFactory* clusterFactory = new ProbaDistClusterFactory(proba_dist_v, statdist_traj_count);
    clusterFactory->makeClusters(runconfig);
    clusterFactory->display(displayer);
    clusterFactory->computeStationaryDistribution();
    clusterFactory->displayStationaryDistribution(displayer);
    displayer->end();
    delete clusterFactory;
  }


std::map<std::string, std::vector<double> > getVariablesTimecourse(std::map<std::string, Expression*> variables) const 
{  
  std::map<std::string, std::vector<double> > ret;
  for (auto& variable: variables)
    ret[variable.first] = std::vector<double>(getMaxTickIndex(),0.0);
  
  
  double ratio = time_tick*sample_count;
  for (int nn=0; nn < getMaxTickIndex(); nn++) 
  {      
    const CumulMap& mp = get_map(nn);
    auto iter = mp.iterator();
 
    while (iter.hasNext()) {
      TickValue tick_value;
      const S& state = iter.next2(tick_value);
      double proba = tick_value.tm_slice / ratio;
      
      for (auto& variable: variables)
        ret[variable.first][nn] += proba * variable.second->eval(state, time_tick*nn);

    }
  }
  
  return ret;
}  
  
std::set<S> getStates() const
{
  std::set<S> result_states;

  for (int nn=0; nn < getMaxTickIndex(); nn++) {
    const CumulMap& mp = get_map(nn);
    auto iter = mp.iterator();

    while (iter.hasNext()) {
      TickValue tick_value;
      const S& state = iter.next2(tick_value);
      result_states.insert(state);
    }
  }

  return result_states;
}

std::vector<S> getLastStates() const
{
  std::vector<S> result_states;

    const CumulMap& mp = get_map(getMaxTickIndex()-1);
    auto iter = mp.iterator();

    while (iter.hasNext()) {
      TickValue tick_value;
      const S& state = iter.next2(tick_value);
      result_states.push_back(state);
    }

  return result_states;
}


std::set<NetworkState_Impl> getSimpleStates() const
{
  std::set<NetworkState_Impl> result_states;

  if (isPopCumulator) {

    for (int nn=0; nn < getMaxTickIndex(); nn++) {
      const CumulMap& mp = get_map(nn);
      auto iter = mp.iterator();

      while (iter.hasNext()) {
        TickValue tick_value;
        const S& state = iter.next2(tick_value);
        std::set<NetworkState_Impl>* t_network_states = state.getNetworkStates();
        result_states.insert(t_network_states->begin(), t_network_states->end());
        delete t_network_states;
        
      }
    }
  }
  
  return result_states;
}

std::set<NetworkState_Impl> getSimpleLastStates() const
{
  std::set<NetworkState_Impl> result_states;

  if (isPopCumulator) {

    const CumulMap& mp = get_map(getMaxTickIndex()-1);
    auto iter = mp.iterator();

    while (iter.hasNext()) {
      TickValue tick_value;
      const S& state = iter.next2(tick_value);
      std::set<NetworkState_Impl>* t_network_states = state.getNetworkStates();
      result_states.insert(t_network_states->begin(), t_network_states->end());
      delete t_network_states;
      
    }
  }
  
  return result_states;
}

#ifdef PYTHON_API

PyObject* getNumpyStatesDists(Network* network) const 
{
  std::set<S> result_states = getStates();
  
  npy_intp dims[2] = {(npy_intp) getMaxTickIndex(), (npy_intp) result_states.size()};
  PyArrayObject* result = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 
  PyArrayObject* errors = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 

  std::vector<S> list_states(result_states.begin(), result_states.end());
  STATE_MAP<S, unsigned int> pos_states;
  for(unsigned int i=0; i < list_states.size(); i++) {
    pos_states[list_states[i]] = i;
  }

  double ratio = time_tick*sample_count;
  double time_tick2 = time_tick * time_tick;

  for (int nn=0; nn < getMaxTickIndex(); nn++) {
    const CumulMap& mp = get_map(nn);
    auto iter = mp.iterator();

    while (iter.hasNext()) {
      TickValue tick_value;
      const S& state = iter.next2(tick_value);
      double proba = tick_value.tm_slice / ratio;
      void* ptr = PyArray_GETPTR2(result, nn, pos_states[state]);
      PyArray_SETITEM(
        result, 
        (char*) ptr,
        PyFloat_FromDouble(proba)
      );
      
      if (COMPUTE_ERRORS) {
        double tm_slice_square = tick_value.tm_slice_square;
        double variance_proba = (tm_slice_square / ((sample_count-1) * time_tick2)) - (proba*proba*sample_count/(sample_count-1));
        double err_proba;
        double variance_proba_sample_count = variance_proba/sample_count;
        if (variance_proba_sample_count >= DBL_MIN) {
          err_proba = sqrt(variance_proba_sample_count);
        } else {
          err_proba = 0.;
        }
        void* ptr = PyArray_GETPTR2(errors, nn, pos_states[state]);
        PyArray_SETITEM(
          errors, 
          (char*) ptr,
          PyFloat_FromDouble(err_proba)
        );
      }
    }
  }
  PyObject* pylist_state = PyList_New(list_states.size());
  for (unsigned int i=0; i < list_states.size(); i++) {
    PyList_SetItem(
      pylist_state, i, 
      PyUnicode_FromString(list_states[i].getName(network).c_str())
    );
  }

  PyObject* timepoints = PyList_New(getMaxTickIndex());
  for (int i=0; i < getMaxTickIndex(); i++) {
    PyList_SetItem(timepoints, i, PyFloat_FromDouble(((double) i) * time_tick));
  }

  return PyTuple_Pack(4, PyArray_Return(result), timepoints, pylist_state, PyArray_Return(errors));
}


PyObject* getNumpyLastStatesDists(Network* network) const 
{
  std::vector<S> result_last_states = getLastStates();
  
  npy_intp dims[2] = {(npy_intp) 1, (npy_intp) result_last_states.size()};
  PyArrayObject* result = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 
  PyArrayObject* errors = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 

  STATE_MAP<S, unsigned int> pos_states;
  for(unsigned int i=0; i < result_last_states.size(); i++) {
    pos_states[result_last_states[i]] = i;
  }

  double ratio = time_tick*sample_count;
  double time_tick2 = time_tick * time_tick;

  const CumulMap& mp = get_map(getMaxTickIndex()-1);
  auto iter = mp.iterator();

  while (iter.hasNext()) {
    TickValue tick_value;
    const S& state = iter.next2(tick_value);
    double proba = tick_value.tm_slice / ratio;

    void* ptr = PyArray_GETPTR2(result, 0, pos_states[state]);
    PyArray_SETITEM(
      result, 
      (char*) ptr,
      PyFloat_FromDouble(proba)
    );
    
    if (COMPUTE_ERRORS) {
      double tm_slice_square = tick_value.tm_slice_square;
      double variance_proba = (tm_slice_square / ((sample_count-1) * time_tick2)) - (proba*proba*sample_count/(sample_count-1));
      double err_proba;
      double variance_proba_sample_count = variance_proba/sample_count;
      if (variance_proba_sample_count >= DBL_MIN) {
        err_proba = sqrt(variance_proba_sample_count);
      } else {
        err_proba = 0.;
      }
      void* ptr = PyArray_GETPTR2(errors, 0, pos_states[state]);
      PyArray_SETITEM(
        errors, 
        (char*) ptr,
        PyFloat_FromDouble(err_proba)
      );
    }
  }

  PyObject* pylist_state = PyList_New(result_last_states.size());
  for (unsigned int i=0; i < result_last_states.size(); i++) {
    PyList_SetItem(
      pylist_state, i, 
      PyUnicode_FromString(result_last_states[i].getName(network).c_str())
    );
  }

  PyObject* timepoints = PyList_New(1);
  PyList_SetItem(
    timepoints, 0, 
    PyFloat_FromDouble(
      (
        (double) (getMaxTickIndex()-1)
      )*time_tick
    )
  );

  return PyTuple_Pack(4, PyArray_Return(result), timepoints, pylist_state, PyArray_Return(errors));
}

std::vector<Node*> getNodes(Network* network) const {
  std::vector<Node*> result_nodes;

  for (auto node: network->getNodes()) {
    if (!node->isInternal())
      result_nodes.push_back(node);
  }
  return result_nodes;
}

PyObject* getNumpyNodesDists(Network* network, std::vector<Node*> output_nodes) const 
{
  if (output_nodes.size() == 0){
    output_nodes = getNodes(network);
  }
  
  npy_intp dims[2] = {(npy_intp) getMaxTickIndex(), (npy_intp) output_nodes.size()};
  PyArrayObject* result = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 
  PyArrayObject* errors = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 

  STATE_MAP<Node*, unsigned int> pos_nodes;
  for(unsigned int i=0; i < output_nodes.size(); i++) {
    pos_nodes[output_nodes[i]] = i;
  }

  double ratio = time_tick*sample_count;
  double time_tick2 = time_tick * time_tick;

  std::map<Node*, double> node_probas;
  std::map<Node*, double> node_errors;
      
  for (int nn=0; nn < getMaxTickIndex(); nn++) {
    
    for (auto node: output_nodes)
    {
      node_probas[node] = 0;
      node_errors[node] = 0;
    }
    
    const CumulMap& mp = get_map(nn);
    auto iter = mp.iterator();

    while (iter.hasNext()) {
      TickValue tick_value;
      const S& state = iter.next2(tick_value);
      double proba = tick_value.tm_slice / ratio;
      
      for (auto node: output_nodes) {
        
        if ((state).getNodeState(node)){
          node_probas[node] += proba;
          
          if (COMPUTE_ERRORS) {
            double tm_slice_square = tick_value.tm_slice_square;
            double variance_proba = (tm_slice_square / ((sample_count-1) * time_tick2)) - (proba*proba*sample_count/(sample_count-1));
            double err_proba;
            double variance_proba_sample_count = variance_proba/sample_count;
            if (variance_proba_sample_count >= DBL_MIN) {
              err_proba = sqrt(variance_proba_sample_count);
            } else {
              err_proba = 0.;
            }
            
            node_errors[node] += err_proba;
          }
           
        }
      }
    }
    
    for (auto node: output_nodes) 
    {
      void* ptr_val = PyArray_GETPTR2(result, nn, pos_nodes[node]);
      PyArray_SETITEM(
        result, 
        (char*) ptr_val,
        PyFloat_FromDouble(node_probas[node])
      );
      
      if (COMPUTE_ERRORS) {  
        void* ptr = PyArray_GETPTR2(errors, nn, pos_nodes[node]);
        PyArray_SETITEM(
          errors, 
          (char*) ptr,
          PyFloat_FromDouble(node_errors[node])
        );
      }
    }
  }
  PyObject* pylist_nodes = PyList_New(output_nodes.size());
  for (unsigned int i=0; i < output_nodes.size(); i++) {
    PyList_SetItem(
      pylist_nodes, i, 
      PyUnicode_FromString(output_nodes[i]->getLabel().c_str())
    );
  }

  PyObject* timepoints = PyList_New(getMaxTickIndex());
  for (int i=0; i < getMaxTickIndex(); i++) {
    PyList_SetItem(timepoints, i, PyFloat_FromDouble(((double) i) * time_tick));
  }

  return PyTuple_Pack(4, PyArray_Return(result), timepoints, pylist_nodes, PyArray_Return(errors));
}


PyObject* getNumpyLastNodesDists(Network* network, std::vector<Node*> output_nodes) const 
{
  if (output_nodes.size() == 0){
    output_nodes = getNodes(network);
  }
  
  npy_intp dims[2] = {(npy_intp) 1, (npy_intp) output_nodes.size()};
  PyArrayObject* result = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 
  PyArrayObject* errors = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 

  STATE_MAP<Node*, unsigned int> pos_nodes;
  for(unsigned int i=0; i < output_nodes.size(); i++) {
    pos_nodes[output_nodes[i]] = i;
  }

  double ratio = time_tick*sample_count;
  double time_tick2 = time_tick * time_tick;

  std::map<Node*, double> node_probas;
  std::map<Node*, double> node_errors;
  for (auto node: output_nodes)
  {
    node_probas[node] = 0;
    node_errors[node] = 0;
  }
  
  const CumulMap& mp = get_map(getMaxTickIndex()-1);
  auto iter = mp.iterator();

  while (iter.hasNext()) {
    TickValue tick_value;
    const S& state = iter.next2(tick_value);
    double proba = tick_value.tm_slice / ratio;
    
    for (auto node: output_nodes) {
      
      if ((state).getNodeState(node)){
        node_probas[node] += proba;
        
        if (COMPUTE_ERRORS) {
          double tm_slice_square = tick_value.tm_slice_square;
          double variance_proba = (tm_slice_square / ((sample_count-1) * time_tick2)) - (proba*proba*sample_count/(sample_count-1));
          double err_proba;
          double variance_proba_sample_count = variance_proba/sample_count;
          if (variance_proba_sample_count >= DBL_MIN) {
            err_proba = sqrt(variance_proba_sample_count);
          } else {
            err_proba = 0.;
          }
          
          node_errors[node] += err_proba;
        }
          
      }
    }
  }
    
  for (auto node: output_nodes) 
  {
    void* ptr_val = PyArray_GETPTR2(result, 0, pos_nodes[node]);
    PyArray_SETITEM(
      result, 
      (char*) ptr_val,
      PyFloat_FromDouble(node_probas[node])
    );
    
    if (COMPUTE_ERRORS) {  
      void* ptr = PyArray_GETPTR2(errors, 0, pos_nodes[node]);
      PyArray_SETITEM(
        errors, 
        (char*) ptr,
        PyFloat_FromDouble(node_errors[node])
      );
    }
  }

  PyObject* pylist_nodes = PyList_New(output_nodes.size());
  for (unsigned int i=0; i < output_nodes.size(); i++) {
    PyList_SetItem(
      pylist_nodes, i, 
      PyUnicode_FromString(output_nodes[i]->getLabel().c_str())
    );
  }

  PyObject* timepoints = PyList_New(1);
  PyList_SetItem(timepoints, 0, PyFloat_FromDouble(((double) (getMaxTickIndex()-1)) * time_tick));

  return PyTuple_Pack(4, PyArray_Return(result), timepoints, pylist_nodes, PyArray_Return(errors));
}

PyObject* getNumpySimpleStatesDists(Network* network) const 
{
  if (!isPopCumulator) { Py_RETURN_NONE; }
 
  std::set<NetworkState_Impl> result_states = getSimpleStates();
  
  npy_intp dims[2] = {(npy_intp) getMaxTickIndex(), (npy_intp) (1+result_states.size())};
  PyArrayObject* result = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 
  PyArrayObject* errors = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 

  std::vector<NetworkState_Impl> list_states(result_states.begin(), result_states.end());
  STATE_MAP<NetworkState_Impl, unsigned int> pos_states;
  for(unsigned int i=0; i < list_states.size(); i++) {
    pos_states[list_states[i]] = i+1;
  }

  double ratio = time_tick*sample_count;
  double time_tick2 = time_tick * time_tick;

  for (int nn=0; nn < getMaxTickIndex(); nn++) {
    const CumulMap& mp = get_map(nn);
    auto iter = mp.iterator();

    double pop = 0;
    std::map<NetworkState_Impl, double> network_state_probas;
    std::map<NetworkState_Impl, double> network_state_errors;
    std::map<unsigned int, double> pop_size_distrib;

    while (iter.hasNext()) {
      TickValue tick_value;
      const S& state = iter.next2(tick_value);
      double proba = tick_value.tm_slice / ratio;
      
      pop += proba * state.count(NULL);
      
      if (COMPUTE_ERRORS)
      {
        std::map<unsigned int, double>::iterator it = pop_size_distrib.find(state.count(NULL));
        if (it != pop_size_distrib.end()) 
        { 
          it->second += proba;
        }
        else 
        {
          pop_size_distrib[state.count(NULL)] = proba;
        }
      }
      
      for (const auto& network_state : state.getMap()) 
      {
        std::map<NetworkState_Impl, double>::iterator it = network_state_probas.find(network_state.first);
        if (it != network_state_probas.end())
        {
          it->second += proba * network_state.second;
        }
        else
        {
          network_state_probas[network_state.first] = proba * network_state.second;
        }
        
        if (COMPUTE_ERRORS)
        {
          double state_proba = proba;// * network_state.second;
          double tm_slice_square = tick_value.tm_slice_square;
          double variance_proba = (tm_slice_square / ((sample_count-1) * time_tick2)) - (state_proba*state_proba*sample_count/(sample_count-1));
          double err_proba;
          double variance_proba_sample_count = variance_proba/sample_count;
          if (variance_proba_sample_count >= DBL_MIN) {
            err_proba = sqrt(variance_proba_sample_count);
          } else {
            err_proba = 0.;
          }
          
          std::map<NetworkState_Impl, double>::iterator it = network_state_errors.find(network_state.first);
          if (it != network_state_errors.end())
          {
            it->second += err_proba;
          }
          else
          {
            network_state_errors[network_state.first] = err_proba;
          }
        }
      }
    }
     
    void* ptr = PyArray_GETPTR2(result, nn, 0);
    PyArray_SETITEM(
      result, 
      (char*) ptr,
      PyFloat_FromDouble(pop)
    );
     
    if (COMPUTE_ERRORS)
    {
      double network_state_variance = - pop*pop;
      // double network_state_entropy = 0;
        
      for (const auto &size_proba: pop_size_distrib) 
      {
        network_state_variance += size_proba.second * (size_proba.first * size_proba.first);
        // network_state_entropy -= log2(size_proba.second)*size_proba.second;
      }
      void* ptr = PyArray_GETPTR2(errors, nn, 0);
      PyArray_SETITEM(
        errors, 
        (char*) ptr,
        PyFloat_FromDouble(network_state_variance)
      );
    }
    
    for (const auto& network_state : network_state_probas) 
    {
      void* ptr = PyArray_GETPTR2(result, nn, pos_states[network_state.first]);
      PyArray_SETITEM(
        result, 
        (char*) ptr,
        PyFloat_FromDouble(network_state.second/pop)
      );
    
      if (COMPUTE_ERRORS) {      
        void* ptr = PyArray_GETPTR2(errors, nn, pos_states[network_state.first]);
        PyArray_SETITEM(
          errors, 
          (char*) ptr,
          PyFloat_FromDouble(network_state_errors[network_state.first])
        );
      }
    }
  }

  PyObject* pylist_state = PyList_New(list_states.size()+1);
  PyList_SetItem(
    pylist_state, 0, 
    PyUnicode_FromString("Population")
  );
  for (unsigned int i=0; i < list_states.size(); i++) {
    PyList_SetItem(
      pylist_state, i+1, 
      PyUnicode_FromString(NetworkState(list_states[i]).getName(network).c_str())
    );
  }
  
  PyObject* timepoints = PyList_New(getMaxTickIndex());
  for (int i=0; i < getMaxTickIndex(); i++) {
    PyList_SetItem(timepoints, i, PyFloat_FromDouble(((double) i) * time_tick));
  }

  return PyTuple_Pack(4, PyArray_Return(result), timepoints, pylist_state, PyArray_Return(errors));
}

PyObject* getNumpySimpleLastStatesDists(Network* network) const 
{
  if (!isPopCumulator) { Py_RETURN_NONE; }
 
  std::set<NetworkState_Impl> result_states = getSimpleLastStates();
  
  npy_intp dims[2] = {(npy_intp) 1, (npy_intp) (1+result_states.size())};
  PyArrayObject* result = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 
  PyArrayObject* errors = (PyArrayObject *) PyArray_ZEROS(2,dims,NPY_DOUBLE, 0); 

  std::vector<NetworkState_Impl> list_states(result_states.begin(), result_states.end());
  STATE_MAP<NetworkState_Impl, unsigned int> pos_states;
  for(unsigned int i=0; i < list_states.size(); i++) {
    pos_states[list_states[i]] = i+1;
  }

  double ratio = time_tick*sample_count;
  double time_tick2 = time_tick * time_tick;

  const CumulMap& mp = get_map(getMaxTickIndex()-1);
  auto iter = mp.iterator();

  double pop = 0;
  std::map<NetworkState_Impl, double> network_state_probas;
  std::map<NetworkState_Impl, double> network_state_errors;
  std::map<unsigned int, double> pop_size_distrib;

  while (iter.hasNext()) {
    TickValue tick_value;
    const S& state = iter.next2(tick_value);
    double proba = tick_value.tm_slice / ratio;
    
    pop += proba * state.count(NULL);
    
    if (COMPUTE_ERRORS)
    {
      std::map<unsigned int, double>::iterator it = pop_size_distrib.find(state.count(NULL));
      if (it != pop_size_distrib.end()) 
      { 
        it->second += proba;
      }
      else 
      {
        pop_size_distrib[state.count(NULL)] = proba;
      }
    }
    
    for (const auto& network_state : state.getMap()) 
    {
      std::map<NetworkState_Impl, double>::iterator it = network_state_probas.find(network_state.first);
      if (it != network_state_probas.end())
      {
        it->second += proba * network_state.second;
      }
      else
      {
        network_state_probas[network_state.first] = proba * network_state.second;
      }
      
      if (COMPUTE_ERRORS)
      {
        double state_proba = proba;// * network_state.second;
        double tm_slice_square = tick_value.tm_slice_square;
        double variance_proba = (tm_slice_square / ((sample_count-1) * time_tick2)) - (state_proba*state_proba*sample_count/(sample_count-1));
        double err_proba;
        double variance_proba_sample_count = variance_proba/sample_count;
        if (variance_proba_sample_count >= DBL_MIN) {
          err_proba = sqrt(variance_proba_sample_count);
        } else {
          err_proba = 0.;
        }
        
        std::map<NetworkState_Impl, double>::iterator it = network_state_errors.find(network_state.first);
        if (it != network_state_errors.end())
        {
          it->second += err_proba;
        }
        else
        {
          network_state_errors[network_state.first] = err_proba;
        }
      }
    }
  }
  
  void* ptr = PyArray_GETPTR2(result, 0, 0);
  PyArray_SETITEM(
    result, 
    (char*) ptr,
    PyFloat_FromDouble(pop)
  );
    
  if (COMPUTE_ERRORS)
  {
    double network_state_variance = - pop*pop;
    // double network_state_entropy = 0;
      
    for (const auto &size_proba: pop_size_distrib) 
    {
      network_state_variance += size_proba.second * (size_proba.first * size_proba.first);
      // network_state_entropy -= log2(size_proba.second)*size_proba.second;
    }
    void* ptr = PyArray_GETPTR2(errors, 0, 0);
    PyArray_SETITEM(
      errors, 
      (char*) ptr,
      PyFloat_FromDouble(network_state_variance)
    );
  }
  
  for (const auto& network_state : network_state_probas) 
  {
    void* ptr = PyArray_GETPTR2(result, 0, pos_states[network_state.first]);
    PyArray_SETITEM(
      result, 
      (char*) ptr,
      PyFloat_FromDouble(network_state.second/pop)
    );
  
    if (COMPUTE_ERRORS) {      
      void* ptr = PyArray_GETPTR2(errors, 0, pos_states[network_state.first]);
      PyArray_SETITEM(
        errors, 
        (char*) ptr,
        PyFloat_FromDouble(network_state_errors[network_state.first])
      );
    }
  }

  PyObject* pylist_state = PyList_New(list_states.size()+1);
  PyList_SetItem(
    pylist_state, 0, 
    PyUnicode_FromString("Population")
  );
  for (unsigned int i=0; i < list_states.size(); i++) {
    PyList_SetItem(
      pylist_state, i+1, 
      PyUnicode_FromString(NetworkState(list_states[i]).getName(network).c_str())
    );
  }
  
  PyObject* timepoints = PyList_New(1);
  PyList_SetItem(timepoints, 0, PyFloat_FromDouble(((double) (getMaxTickIndex()-1)) * time_tick));

  return PyTuple_Pack(4, PyArray_Return(result), timepoints, pylist_state, PyArray_Return(errors));
}

  
#endif

  double getFinalTime() const {
      return time_tick*(getMaxTickIndex()-1);

  }


  int getMaxTickIndex() const { return max_tick_index;} 

  void epilogue(Network* network, const NetworkState& reference_state) 
  {
    // compute H (Entropy), TH (Transition entropy) and HD (Hamming distance)
    H_v.resize(max_tick_index);
    TH_v.resize(max_tick_index);

    maxcols = 0;
    max_simplecols = 0;
    std::set<NetworkState_Impl> network_states;
    double ratio = time_tick * sample_count;
    for (int nn = 0; nn < max_tick_index; ++nn) { // time tick
      const CumulMap& mp = get_map(nn);
      auto iter = mp.iterator();
      H_v[nn] = 0.;
      TH_v[nn] = 0.;
      while (iter.hasNext()) {
        TickValue tick_value;
        const S &state = iter.next2(tick_value);
        if (isPopCumulator) {
          std::set<NetworkState_Impl>* t_network_states = state.getNetworkStates();
          network_states.insert(t_network_states->begin(), t_network_states->end());
          delete t_network_states;
        }
        double tm_slice = tick_value.tm_slice;
        double proba = tm_slice / ratio;      
        double TH = tick_value.TH / sample_count;
        H_v[nn] += -log2(proba) * proba;
        TH_v[nn] += TH;
      }
      TH_v[nn] /= time_tick;
      if (mp.size() > maxcols) {
        maxcols = (unsigned int) mp.size();
      }
      if (network_states.size() > max_simplecols) {
        max_simplecols = (unsigned int) network_states.size();
      }
      
      network_states.clear();
    }
    
    HD_v.resize(max_tick_index);

    for (int nn = 0; nn < max_tick_index; ++nn) { // time tick
      const HDCumulMap& hd_mp = get_hd_map(nn);
      auto iter = hd_mp.iterator();
      MAP<unsigned int, double>& hd_m = HD_v[nn];
      while (iter.hasNext()) {
        double tm_slice;
        const S &state = iter.next2(tm_slice);
        double proba = tm_slice / ratio;      
        int hd = state.hamming(network, reference_state);
        if (hd_m.find(hd) == hd_m.end()) {
    hd_m[hd] = proba;
        } else {
    hd_m[hd] += proba;
        }
      }
    }
  }
  
  void trajectoryEpilogue() 
  {
    assert(sample_num < sample_count);

    if (sample_num < statdist_trajcount) 
    {
      auto curtraj_proba_dist_iter = curtraj_proba_dist.iterator();

      double proba_max_time = 0.;

      while (curtraj_proba_dist_iter.hasNext()) {
        double tm_slice;
        curtraj_proba_dist_iter.next(tm_slice);
        proba_max_time += tm_slice;
      }

      //std::cout << "Trajepilogue #" << (sample_num+1) << " " << proba_max_time << '\n';
#ifdef DEBUG
      double proba = 0;
#endif
      curtraj_proba_dist_iter.rewind();

      ProbaDist<S>& proba_dist = proba_dist_v[sample_num++];
      while (curtraj_proba_dist_iter.hasNext()) {
        S state;
        double tm_slice;
        curtraj_proba_dist_iter.next(state, tm_slice);
        //assert(proba_dist.find(state) == proba_dist.end());
        double new_tm_slice = tm_slice / proba_max_time;
        proba_dist.set(state, new_tm_slice);
#ifdef DEBUG      
        proba += new_tm_slice;
#endif
      }
#ifdef DEBUG      
      assert(proba >= 0.9999 && proba <= 1.0001);
#endif
    }
  }

  unsigned int getSampleCount() const {return sample_count;}

static void mergePairOfCumulators(Cumulator<S>* cumulator_1, Cumulator<S>* cumulator_2) {
    
  cumulator_1->sample_count += cumulator_2->sample_count;
  size_t rr = cumulator_1->proba_dist_v.size();
  cumulator_1->statdist_trajcount += cumulator_2->statdist_trajcount;
  cumulator_1->proba_dist_v.resize(cumulator_1->statdist_trajcount);
  
  if (cumulator_2->cumul_map_v.size() > cumulator_1->cumul_map_v.size()) {
    cumulator_1->cumul_map_v.resize(cumulator_2->cumul_map_v.size());
    cumulator_1->hd_cumul_map_v.resize(cumulator_2->cumul_map_v.size());
  }
  if (cumulator_2->max_tick_index > cumulator_1->max_tick_index) {
    cumulator_1->max_tick_index = cumulator_1->tick_index = cumulator_2->max_tick_index;
  }

  size_t t_cumul_size = cumulator_2->cumul_map_v.size();
  for (unsigned int nn = 0; nn < t_cumul_size; ++nn) {
    size_t index = t_cumul_size-nn-1;
    cumulator_1->add((unsigned int) index, cumulator_2->cumul_map_v[index]);
    cumulator_2->cumul_map_v.pop_back();
    cumulator_1->add((unsigned int) index, cumulator_2->hd_cumul_map_v[index]);
    cumulator_2->hd_cumul_map_v.pop_back();
    cumulator_1->TH_square_v[index] += cumulator_2->TH_square_v[index];
    cumulator_2->TH_square_v.pop_back();
  }
  
  size_t proba_dist_size = cumulator_2->proba_dist_v.size();
  for (size_t ii = 0; ii < proba_dist_size; ++ii) {
    assert(cumulator_1->proba_dist_v.size() > rr);
    cumulator_1->proba_dist_v[rr++] = cumulator_2->proba_dist_v[ii];
  }
  delete cumulator_2;
  cumulator_2 = NULL;
}

#ifdef MPI_COMPAT
static size_t MPI_Size_Cumulator(Cumulator<S>* ret_cumul)
{
  size_t total_size = sizeof(size_t);
  size_t t_cumul_size = ret_cumul != NULL ? ret_cumul->cumul_map_v.size() : 0;
  
  
  for (size_t nn = 0; nn < t_cumul_size; ++nn) {
    
    total_size += ret_cumul->get_map(nn).my_MPI_Size();

    total_size += ret_cumul->get_hd_map(nn).my_MPI_Size();
    
    total_size += sizeof(double);  
  }
  
  total_size += sizeof(size_t);
  size_t t_proba_dist_size = ret_cumul != NULL ? ret_cumul->proba_dist_v.size() : 0;
  
  for (size_t ii = 0; ii < t_proba_dist_size; ii++) {
    total_size += ret_cumul->proba_dist_v[ii].my_MPI_Size();
  }
  return total_size;
}

static char* MPI_Pack_Cumulator(Cumulator<S>* ret_cumul, int dest, unsigned int * buff_size) 
{
  *buff_size = MPI_Size_Cumulator(ret_cumul);
  char* buff = new char[*buff_size];
  int position = 0;
  size_t t_cumul_size = ret_cumul != NULL ? ret_cumul->cumul_map_v.size() : 0;
  MPI_Pack(&t_cumul_size, 1, my_MPI_SIZE_T, buff, *buff_size, &position, MPI_COMM_WORLD);
  
  for (size_t nn = 0; nn < t_cumul_size; ++nn) {
    
    ret_cumul->get_map(nn).my_MPI_Pack(buff, *buff_size, &position);

    ret_cumul->get_hd_map(nn).my_MPI_Pack(buff, *buff_size, &position);
      
    double t_th_square = ret_cumul->TH_square_v[nn];
    MPI_Pack(&t_th_square, 1, MPI_DOUBLE, buff, *buff_size, &position, MPI_COMM_WORLD);
  }
  
  size_t t_proba_dist_size = ret_cumul != NULL ? ret_cumul->proba_dist_v.size() : 0;
  MPI_Pack(&t_proba_dist_size, 1, my_MPI_SIZE_T, buff, *buff_size, &position, MPI_COMM_WORLD);

  for (size_t ii = 0; ii < t_proba_dist_size; ii++) {
    ret_cumul->proba_dist_v[ii].my_MPI_Pack(buff, *buff_size, &position);
  }
  
  
  return buff;
}

static void MPI_Unpack_Cumulator(Cumulator<S>* mpi_ret_cumul, char* buff, unsigned int buff_size )
{
  size_t t_cumul_size;
  int position = 0;
  MPI_Unpack(buff, buff_size, &position, &t_cumul_size, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);

  for (size_t nn = 0; nn < t_cumul_size; ++nn) {

    CumulMap t_cumulMap;
    t_cumulMap.my_MPI_Unpack(buff, buff_size, &position);  
    mpi_ret_cumul->add(nn, t_cumulMap);

    HDCumulMap t_HDCumulMap;
    t_HDCumulMap.my_MPI_Unpack(buff, buff_size, &position);
    mpi_ret_cumul->add(nn, t_HDCumulMap);
  
    double t_th_square;
    MPI_Unpack(buff, buff_size, &position, &t_th_square, 1, MPI_DOUBLE, MPI_COMM_WORLD);
    mpi_ret_cumul->TH_square_v.push_back(t_th_square);
  }
  
  size_t t_proba_dist_size;
  MPI_Unpack(buff, buff_size, &position, &t_proba_dist_size, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
  
  size_t begin = mpi_ret_cumul->statdist_trajcount - t_proba_dist_size;
  for (size_t ii = 0; ii < t_proba_dist_size; ii++) {
    ProbaDist<S> t_proba_dist;
    t_proba_dist.my_MPI_Unpack(buff, buff_size, &position);
    mpi_ret_cumul->proba_dist_v[begin + ii] = t_proba_dist;
  }    
}


static void MPI_Send_Cumulator(Cumulator<S>* ret_cumul, int dest) 
{
  size_t t_cumul_size = ret_cumul != NULL ? ret_cumul->cumul_map_v.size() : 0;
  MPI_Send(&t_cumul_size, 1, my_MPI_SIZE_T, dest, 0, MPI_COMM_WORLD);

  for (size_t nn = 0; nn < t_cumul_size; ++nn) {
    
    ret_cumul->get_map(nn).my_MPI_Send(dest);
    ret_cumul->get_hd_map(nn).my_MPI_Send(dest);
      
    double t_th_square = ret_cumul->TH_square_v[nn];
    MPI_Send(&t_th_square, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
  }
  
  size_t t_proba_dist_size = ret_cumul != NULL ? ret_cumul->proba_dist_v.size() : 0;
  MPI_Send(&t_proba_dist_size, 1, my_MPI_SIZE_T, dest, 0, MPI_COMM_WORLD);
  
  for (size_t ii = 0; ii < t_proba_dist_size; ii++) {
    ret_cumul->proba_dist_v[ii].my_MPI_Send(dest);
  }          
}

static void MPI_Recv_Cumulator(Cumulator<S>* mpi_ret_cumul, int origin) 
{
  size_t t_cumul_size;
  MPI_Recv(&t_cumul_size, 1, my_MPI_SIZE_T, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  for (size_t nn = 0; nn < t_cumul_size; ++nn) {

    CumulMap t_cumulMap;
    t_cumulMap.my_MPI_Recv(origin);  
    mpi_ret_cumul->add(nn, t_cumulMap);

    HDCumulMap t_HDCumulMap;
    t_HDCumulMap.my_MPI_Recv(origin);
    mpi_ret_cumul->add(nn, t_HDCumulMap);
  
    double t_th_square;
    MPI_Recv(&t_th_square, 1, MPI_DOUBLE, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    mpi_ret_cumul->TH_square_v.push_back(t_th_square);
  }
  
  size_t t_proba_dist_size;
  MPI_Recv(&t_proba_dist_size, 1, my_MPI_SIZE_T, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  size_t begin = mpi_ret_cumul->statdist_trajcount - t_proba_dist_size;

  for (size_t ii = 0; ii < t_proba_dist_size; ii++) {
    // Here we are receiving the proba_dist, which is a map of <state, double>
    ProbaDist<S> t_proba_dist;
    t_proba_dist.my_MPI_Recv(origin);
    mpi_ret_cumul->proba_dist_v[begin+ii] = t_proba_dist;  
  }   
}

static void mergePairOfMPICumulators(Cumulator<S>* ret_cumul, int world_rank, int dest, int origin, RunConfig* runconfig, bool pack) 
{
  if (world_rank == dest) {
        
    unsigned int other_cumulator_size;
    MPI_Recv( &other_cumulator_size, 1, MPI_UNSIGNED, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    unsigned int other_cumulator_statdist;
    MPI_Recv( &other_cumulator_statdist, 1, MPI_UNSIGNED, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (ret_cumul != NULL) {
      ret_cumul->sample_count += other_cumulator_size;
      ret_cumul->statdist_trajcount += other_cumulator_statdist;
      ret_cumul->proba_dist_v.resize(ret_cumul->statdist_trajcount);
      
    } else {
      ret_cumul = new Cumulator<S>(runconfig, runconfig->getTimeTick(), runconfig->getMaxTime(), other_cumulator_size, other_cumulator_statdist);
    }
    
    size_t remote_cumul_size;
    MPI_Recv( &remote_cumul_size, 1, my_MPI_SIZE_T, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int remote_max_tick_index;
    MPI_Recv( &remote_max_tick_index, 1, MPI_INT, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (remote_cumul_size > ret_cumul->cumul_map_v.size()) {
      ret_cumul->cumul_map_v.resize(remote_cumul_size);
      ret_cumul->hd_cumul_map_v.resize(remote_cumul_size);
    }
    
    if (remote_max_tick_index > ret_cumul->max_tick_index) {
      ret_cumul->max_tick_index = ret_cumul->tick_index = remote_max_tick_index;
    }
    
    if (pack) {
      unsigned int buff_size;
      MPI_Recv( &buff_size, 1, MPI_UNSIGNED, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      char* buff = new char[buff_size];
      MPI_Recv( buff, buff_size, MPI_PACKED, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
      
      MPI_Unpack_Cumulator(ret_cumul, buff, buff_size);
      delete [] buff;
      
    } else {
      MPI_Recv_Cumulator(ret_cumul, origin);
    }
    
  } else if (world_rank == origin) {
        
    unsigned int local_cumulator_size = ret_cumul != NULL ? ret_cumul->sample_count : 0;
    MPI_Send(&local_cumulator_size, 1, MPI_UNSIGNED, dest, 0, MPI_COMM_WORLD);
    
    unsigned int local_statdist_trajcount = ret_cumul != NULL ? ret_cumul->statdist_trajcount : 0;
    MPI_Send(&local_statdist_trajcount, 1, MPI_UNSIGNED, dest, 0, MPI_COMM_WORLD);
    
    size_t local_cumul_size = ret_cumul != NULL ? ret_cumul->cumul_map_v.size() : 0;
    MPI_Send(&local_cumul_size, 1, my_MPI_SIZE_T, dest, 0, MPI_COMM_WORLD);

    int local_max_tick_index = ret_cumul != NULL ? ret_cumul->max_tick_index : 0;
    MPI_Send(&local_max_tick_index, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);

    if (pack) {

      unsigned int buff_size;
      char* buff = MPI_Pack_Cumulator(ret_cumul, 0, &buff_size);
      MPI_Send(&buff_size, 1, MPI_UNSIGNED, dest, 0, MPI_COMM_WORLD);
      MPI_Send( buff, buff_size, MPI_PACKED, dest, 0, MPI_COMM_WORLD); 
      delete [] buff;
      
    } else {
     
      MPI_Send_Cumulator(ret_cumul, dest);
    }
  }
}

#endif

};

#endif
