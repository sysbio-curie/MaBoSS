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
     RequestController.h
     
     Authors:
     Eric Viara <viara@sysra.com>
     
     Date:
     Feb 2024
*/

#ifndef _REQUESTCONTROLLER_H_
#define _REQUESTCONTROLLER_H_

#include <vector>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

#include "TokenGenerator.h"
#include "DataStreamer.h"
#include "Job.h"

class Server;

class RequestController {

  Server* server;
  JobQueue* jobQueue;
  unsigned int queue_size;
  unsigned int max_cores;
  unsigned int cores_per_job;
  int sendErrorData(int fd, int status, const std::string& err_data);

public:
  static const unsigned int QUEUE_SIZE_DEFAULT = 100;
  static const unsigned int MAX_CORES_DEFAULT = 6;
  static const unsigned int CORES_PER_JOB_DEFAULT = 6;

  RequestController(Server* server, unsigned int queue_size, unsigned int max_cores, unsigned int cores_per_job) {
    this->server = server;
    this->queue_size = queue_size != 0 ? queue_size : QUEUE_SIZE_DEFAULT;
    this->max_cores = max_cores != 0 ? max_cores : MAX_CORES_DEFAULT;
    this->cores_per_job = cores_per_job != 0 ? cores_per_job : CORES_PER_JOB_DEFAULT;

    this->jobQueue = new JobQueue();
  }

  int manageRequest(int fd, const std::string& request);
};
  
#endif
