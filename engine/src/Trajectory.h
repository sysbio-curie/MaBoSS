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
     Trajectory.h

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
   July 2018
*/

#pragma once

#include "NetworkState.h"

namespace MaBoSS {
  
// A recorded trajectory, as the successive states it went through. Each segment
// keeps its state and the time it was left: it starts where the previous one
// ended (at 0 for the first one), so start times are not stored. Consecutive
// segments always hold different states.
class Trajectory {
  std::vector<std::pair<double, NetworkState_Impl>> segments;

public:
  void add(double end_time, const NetworkState& state) {
    if (!segments.empty() && segments.back().second == state.getState()) {
      segments.back().first = end_time;
      return;
    }
#ifdef USE_DYNAMIC_BITSET
    // copies of a dynamic bitset share its storage, and the simulation flips
    // network_state in place
    segments.emplace_back(end_time, state.getState(1));
#else
    segments.emplace_back(end_time, state.getState());
#endif
  }

  const std::vector<std::pair<double, NetworkState_Impl>>& getSegments() const { return segments; }
  
  const std::vector<std::pair<double, NetworkState_Impl>> getMaskedSegments(const NetworkState& mask) const 
  { 
    std::vector<std::pair<double, NetworkState_Impl>> masked;
    for (const auto& segment : segments) {
      NetworkState_Impl state = (NetworkState(segment.second) & mask).getState();
      if (!masked.empty() && masked.back().second == state) {
        masked.back().first = segment.first;
      } else {
        masked.emplace_back(segment.first, state);
      }
    }

    return masked;
  }
};

}