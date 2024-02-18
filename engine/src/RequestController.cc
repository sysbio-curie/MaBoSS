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
     RequestController.cc

   Authors:
     Eric Viara <viara@sysra.com>
 
   Date:
     Feb 2024
*/

#include <sstream>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>

#include "Server.h"
#include "DataStreamer.h"
#include "RequestController.h"

int RequestController::sendErrorData(int fd, int status, const std::string& err_data)
{
  ServerData server_data;
  server_data.setStatus(status);
  server_data.setErrorMessage(err_data);
  std::string data;
  DataStreamer::buildStreamData(data, server_data);
  rpc_writeStringData(fd, data.c_str(), data.length());
  return 1;
}

int RequestController::manageRequest(int fd, const std::string& request)
{
  // 1. must check that queue is not full, if yes returns an error
  /* // for instance:
  if (true) {
    return sendErrorData(fd, 4, "MaBoSS-server: too many jobs in queue");
  }
  */
  // 2. then
  std::string header;
  std::string data;
  std::string err_data;
  int status = DataStreamer::parseStreamDataBegin(request, err_data, header, data);
  if (status) {
    return sendErrorData(fd, status, err_data);
  }
  std::cerr << "RequestController Header [" << header << "]\n";
  std::vector<DataStreamer::HeaderItem> headerItems;
  if (DataStreamer::parseHeaderItems(header, headerItems, err_data)) {
    return sendErrorData(fd, 3, err_data);
  }

  // at this step, parseStreamDataBegin() and parseHeaderItems have already been called -> to be passed to Server::manageRequest ? Yes, it called. But if job is queued, one need to store this information (data, headerItems) in Job... why not
  // actually, full parsing will be better => build client_data and associate it to the job:
  //Job* job = jobQueue->addJob(fd, request, client_data);
  // no ! because at this step, scanning headerItems is different:
  // - we can ignore flats, network, configuration expressions and variables
  // - we need to manage control commands as well as MODE (batch or fetch_result)
  Job* job = jobQueue->addJob(fd, request, headerItems, data);

  /*
  std::string s_request = request;
  size_t pos = s_request.find("\n\n");
  std::string header = s_request.substr(0, pos-1);
  */
  // no MABOSS_CONTROL_MAGIC, use Command directive instead:
  // Token: <admin_or_job_token>
  // admin: ShowQueue, ShowQueueDetails, KillJobs (needs JobList directive), EmptyQueue
  // client: JobStatus, KillJob
  // other directives:
  // Mode: Batch or FetchResult/WaitForResult
  // JobList: <comma_sep_list_of_tokens>
  /*
  if (s_request.substr(0, DataStreamer::MABOSS_CONTROL_MAGIC.length()) == DataStreamer::MABOSS_CONTROL_MAGIC) {
  } else {
  }
  */
  

  server->launchJob(job);
  /*
  pid_t pid;
  if ((pid = fork()) == 0) {
    server->manageRequestPerform(fd, request, headerItems, data);
    //free(request);
    close(fd);
    exit(0);
  }

  job->setPid(pid);
  job->setStatus(JobStatus::RUNNING);
  std::cerr << "Job Token [" << job->getToken() << "] Pid [" << pid << "] Status [" << job->getStatus() << "]\n";
  */
  return 0;
}

