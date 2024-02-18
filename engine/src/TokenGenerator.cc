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
     TokenGenerator.cc

   Authors:
     Eric Viara <viara@sysra.com>
 
   Date:
     Feb 2024
*/

#include <sstream>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include "TokenGenerator.h"
#include "RandomGenerator.h"

std::string TokenGenerator::randomIntString()
{
  std::ostringstream ostr;
  double rd = randomGenerator->generate();
  ostr << rd;
  std::string s_rd = ostr.str();
  s_rd.replace(0, 2, "");
  return s_rd;
}

TokenGenerator* TokenGenerator::tokenGenerator;

TokenGenerator::TokenGenerator()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  randomGenerator = new Rand48RandomGenerator(tv.tv_sec);
  std::string rd = randomIntString(); 
  jobStart = atoi(rd.c_str());
}

std::string TokenGenerator::generateJobToken()
{
  std::ostringstream ostr;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  ostr << jobStart++ << "::" << randomIntString() << "::" << tv.tv_usec;
  return ostr.str();
}

std::string TokenGenerator::generateAdminToken()
{
  std::ostringstream ostr;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  ostr << "adm::" << randomIntString() << "::" << randomIntString() << "::" << tv.tv_usec;
  return ostr.str();
}
