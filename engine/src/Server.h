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
     Server.h
     
     Authors:
     Eric Viara <viara@sysra.com>
     
     Date:
     May 2018
*/

#ifndef _SERVER_H_
#define _SERVER_H_

#include <string>

#include "RPC.h"
#include "Utils.h"
#include "ServerData.h"
#include "DataStreamer.h"
#include "RequestController.h"

class ClientData;

class Server : public rpc_Server {
  std::string prog;
  std::string pidfile;
  bool quiet;
  bool verbose;
  RequestController* requestController;
  static Server* server;

  Server(const std::string& host, const std::string& port, const std::string& prog, const std::string& pidfile, bool quiet = false, bool verbose = false) : rpc_Server(host, port), prog(prog), pidfile(pidfile), quiet(verbose), verbose(verbose) { }

  void run(const ClientData& client_data, ServerData& server_data);
  void manageRequestPerform(int fd, const std::string& request, const std::vector<DataStreamer::HeaderItem>& headerItems, const std::string& data);

public:
  static Server* getServer(unsigned int max_queue_size, unsigned int max_cores_size, unsigned int cores_per_job, const std::string& host, const std::string& port, const std::string& prog, const std::string& pidfile = "", bool quiet = false, bool verbose = false) {
    if (NULL == server) {
      server = new Server(host, port, prog, pidfile, quiet, verbose);
      server->setRequestcontroller(new RequestController(server, max_queue_size, max_cores_size, cores_per_job));
    }
    return server;
  }

  static Server* getInstance() {
    return server;
  }

  void setRequestcontroller(RequestController* requestController) {
    this->requestController = requestController;
  }

  void setQuiet(bool quiet) {this->quiet = quiet;}
  void setVerbose(bool verbose) {this->verbose = verbose;}

  const std::string& getPidFile() const {return pidfile;}

  RequestController* getRequestcontroller() {return requestController;}
  
  int manageRequests();
  void manageRequest(int fd, const std::string& request);
  void launchJob(Job* job);
};

#endif
