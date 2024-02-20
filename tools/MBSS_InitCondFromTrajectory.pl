#!/usr/bin/env perl
#
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
##############################################################################
# Tool: MBSS_InitCondFromTrajectory.pl
# Author: Gautier Stoll, gautier_stoll@yahoo.fr
# Date: Jan 2017
#

use strict;

my $bndFile=shift;
if (!$bndFile) {printf "Missing bnd file, MBSS_InitCondFromTrajectory.pl <file.bnd> <filetraj.csv> <linenumber>\n";exit;}
my $probtrajFile=shift;
if (!$probtrajFile) {printf "Missing bnd file, MBSS_InitCondFromTrajectory.pl <file.bnd> <filetraj.csv> <linenumber>\n";exit;}
my $linenumber=shift;
if (!$linenumber) {printf "Missing bnd file, MBSS_InitCondFromTrajectory.pl <file.bnd> <filetraj.csv> <linenumber>\n";exit;}

open(BND,$bndFile) or die "cannot find .bnd file";
my @NodeList;
while(<BND>)
{
    if ( (/node/) || (/Node/))
    {
	s/node//;
	s/Node//;
	s/\{.*//;
	s/\s+//g;
	@NodeList=(@NodeList,$_);
    }
}
close(BND);
open(TRJF,$probtrajFile) or die "cannot find trajectory file";
for (my $i=0;$i <= $linenumber;$i++){defined($_=<TRJF>) or die "traj file is empty";} #header of probtraj file is not consider
close(TRJF);
my @LineList=split(/\t/);

my $LineInitCond="[".$NodeList[0];
foreach (@NodeList[1..$#NodeList]){$LineInitCond=$LineInitCond.",".$_;}
$LineInitCond=$LineInitCond."].istate =";
for (my $index=0;$index<=$#LineList;$index++) 
{
    $_=$LineList[$index];  
#    if (/[A-Z,a-z]/) 
    if ((/^[A-Z,a-z]/) || (/nil/))
    {
	$LineInitCond=$LineInitCond.$LineList[$index+1]." [";
#	if (/$NodeList[0]/) {$LineInitCond=$LineInitCond."1";}
	if (($_ eq $NodeList[0]) || /^$NodeList[0] / || / $NodeList[0] / || / $NodeList[0]$/) 
	{$LineInitCond=$LineInitCond."1";}

	else {$LineInitCond=$LineInitCond."0";}
	foreach my $Node (@NodeList[1..$#NodeList])
	{
	 $LineInitCond=$LineInitCond.",";
	 if (($_ eq $Node) || /^$Node / || / $Node / || / $Node$/) {$LineInitCond=$LineInitCond."1";}
	else {$LineInitCond=$LineInitCond."0";}
	}
	$LineInitCond=$LineInitCond."] , ";	
    }
}
chop($LineInitCond);
chop($LineInitCond);
$LineInitCond=$LineInitCond.";\n";
printf $LineInitCond;