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
     DataStreamer.cc

   Authors:
     Eric Viara <viara@sysra.com>
 
   Date:
     May 2018
*/

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "DataStreamer.h"
#include "Utils.h"

const std::string DataStreamer::PROTOCOL_VERSION_NUMBER = "1.0";

const std::string DataStreamer::MABOSS_MAGIC = "MaBoSS-2.0";
const std::string DataStreamer::PROTOCOL_VERSION = "Protocol-Version:";
const std::string DataStreamer::FLAGS = "Flags:";
const unsigned long long DataStreamer::HEXFLOAT_FLAG = 0x1ULL;
const unsigned long long DataStreamer::OVERRIDE_FLAG = 0x2ULL;
const unsigned long long DataStreamer::AUGMENT_FLAG = 0x4ULL;
const unsigned long long DataStreamer::FINAL_SIMULATION_FLAG = 0x1000ULL;

// commands
const std::string DataStreamer::COMMAND = "Command:";
const std::string DataStreamer::RUN_COMMAND = "run";
const std::string DataStreamer::CHECK_COMMAND = "check";
const std::string DataStreamer::PARSE_COMMAND = "parse";
// admin control commmands
const std::string DataStreamer::SHOW_QUEUE_COMMAND = "showQueue";
const std::string DataStreamer::SHOW_QUEUE_DETAILS_COMMAND = "showQueueDetails";
const std::string DataStreamer::EMPTY_QUEUE_COMMAND = "emptyQueue";
const std::string DataStreamer::KILL_JOBS_COMMAND = "killJobs";
// client control commmands
const std::string DataStreamer::JOB_STATUS_COMMAND = "jobStatus";
const std::string DataStreamer::KILL_JOB_COMMAND = "killJob";

const std::string DataStreamer::JOB_LIST = "JobList";
const std::string DataStreamer::TOKEN = "Token";

const std::string DataStreamer::MODE = "Mode";
const std::string DataStreamer::BATCH = "batch";
const std::string DataStreamer::FETCH_RESULT = "fetchResult";

const std::string DataStreamer::NETWORK = "Network:";
const std::string DataStreamer::CONFIGURATION = "Configuration:";
const std::string DataStreamer::CONFIGURATION_EXPRESSIONS = "Configuration-Expressions:";
const std::string DataStreamer::CONFIGURATION_VARIABLES = "Configuration-Variables:";
const std::string DataStreamer::RETURN = "RETURN";
const std::string DataStreamer::STATUS = "Status:";
const std::string DataStreamer::ERROR_MESSAGE = "Error-Message:";
const std::string DataStreamer::STATIONARY_DISTRIBUTION = "Stationary-Distribution:";
const std::string DataStreamer::TRAJECTORY_PROBABILITY = "Trajectory-Probability:";
const std::string DataStreamer::TRAJECTORIES = "Trajectories:";
const std::string DataStreamer::FIXED_POINTS = "Fixed-Points:";
const std::string DataStreamer::FINAL_PROB = "Final-States:";
const std::string DataStreamer::RUN_LOG = "Run-Log:";

static size_t add_header(std::ostringstream& o_header, const std::string& directive, size_t o_offset, size_t offset)
{
  if (o_offset != offset) {
    o_header << directive << o_offset << "-" << (offset-1) << '\n';
  }
  return offset;
}

void DataStreamer::buildStreamData(std::string& data, const ClientData& client_data)
{
  std::ostringstream o_header;
  std::ostringstream o_data;
  size_t offset = 0;
  size_t o_offset = offset;

  o_header << MABOSS_MAGIC << "\n";
  o_header << PROTOCOL_VERSION << PROTOCOL_VERSION_NUMBER << "\n";
  o_header << FLAGS << client_data.getFlags() << "\n";
  o_header << COMMAND << client_data.getCommand() << "\n";

  const std::vector<std::string>& config_v = client_data.getConfigs();
  for (std::vector<std::string>::const_iterator iter_config = config_v.begin(); iter_config != config_v.end(); ++iter_config) {
    const std::string& config = *iter_config;
    o_data << config;
    offset += config.length();
  }

  o_offset = add_header(o_header, CONFIGURATION, o_offset, offset);

  const std::vector<std::string>& config_expr_v = client_data.getConfigExprs();
  for (std::vector<std::string>::const_iterator iter_config_expr = config_expr_v.begin(); iter_config_expr != config_expr_v.end(); ++iter_config_expr) {
    const std::string& config_expr = *iter_config_expr;
    o_data << config_expr << ';';
    offset += config_expr.length() + 1;
  }

  o_offset = add_header(o_header, CONFIGURATION_EXPRESSIONS, o_offset, offset);

  const std::string& config_vars = client_data.getConfigVars();
  o_data << config_vars;
  offset += config_vars.length();
  o_offset = add_header(o_header, CONFIGURATION_VARIABLES, o_offset, offset);

  const std::string& network = client_data.getNetwork();
  o_data << network;
  offset += network.length();

  o_offset = add_header(o_header, NETWORK, o_offset, offset);

  data = o_header.str() + "\n" + o_data.str();
}

void DataStreamer::buildStreamData(std::string &data, const ServerData& server_data)
{
  std::ostringstream o_header;
  std::ostringstream o_data;
  size_t offset = 0;
  size_t o_offset = offset;

  o_header << RETURN << " " << MABOSS_MAGIC << "\n";
  o_header << STATUS << server_data.getStatus() << "\n";
  if (server_data.getStatus() != 0) {
    o_header << ERROR_MESSAGE << server_data.getErrorMessageRaw() << "\n";
    data = o_header.str() + "\n\n";
    return;
  }

  const std::string& traj = server_data.getTraj();
  if (traj.length() > 0) {
    o_data << traj;
    offset += traj.length();
    o_offset = add_header(o_header, TRAJECTORIES, o_offset, offset);
  }

  const std::string& stat_dist = server_data.getStatDist();
  if (stat_dist.length() > 0) {
    o_data << stat_dist;
    offset += stat_dist.length();
    o_offset = add_header(o_header, STATIONARY_DISTRIBUTION, o_offset, offset);
  }

  const std::string& prob_traj = server_data.getProbTraj();
  if (prob_traj.length() > 0) {
    o_data << prob_traj;
    offset += prob_traj.length();
    o_offset = add_header(o_header, TRAJECTORY_PROBABILITY, o_offset, offset);
  }

  const std::string& fp = server_data.getFP();
  if (fp.length() > 0) {
    o_data << fp;
    offset += fp.length();
    o_offset = add_header(o_header, FIXED_POINTS, o_offset, offset);
  }

  const std::string& finalprob = server_data.getFinalProb();
  if (finalprob.length() > 0) {
    o_data << finalprob;
    offset += finalprob.length();
    o_offset = add_header(o_header, FINAL_PROB, o_offset, offset);
  }

  const std::string& run_log = server_data.getRunLog();
  if (run_log.length() > 0) {
    o_data << run_log;
    offset += run_log.length();
    o_offset = add_header(o_header, RUN_LOG, o_offset, offset);
  }

  data = o_header.str() + "\n" + o_data.str();
}

std::string DataStreamer::error(int status, const std::string& errmsg)
{
  std::ostringstream o_str;
  o_str << RETURN << " " << MABOSS_MAGIC << "\n" << STATUS << status << "\n" << ERROR_MESSAGE << errmsg << "\n\n";
  return o_str.str();
}

int DataStreamer::parseHeaderItems(const std::string &header, std::vector<HeaderItem>& headerItems, std::string& err_data)
{
  size_t opos = 0;
  size_t pos = 0;

  for (;;) {
    pos = header.find(':', opos);
    if (pos == std::string::npos) {
      break;
    }
    std::string directive = header.substr(opos, pos+1-opos);
    opos = pos+1;
    pos = header.find('\n', opos);
    if (pos == std::string::npos) {
      err_data = "newline not found in header after directive " + directive;
      return 1;
    }
    std::string value = header.substr(opos, pos-opos);
    opos = pos+1;
    size_t pos2 = value.find("-");
    /*
    if (directive == STATUS || directive == ERROR_MESSAGE || directive == PROTOCOL_VERSION || directive == FLAGS || directive == COMMAND) {
      headerItems.push_back(HeaderItem(directive, value));
    } else if (pos2 != std::string::npos) {
      headerItems.push_back(HeaderItem(directive, atoll(value.substr(0, pos2).c_str()), atoll(value.substr(pos2+1).c_str())));
    } else {
      err_data = "dash - not found in value " + value + " after directive " + directive;
      return 1;
    */
    if (pos2 != std::string::npos) {
      //if (directive == CONFIGURATION || directive == NETWORK) {
      headerItems.push_back(HeaderItem(directive, atoll(value.substr(0, pos2).c_str()), atoll(value.substr(pos2+1).c_str())));
    } else {
      headerItems.push_back(HeaderItem(directive, value));
    }
  }

  return 0;
}

int DataStreamer::parseStreamDataBegin(const std::string& request, std::string& err_data, std::string& header, std::string& data, const std::string& MAGIC_PREFIX)
{
  std::string magic = MAGIC_PREFIX + MABOSS_MAGIC;
  size_t pos = request.find(magic);
  if (pos == std::string::npos) {
    int status = 1;
    err_data = error(status, "Magic " + magic + " not found in header");
    return status;
  }

  size_t offset = magic.length();
  pos = request.find("\n\n", offset);
  if (pos == std::string::npos) {
    int status = 2;
    err_data = error(status, "Two newlines separator not found in header");
    return status;
  }

  offset++;
  header = request.substr(offset, pos-offset+1);
  data  = request.substr(pos+2);
  return 0;
}

int DataStreamer::parseStreamData(ClientData& client_data, const std::vector<HeaderItem>& headerItems, const std::string& data, ServerData& server_data)
{
  for (std::vector<HeaderItem>::const_iterator headerItemIter = headerItems.begin(); headerItemIter != headerItems.end(); ++headerItemIter) {
    const std::string& directive = headerItemIter->getDirective();
    std::string data_value = data.substr(headerItemIter->getFrom(), headerItemIter->getTo() - headerItemIter->getFrom() + 1);
    if (directive == PROTOCOL_VERSION) {
      client_data.setProtocolVersion(headerItemIter->getValue());
    } else if (directive == FLAGS) {
      client_data.setFlags(atoll(headerItemIter->getValue().c_str()));
    } else if (directive == COMMAND) {
      client_data.setCommand(headerItemIter->getValue());
    } else if (directive == NETWORK) {
      client_data.setNetwork(data_value);
    } else if (directive == CONFIGURATION) {
      client_data.addConfig(data_value);
    } else if (directive == CONFIGURATION_EXPRESSIONS) {
      client_data.addConfigExpr(data_value);
    } else if (directive == CONFIGURATION_VARIABLES) {
      client_data.setConfigVars(data_value);
    } else {
      server_data.setStatus(4);
      server_data.setErrorMessage("unknown directive " + directive);
      return 1;
    }
    /*
    std::cout << "directive [" << headerItemIter->getDirective() << "]\n";
    std::cout << "from [" << headerItemIter->getFrom() << "]\n";
    std::cout << "to [" << headerItemIter->getTo() << "]\n";
    */
  }

  //std::cout << "header [" << header << "]\n";
  //std::cout << "data [" << data << "]\n";
  return 0;
}

int DataStreamer::parseStreamData(ServerData& server_data, const std::string& request)
{
  std::string header;
  std::string data;
  std::string err_data;
  int status = parseStreamDataBegin(request, err_data, header, data, RETURN + " ");
  if (status) {
    server_data.setStatus(status);
    server_data.setErrorMessage(err_data);
    return 1;
  }

  std::vector<HeaderItem> headerItems;
  if (parseHeaderItems(header, headerItems, err_data)) {
    server_data.setStatus(3);
    server_data.setErrorMessage(err_data);
    return 1;
  }

  for (std::vector<HeaderItem>::const_iterator headerItemIter = headerItems.begin(); headerItemIter != headerItems.end(); ++headerItemIter) {
    const std::string& directive = headerItemIter->getDirective();
    if (directive == STATUS) {
      server_data.setStatus(atoi(headerItemIter->getValue().c_str()));
    } else if (directive == ERROR_MESSAGE) {
      server_data.setErrorMessage(headerItemIter->getValue());
    } else {
      std::string data_value = data.substr(headerItemIter->getFrom(), headerItemIter->getTo() - headerItemIter->getFrom() + 1);

      if (directive == STATIONARY_DISTRIBUTION) {
	server_data.setStatDist(data_value);
      } else if (directive == TRAJECTORY_PROBABILITY) {
	server_data.setProbTraj(data_value);
      } else if (directive == TRAJECTORIES) {
	server_data.setTraj(data_value);
      } else if (directive == FIXED_POINTS) {
	server_data.setFP(data_value);
      } else if (directive == FINAL_PROB) {
	server_data.setFinalProb(data_value);
      } else if (directive == RUN_LOG) {
	server_data.setRunLog(data_value);
      } else {
	server_data.setErrorMessage("unknown directive " + directive);
	server_data.setStatus(4);
	return 1;
      }
    }
  }

  return 0;
}

