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
     Job.h
     
     Authors:
     Eric Viara <viara@sysra.com>
     
     Date:
     Feb 2024
*/

#ifndef _JOB_H_
#define _JOB_H_

#include "DataStreamer.h"
#include "TokenGenerator.h"

class JobStatus {

public:
  enum Status {
    QUEUED = 1,
    KILLED,
    RUNNING,
    TERMINATED
  } status;

  JobStatus(Status status) {
    this->status = status;
  }

  std::string toString() const;
};

std::ostream& operator<<(std::ostream& os, JobStatus status);

class Job {
  int fd;
  std::string request;
  const std::vector<DataStreamer::HeaderItem> headerItems;
  const std::string data;
  std::string token;
  pid_t pid;
  JobStatus status;
  time_t ts_submitted;
  
public:
  Job(int fd, const std::string& request, const std::vector<DataStreamer::HeaderItem>& headerItems, const std::string& data, const std::string& token) : request(request), headerItems(headerItems), data(data), token(token), status(JobStatus::QUEUED) {
    this->fd = fd;
    pid = 0;
    time(&ts_submitted);
  }

  int getFd() const {return fd;}
  const std::string& getRequest() const {return request;}
  const std::vector<DataStreamer::HeaderItem>& getHeaderItems() const {return headerItems;}
  const std::string& getData() const {return data;}

  const std::string& getToken() const {return token;}
  void setPid(pid_t pid) {this->pid = pid;}
  void setStatus(JobStatus status) {this->status = status;}
  JobStatus getStatus() const {return status;}
};

class JobQueue {
  std::vector<Job*> jobQueue;

public:
  JobQueue();

  Job* addJob(int fd, const std::string& request, const std::vector<DataStreamer::HeaderItem>& headerItems, const std::string& data) {
    Job* job = new Job(fd, request, headerItems, data, TokenGenerator::getTokenGenerator()->generateJobToken());
    jobQueue.push_back(job);
    return job;
  }

  size_t getQueueSize() const {return jobQueue.size();}
};

#endif
