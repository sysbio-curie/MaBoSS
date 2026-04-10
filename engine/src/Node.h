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
     Node.h

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2011
*/

#ifndef _NODE_H_
#define _NODE_H_

#include <stdlib.h>
#include <string>
#include <sstream>
#include <map>

#ifdef SBML_COMPAT
#include <sbml/SBMLTypes.h>
#include "sbml/packages/qual/common/QualExtensionTypes.h"
 
LIBSBML_CPP_NAMESPACE_USE
#endif

#include "NetworkState_Impl.h"
#include "RandomGenerator.h"
#include "maps_header.h"

typedef unsigned int NodeIndex;
typedef bool NodeState; // for now... could be a class


static const std::string ATTR_RATE_UP = "rate_up";
static const std::string ATTR_RATE_DOWN = "rate_down";
static const std::string ATTR_LOGIC = "logic";
static const std::string ATTR_DESCRIPTION = "description";
static const NodeIndex INVALID_NODE_INDEX = (NodeIndex)~0U;

class Expression;
class Network;
class NetworkState;
class PopNetworkState;
class LogicalExprGenContext;
// extern std::ostream& operator<<(std::ostream& os, const BNException& e);

class Node {
  static bool override;
  static bool augment;
  std::string label;
  std::string description;
  bool istate_set;
  bool is_internal;
  bool is_reference;
  bool in_graph;
  bool is_mutable;
  NodeState referenceState;
  const Expression* logicalInputExpr;
  const Expression* rateUpExpr;
  const Expression* rateDownExpr;
  mutable NodeState istate;
  NodeIndex index;
  
  MAP<std::string, const Expression*> attr_expr_map;
  MAP<std::string, std::string> attr_str_map;

  /// Not implemented yet
  /// @param rateUpExpr logical expression that conditions the node to go from deactivated to activated
  /// @param rateDownExpr logical expression that conditions the node to go from activated to deactivated
  /// @return /
  Expression* rewriteLogicalExpression(Expression* rateUpExpr, Expression* rateDownExpr) const;
  NetworkState_Impl node_bit;
  std::map<double, Expression*>* schedule;

 public:
  Node(const std::string& label, const std::string& description, NodeIndex index);

  /// Change the index of the node
  /// @param new_index the index to set
  void setIndex(NodeIndex new_index) {
    index = new_index;
#if !defined(USE_STATIC_BITSET) && !defined(USE_DYNAMIC_BITSET)
    node_bit = 1ULL << new_index;
#endif
  }

  ///
  /// @return the label of the node
  const std::string& getLabel() const {
    return label;
  }

  ///
  /// @param _description the description of the node
  void setDescription(const std::string& _description) {
    this->description = _description;
  }

  ///
  /// @return the description of the node
  const std::string& getDescription() const {
    return description;
  }

  ///
  /// @param logicalInputExpr the logical expression used to compute rateup and ratedown if they are null
  void setLogicalInputExpression(const Expression* logicalInputExpr);

  ///
  /// @param expr logical expression that conditions the node to go from deactivated to activated
  void setRateUpExpression(const Expression* expr);

  ///
  /// @param expr logical expression that conditions the node to go from activated to deactivated
  void setRateDownExpression(const Expression* expr);

  ///
  /// @return the logical expression used to compute rateup and ratedown if they are null
  const Expression* getLogicalInputExpression() const {
    return logicalInputExpr;
  }

  ///
  /// @return logical expression that conditions the node to go from deactivated to activated
  const Expression* getRateUpExpression() const {
    return rateUpExpr;
  }

  ///
  /// @return the logical expression that conditions the node to go from activated to deactivated
  const Expression* getRateDownExpression() const {
    return rateDownExpr;
  }

  ///
  /// @param attr_name Special values are ATTR_RATE_UP, ATTR_RATE_DOWN and ATTR_LOGIC;
  /// other names are stored in the custom attribute map
  /// - \c ATTR_RATE_UP : rate up expression (deactivated -> activated)
  /// - \c ATTR_RATE_DOWN : rate down expression (activated -> deactivated)
  /// - \c ATTR_LOGIC : logical expression
  /// @param expr the expression to put
  void setAttributeExpression(const std::string& attr_name, const Expression* expr) {
    if (attr_name == ATTR_RATE_UP) {
      setRateUpExpression(expr);
      return;
    }
    if (attr_name == ATTR_RATE_DOWN) {
      setRateDownExpression(expr);
      return;
    }
    if (attr_name == ATTR_LOGIC) {
      setLogicalInputExpression(expr);
      return;
    }
    attr_expr_map[attr_name] = expr;
  }

  /// Function that disables all the logical expression of the node, replacing the logical input by a constant
  /// that is represented by the \param value
  /// @param value the constant to put for the node
  void mutate(double value);

  /// Function that makes the node mutable and modify the network in consequence
  /// @param network the network to prepare
  void makeMutable(Network* network);

  /// If it is not fixed, the initial state is randomly chosen
  /// @param network / not used
  /// @param randgen the random generator to compute the state of the node
  /// @return the initial state of the node
  NodeState getIState(const Network* network, RandomGenerator* randgen) const;

  /// Set the initial set of the node to the \class NodeState passed in parameter
  /// @param _istate the initial state to put the node in
  void setIState(NodeState _istate) {
    istate_set = true;
    this->istate = _istate;
  }

  ///
  /// @return the logical not of state_set
  bool istateSetRandomly() const {
    return !istate_set;
  }

  /// An internal node is part of the compute but is not computed itself
  /// @return true if the node is internal, else false
  bool isInternal() const {
    return is_internal;
  }

  ///
  /// @return true if the node is in the graph else false
  bool inGraph() const {
    return in_graph;
  }

  /// Set the \a is_internal with the value of \param _is_internal
  void isInternal(bool _is_internal) {
    this->is_internal = _is_internal;
  }
  /// Set the \a in_graph with the value of \param _in_graph
  void inGraph(bool _in_graph) {
    this->in_graph = _in_graph;
  }

  /// Set the flip schedule (when the state of the node changes) to the node
  /// @param _schedule a map of speed and expression of how the node must flip state
  void setScheduledFlips(std::map<double, Expression*>* _schedule) {
    schedule = _schedule;
  }

  ///
  /// @return the flip schedule of the node (when the state of the node changes)
  std::map<double, Expression*>* getScheduledFlips() const {
    return schedule;
  }

  ///
  /// @return true if the node is marked as a reference node //todo precise ?
  bool isReference() const {
    return is_reference;
  }

  /// Set the \a is_reference to the value of \param _is_reference
  void setReference(bool _is_reference) {
    this->is_reference = _is_reference;
  }

  ///
  /// @return the reference state of the node
  NodeState getReferenceState() const {
    return referenceState;
  }

  /// Set the reference state of the node to the value of \param _referenceState
  void setReferenceState(NodeState _referenceState) {
    this->is_reference = true;
    this->referenceState = _referenceState;
  }

  ///
  /// @param attr_name the name of the attribute:
  /// - \c ATTR_RATE_UP: node goes from deactivated to activated
  /// - \c ATTR_RATE_DOWN: node goes from activated to deactivated
  /// - \c ATTR_LOGIC: logical expression
  /// @return the expression of the attribute passed in parameter
  const Expression* getAttributeExpression(const std::string& attr_name) const {
    MAP<std::string, const Expression*>::const_iterator iter = attr_expr_map.find(attr_name);
    if (iter == attr_expr_map.end()) {
      if (attr_name == ATTR_RATE_UP) {
	return getRateUpExpression();
      }
      if (attr_name == ATTR_RATE_DOWN) {
	return getRateDownExpression();
      }
      if (attr_name == ATTR_LOGIC) {
	return getLogicalInputExpression();
      }
      return NULL;
    }
    return (*iter).second;
  }

  /// Sets a string attribute. If attr_name is ATTR_DESCRIPTION, it updates the node description; otherwise it stores
  /// the value in the custom string attribute map
  /// @param attr_name the name of the attribute.
  /// @param str the string to associate with the attribute
  void setAttributeString(const std::string& attr_name, const std::string& str) {
    if (attr_name == ATTR_DESCRIPTION) {
      setDescription(str);
      return;
    }

    attr_str_map[attr_name] = str;
  }

  ///
  /// @param attr_name the attribute to get the string associated with
  /// @return the string associated with the attribute. If attr_name does not exist, returns ""
  std::string getAttributeString(const std::string& attr_name) const {
    MAP<std::string, std::string>::const_iterator iter = attr_str_map.find(attr_name);
    if (iter == attr_str_map.end()) {
      if (attr_name == ATTR_DESCRIPTION) {
	return getDescription();
      }
      return "";
    }
    return (*iter).second;
  }

  ///
  /// @return the index of the node
  NodeIndex getIndex() const {return index;}

#if !defined(USE_STATIC_BITSET) && !defined(USE_DYNAMIC_BITSET)
  NetworkState_Impl getNodeBit() const {return node_bit;}
#endif

  ///
  /// @return the map of attribute and expressions
  const MAP<std::string, const Expression*>& getAttributeExpressionMap() const {
    return attr_expr_map;
  }

  ///
  /// @return the map of attribute and string
  const MAP<std::string, std::string>& getAttributeStringMap() const {
    return attr_str_map;
  }

  ///
  /// @return true if node state does not depend on other node states
  bool isInputNode() const;

  ///
  /// @param network_state
  /// @return the transition rate of the node going from deactivated to activated
  double getRateUp(const NetworkState& network_state) const;

  ///
  /// @param network_state
  /// @return the transition rate of the node going from activated to deactivated
  double getRateDown(const NetworkState& network_state) const;

  /// Same principle as \link getRateUp \endlink but taking into account the population context
  /// @param network_state
  /// @param pop population context
  /// @return
  double getRateUp(const NetworkState& network_state, const PopNetworkState& pop) const;

  /// Same principle as \link getRateDown \endlink but taking into account the population context
  /// @param network_state
  /// @param pop population context
  /// @return
  double getRateDown(const NetworkState& network_state, const PopNetworkState& pop) const;

  /// Interrogates the global state
  /// @param network_state
  /// @return the node state
  NodeState getNodeState(const NetworkState& network_state) const;

  /// Force the state of the node in the context of the network_state
  /// @param network_state
  /// @param state
  void setNodeState(NetworkState& network_state, NodeState state);

  ///
  /// @returns true if and only if there is a logical input expression that allows to compute state from input nodes
  bool computeNodeState(NetworkState& network_state, NodeState& node_state) const;

  std::string toString() const {
    std::ostringstream ostr;
    display(ostr);
    return ostr.str();
  }

  void display(std::ostream& os) const;
  Expression* generateRawLogicalExpression() const;
  void generateLogicalExpression(LogicalExprGenContext& gen) const;

  ///
  /// @param _override set this value of override to the node
  static void setOverride(bool _override) {
    Node::override = _override;
  }

  ///
  /// @return the value of \a isOverride
  static bool isOverride() {return override;}

  ///
  /// @param _augment set this value of augment to the node
  static void setAugment(bool _augment) {
    Node::augment = _augment;
  }

  ///
  /// @return the value of \a augment
  static bool isAugment() {return augment;}

#ifdef SBML_COMPAT
  void writeSBML(QualitativeSpecies* qs) 
  {
    qs->setId(this->getLabel());
    qs->setCompartment("c");
    qs->setConstant(false);
    qs->setInitialLevel(1);
    qs->setMaxLevel(1);
    qs->setName(this->getLabel());
  }
#endif

  void reset();

  ~Node();
};

#endif
