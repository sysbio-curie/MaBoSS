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
  MaBoSS.cc

  Authors:
  Eric Viara <viara@sysra.com>
  Gautier Stoll <gautier.stoll@curie.fr>
  Vincent Noël <vincent.noel@curie.fr>
 
  Date:
  January-March 2011
*/

#include <stdlib.h>
#include <ctime>
#include "engines/MaBEstEngine.h"
#include "engines/EnsembleEngine.h"
#include "engines/StochasticSimulationEngine.h"
#include "engines/FinalStateSimulationEngine.h"
#include "Function.h"
#include <fstream>
#include "Utils.h"
#include "displayers/ProbTrajDisplayer.h"
#include "displayers/StatDistDisplayer.h"
#include "RandomGenerator.h"
#include "compatibility_header.h"

#ifdef SBML_COMPAT
#include "sbml/SBMLExporter.h"
#ifdef SEDML_COMPAT
#include "sedml/SedEngine.h"
#endif
#endif
#ifdef MPI_COMPAT
#include <mpi.h>
int world_size, world_rank;
#endif


const char* prog = "MaBoSS";

static int usage(std::ostream& os = std::cerr)
{
  os << "\nUsage:\n\n";
  os << "  " << prog << " [-h|--help]\n\n";
  os << "  " << prog << " [-V|--version]\n\n";
  os << "  " << prog << " [-c|--config CONF_FILE] [-v|--config-vars VAR1=NUMERICzC[,VAR2=...]] [-e|--config-expr CONFIG_EXPR] -d|--dump-config BOOLEAN_NETWORK_FILE\n\n";
  os << "  " << prog << " [-c|--config CONF_FILE] [-v|--config-vars VAR1=NUMERIC[,VAR2=...]] [-e|--config-expr CONFIG_EXPR] -l|--generate-logical-expressions BOOLEAN_NETWORK_FILE\n\n";
  os << "  " << prog << " [-c|--config CONF_FILE] [-v|--config-vars VAR1=NUMERIC[,VAR2=...]] [-e|--config-expr CONFIG_EXPR] -x|--export-sbml SBML_FILE BOOLEAN_NETWORK_FILE\n\n";
#ifdef SEDML_COMPAT
  os << "  " << prog << " [-s|--sedml CONF_FILE]\n\n";
#endif
  os << "  " << prog << " -t|--generate-config-template BOOLEAN_NETWORK_FILE\n";
  os << "  " << prog << " [-q|--quiet]\n";
  os << "  " << prog << " [--verbose level\n";
#ifdef HDF5_COMPAT
  os << "  " << prog << " [--format csv|json|hdf5]\n";
#else
  os << "  " << prog << " [--format csv|json]\n";
#endif
  os << "  " << prog << " [--check]\n";
  os << "  " << prog << " [--override]\n";
  os << "  " << prog << " [--augment]\n";
  os << "  " << prog << " [--hexfloat]\n";
  os << "  " << prog << " [--ensemble [--save-individual] [--random-sampling] [--ensemble-istates]]\n";
  os << "  " << prog << " [--final]\n";
  os << "  " << prog << " [--use-sbml-names]\n";
  return 1;
}

static int help()
{
  //  std::cout << "\n=================================================== " << prog << " help " << "===================================================\n";
  (void)usage(std::cout);
  std::cout << "\nOptions:\n\n";
  std::cout << "  -V --version                            : displays MaBoSS version\n";
  std::cout << "  -c --config CONF_FILE                   : uses CONF_FILE as a configuration file\n";
  std::cout << "  -s --sedml SEDML_FILE                   : uses SEDML_FILE as a configuration file\n";
  std::cout << "  -v --config-vars VAR=NUMERIC[,VAR2=...] : sets the value of the given variables to the given numeric values\n";
  //  std::cout << "                                        the VAR value in the configuration file (if present) will be overriden\n";
  std::cout << "  -e --config-expr CONFIG_EXPR            : evaluates the configuration expression; may have multiple expressions\n";
  std::cout << "                                            separated by semi-colons\n";
#ifdef HDF5_COMPAT
  std::cout << "  --format csv|json|hdf5                  : if set, format output in the given option: csv (tab delimited), json or hdf5; csv being the default\n";
#else
  std::cout << "  --format csv|json                       : if set, format output in the given option: csv (tab delimited) or json; csv being the default\n";
#endif
  std::cout << "  --override                              : if set, a new node definition will replace a previous one\n";
  std::cout << "  --augment                               : if set, a new node definition will complete (add non existing attributes) / override (replace existing attributes) a previous one\n";
  std::cout << "  --verbose level                         : Set level of verbose (0-3, default 0)\n";
  std::cout << "  -o --output OUTPUT                      : prefix to be used for output files; when present run MaBoSS simulation process\n";
  std::cout << "  -d --dump-config                        : dumps configuration and exits\n";
  std::cout << "  -t --generate-config-template           : generates template configuration and exits\n";
  std::cout << "  -l --generate-logical-expressions       : generates the logical expressions and exits\n";
  std::cout << "  -q|--quiet                              : no notices and no warnings will be displayed\n";
  std::cout << "  --check                                 : checks network and configuration files and exits\n";
  std::cout << "  --hexfloat                              : displays double in hexadecimal format\n";
  std::cout << "  --use-sbml-names                        : use the names of the species when importing sbml\n";
  std::cout << "  -x|--export-sbml SBML_FILE              : export the model to sbml\n";
  std::cout << "  -h --help                               : displays this message\n";
  std::cout << "\nEnsembles:\n";
  std::cout << "  --ensemble                             : simulate ensembles\n";
  std::cout << "  --random-sampling                      : randomly select which model to simulate\n";
  std::cout << "  --save-individual                      : export results of individual models\n";
  std::cout << "  --ensemble-istates                     : Each model will have it's own cfg file. Must provide configs via -c, in the same order as the models\n";
  std::cout << "\nFinal:\n";
  std::cout << "  --final                                : Only export final probabilities\n";
  std::cout << "\nNotices:\n";
  std::cout << "\n1. --config and --config-expr options can be used multiple times;\n";
  std::cout << "   multiple --config and/or --config-expr options are managed in the order given at the command line;\n";
  std::cout << "   --config-vars VAR=VALUE always overrides any VAR assignment in a configuration file or expression\n";
  std::cout << "\n2. --dump-config, --generate-config-template, --generate-logical-expressions and --output are exclusive options\n";
  std::cout << '\n';

  std::cout << "Builtin functions:\n\n";
  Function::displayFunctionDescriptions(std::cout);

  return 0;
}

enum OutputFormat {
  CSV_FORMAT = 1,
  JSON_FORMAT = 2,
#ifdef HDF5_COMPAT
  HDF5_FORMAT = 3
#endif
};

static std::string format_extension(OutputFormat format) {
  switch(format) {
  case CSV_FORMAT:
    return ".csv";
  case JSON_FORMAT:
    return ".json";
#ifdef HDF5_COMPAT
  case HDF5_FORMAT:
    return ".h5";
#endif
  default:
    return NULL;
  }
}

static void display(ProbTrajEngine* engine, Network* network, const char* prefix, OutputFormat format, bool hexfloat, int individual) 
{
  ProbTrajDisplayer<NetworkState>* probtraj_displayer;
  StatDistDisplayer* statdist_displayer;
  FixedPointDisplayer* fp_displayer;
  
  std::ostream* output_probtraj = NULL;
  std::ostream* output_fp = NULL;
  std::ostream* output_statdist = NULL;
  std::ostream* output_statdist_cluster = NULL;
  std::ostream* output_statdist_distrib = NULL;
  std::ostream* output_observed_graph = NULL;
  std::ostream* output_observed_durations = NULL;
  
  
#ifdef HDF5_COMPAT
  hid_t hdf5_file;
  
  if (format == HDF5_FORMAT) {
#ifdef MPI_COMPAT
  if (world_rank == 0) {
#endif
    hdf5_file = H5Fcreate((std::string(prefix) + format_extension(format)).c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
#ifdef MPI_COMPAT
  }
#endif
  } 
#endif

  if (format == CSV_FORMAT || format == JSON_FORMAT) {
#ifdef MPI_COMPAT
  if (world_rank == 0) {
#endif
    output_probtraj = new std::ofstream((std::string(prefix) + "_probtraj" + format_extension(format)).c_str());
    output_fp = new std::ofstream((std::string(prefix) + "_fp" + format_extension(format)).c_str());
    output_statdist = new std::ofstream((std::string(prefix) + "_statdist" + format_extension(format)).c_str());
    output_statdist_cluster = new std::ofstream((std::string(prefix) + "_statdist_cluster" + format_extension(format)).c_str());
    output_statdist_distrib = new std::ofstream((std::string(prefix) + "_statdist_distrib" + format_extension(format)).c_str());
    output_observed_graph = new std::ofstream((std::string(prefix) + "_observed_graph.csv"));
    output_observed_durations = new std::ofstream((std::string(prefix) + "_observed_durations.csv"));
#ifdef MPI_COMPAT
  }
#endif
  }

  if (format == CSV_FORMAT) {
    probtraj_displayer = new CSVProbTrajDisplayer<NetworkState>(network, *output_probtraj, hexfloat);
    statdist_displayer = new CSVStatDistDisplayer(network, *output_statdist, hexfloat);
    fp_displayer = new CSVFixedPointDisplayer(network, *output_fp, hexfloat);
  } else if (format == JSON_FORMAT) {
    probtraj_displayer =  new JSONProbTrajDisplayer<NetworkState>(network, *output_probtraj, hexfloat);
    statdist_displayer = new JSONStatDistDisplayer(network, *output_statdist, *output_statdist_cluster, *output_statdist_distrib, hexfloat);
    fp_displayer = new JsonFixedPointDisplayer(network, *output_fp, hexfloat);
#ifdef HDF5_COMPAT
  } else if (format == HDF5_FORMAT) {
    probtraj_displayer =  new HDF5ProbTrajDisplayer<NetworkState>(network, hdf5_file);
    statdist_displayer = new HDF5StatDistDisplayer(network, hdf5_file);
    fp_displayer = new HDF5FixedPointDisplayer(network, hdf5_file);
#endif
  } else {
    probtraj_displayer = NULL;
    statdist_displayer = NULL;
    fp_displayer = NULL;
  }
  if (individual >= 0) {
    (static_cast<EnsembleEngine*>(engine))->displayIndividual(individual, probtraj_displayer, statdist_displayer, fp_displayer);
  } else {
    engine->display(probtraj_displayer, statdist_displayer, fp_displayer);
    engine->displayObservedGraph(output_observed_graph, output_observed_durations);
  }
  
  delete probtraj_displayer;
  delete statdist_displayer;
  delete fp_displayer;
  
#ifdef HDF5_COMPAT
  if (format == HDF5_FORMAT) {
#ifdef MPI_COMPAT
  if (world_rank == 0) {
#endif
    H5Fclose(hdf5_file);
#ifdef MPI_COMPAT
  }
#endif
  }
#endif

  if (format == CSV_FORMAT || format == JSON_FORMAT) {
#ifdef MPI_COMPAT
  if (world_rank == 0) {
#endif
    ((std::ofstream*) output_probtraj)->close();
    ((std::ofstream*) output_statdist)->close();
    ((std::ofstream*) output_statdist_cluster)->close();
    ((std::ofstream*) output_statdist_distrib)->close();
    ((std::ofstream*) output_fp)->close();
    ((std::ofstream*) output_observed_graph)->close();
    ((std::ofstream*) output_observed_durations)->close();
#ifdef MPI_COMPAT
  }
#endif
      
    delete output_probtraj;
    delete output_fp;
    delete output_statdist;
    delete output_statdist_cluster;
    delete output_statdist_distrib;
    delete output_observed_graph;
    delete output_observed_durations;
  }
}

int run_ensemble_istates(std::vector<char *> ctbndl_files, std::vector<ConfigOpt> runconfig_file_or_expr_v, const char* output, OutputFormat format, bool hexfloat, bool save_individual_results, bool random_sampling) 
{
  time_t start_time, end_time;
     
  std::ostream* output_probtraj = NULL;
  std::ostream* output_fp = NULL;
     
  std::vector<Network *> networks;
  RunConfig* runconfig = new RunConfig();      

  Network* first_network = new Network();
  first_network->parse(ctbndl_files[0]);
  networks.push_back(first_network);

  ConfigOpt& cfg = runconfig_file_or_expr_v[0];
	if (cfg.isExpr()) {
	  runconfig->parseExpression(networks[0], (cfg.getExpr() + ";").c_str());
	} else {
	  runconfig->parse(networks[0], cfg.getFile().c_str());
	}
  
  IStateGroup::checkAndComplete(networks[0]);

  std::map<std::string, NodeIndex> nodes_indexes;
  std::vector<Node*> first_network_nodes = first_network->getNodes();
  for (unsigned int i=0; i < first_network_nodes.size(); i++) {
    Node* t_node = first_network_nodes[i];
    nodes_indexes[t_node->getLabel()] = t_node->getIndex();
  }

  for (unsigned int i=1; i < ctbndl_files.size(); i++) {
    
    Network* network = new Network();
    network->parse(ctbndl_files[i], &nodes_indexes);
    networks.push_back(network);

    cfg = runconfig_file_or_expr_v[i];
    if (cfg.isExpr()) {
      runconfig->parseExpression(networks[i], (cfg.getExpr() + ";").c_str());
    } else {
      runconfig->parse(networks[i], cfg.getFile().c_str());
    }


    const std::vector<Node*> nodes = networks[i]->getNodes();
    for (unsigned int j=0; j < nodes.size(); j++) {
	    // if (!first_network_nodes[j]->istateSetRandomly()) {
	    //     nodes[j]->setIState(first_network_nodes[j]->getIState(first_network, randgen));
	    // }

	    nodes[j]->isInternal(first_network_nodes[j]->isInternal());

	    // if (!first_network_nodes[j]->isReference()) {
	    //   nodes[j]->setReferenceState(first_network_nodes[j]->getReferenceState());
	    // }
    }

    IStateGroup::checkAndComplete(networks[i]);
    networks[i]->getSymbolTable()->checkSymbols();
  }

  // output_run = new std::ofstream((std::string(output) + "_run.txt").c_str());
#ifdef MPI_COMPAT
  if (world_rank == 0) {
#endif 
  output_probtraj = new std::ofstream((std::string(output) + "_probtraj" + format_extension(format)).c_str());
  output_fp = new std::ofstream((std::string(output) + "_fp.csv").c_str());
#ifdef MPI_COMPAT
  }
#endif
  time(&start_time);

#ifdef MPI_COMPAT
  EnsembleEngine engine(networks, runconfig, world_size, world_rank, save_individual_results, random_sampling);
#else
  EnsembleEngine engine(networks, runconfig, save_individual_results, random_sampling);
#endif

  engine.run(NULL);
  
  display(&engine, networks[0], output, format, hexfloat, -1);
        
  if (save_individual_results) {
    for (unsigned int i=0; i < networks.size(); i++) {
      display(&engine, networks[i], (std::string(output) + "_model_" + std::to_string(i)).c_str(), format, hexfloat, i);
    }
  }
  time(&end_time);

  // ((std::ofstream*)output_run)->close();
#ifdef MPI_COMPAT
  if (world_rank == 0) {
#endif

  ((std::ofstream*)output_probtraj)->close();
  ((std::ofstream*)output_fp)->close();
#ifdef MPI_COMPAT
  }
#endif

  delete output_probtraj;
  delete output_fp;
  
  return 0;
}

int run_ensemble(std::vector<char *> ctbndl_files, std::vector<ConfigOpt> runconfig_file_or_expr_v, const char* output, OutputFormat format, bool hexfloat, bool save_individual_results, bool random_sampling)
{
  
  time_t start_time, end_time;
     
  std::ostream* output_probtraj = NULL;
  std::ostream* output_fp = NULL;
  std::vector<Network *> networks;
  RunConfig* runconfig = new RunConfig();      

  Network* first_network = new Network();
  first_network->parse(ctbndl_files[0]);
  networks.push_back(first_network);

  for (const auto & cfg : runconfig_file_or_expr_v) {
    if (cfg.isExpr()) {
      runconfig->parseExpression(networks[0], (cfg.getExpr() + ";").c_str());
    } else {
      runconfig->parse(networks[0], cfg.getFile().c_str());
    }
  }

  IStateGroup::checkAndComplete(networks[0]);

  RandomGeneratorFactory* randgen_factory = runconfig->getRandomGeneratorFactory();
  RandomGenerator* randgen = randgen_factory->generateRandomGenerator(runconfig->getSeedPseudoRandom());

  std::map<std::string, NodeIndex> nodes_indexes;
  std::vector<Node*> first_network_nodes = first_network->getNodes();
  for (unsigned int i=0; i < first_network_nodes.size(); i++) {
    Node* t_node = first_network_nodes[i];
    nodes_indexes[t_node->getLabel()] = t_node->getIndex();
  }

  for (unsigned int i=1; i < ctbndl_files.size(); i++) {
    
    Network* network = new Network();
    network->parse(ctbndl_files[i], &nodes_indexes);
    networks.push_back(network);

    
    network->cloneIStateGroup(first_network->getIStateGroup());
    const std::vector<Node*> nodes = networks[i]->getNodes();
    for (unsigned int j=0; j < nodes.size(); j++) {
if (!first_network_nodes[j]->istateSetRandomly()) {
  nodes[j]->setIState(first_network_nodes[j]->getIState(first_network, randgen));
}

nodes[j]->isInternal(first_network_nodes[j]->isInternal());

// if (!first_network_nodes[j]->isReference()) {
//   nodes[j]->setReferenceState(first_network_nodes[j]->getReferenceState());
// }
    }

    IStateGroup::checkAndComplete(networks[i]);
    networks[i]->getSymbolTable()->checkSymbols();
  }

  // output_run = new std::ofstream((std::string(output) + "_run.txt").c_str());
#ifdef MPI_COMPAT
  if (world_rank == 0) {
#endif
  output_probtraj = new std::ofstream((std::string(output) + "_probtraj" + format_extension(format)).c_str());
  output_fp = new std::ofstream((std::string(output) + "_fp.csv").c_str());
#ifdef MPI_COMPAT
  }
#endif

  time(&start_time);
#ifdef MPI_COMPAT
  EnsembleEngine engine(networks, runconfig, world_size, world_rank, save_individual_results, random_sampling);
#else
  EnsembleEngine engine(networks, runconfig, save_individual_results, random_sampling);
#endif
  engine.run(NULL);
  
  display(&engine, networks[0], output, format, hexfloat, -1);
        
  if (save_individual_results) {
    for (unsigned int i=0; i < networks.size(); i++) {
      display(&engine, networks[i], (std::string(output) + "_model_" + std::to_string(i)).c_str(), format, hexfloat, i);
    }
  }
  time(&end_time);

  // ((std::ofstream*)output_run)->close();
#ifdef MPI_COMPAT
  if (world_rank == 0) {
#endif
  ((std::ofstream*)output_probtraj)->close();
  ((std::ofstream*)output_fp)->close();
#ifdef MPI_COMPAT
  }
#endif
  delete output_probtraj;
  delete output_fp;

  delete runconfig;
  // for (std::vector<Network*>::iterator it = networks.begin(); it != networks.end(); ++it)
  for (auto * network : networks)
    delete network;

  Function::destroyFuncMap();  
  return 0;
}

int run_single(const char* ctbndl_file, std::vector<std::string> runconfig_var_v, std::vector<ConfigOpt> runconfig_file_or_expr_v, const char* output, bool generate_config_template) 
{
  Network* network = new Network();

  network->parse(ctbndl_file);

  RunConfig* runconfig = new RunConfig();

  if (generate_config_template) {
    IStateGroup::checkAndComplete(network);
    runconfig->generateTemplate(network, std::cout, StochasticSimulationEngine::VERSION);
    return 0;
  }

  if (setConfigVariables(network, prog, runconfig_var_v)) {
    return 1;
  }      

  for (const auto & cfg : runconfig_file_or_expr_v) {
    if (cfg.isExpr()) {
      runconfig->parseExpression(network, (cfg.getExpr() + ";").c_str());
    } else {
      runconfig->parse(network, cfg.getFile().c_str());
    }
  }

  IStateGroup::checkAndComplete(network);

  network->getSymbolTable()->checkSymbols();

  std::ostream* output_run = new std::ofstream((std::string(output) + "_run.txt").c_str());

  StochasticSimulationEngine single_simulation(network, runconfig, runconfig->getSeedPseudoRandom());
  
  NetworkState initial_state;
  network->initStates(initial_state, single_simulation.random_generator);
  NetworkState final_state = single_simulation.run(initial_state, output_run);
  std::cout << final_state.getName(network) << std::endl;
  ((std::ofstream*)output_run)->close();
  delete output_run;


  delete runconfig;
  delete network;

  Function::destroyFuncMap();  
  return 0;
}

#ifdef SEDML_COMPAT
int run_sedml(std::string sedml_file) 
{

  SedEngine sed_engine;
  sed_engine.parse(sedml_file);
  sed_engine.run();
  sed_engine.writeReports();
  Function::destroyFuncMap();  
  return 0;
}
#endif

int main(int argc, char* argv[])
{

#ifdef MPI_COMPAT  
  MPI_Init(NULL, NULL);
  // Get the number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  // Get the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
#endif

  const char* output = NULL;
  std::vector<ConfigOpt> runconfig_file_or_expr_v;
  std::vector<std::string> runconfig_var_v;
  std::string sedml_file = "";
  const char* ctbndl_file = NULL;
  bool single_simulation = false;
  bool final_simulation = false;
  bool ensemble = false;
  bool ensemble_save_individual_results = false;
  bool ensemble_random_sampling = false;
  bool ensemble_istates = false;
  std::vector<char *> ctbndl_files;
  bool dump_config = false;
  bool generate_config_template = false;
  bool generate_bnd_file = false;
  bool generate_logical_expressions = false;
  bool hexfloat = false;
  bool check = false;
  dont_shrink_logical_expressions = false; // global flag
  bool use_sbml_names = false;
  const char* sbml_file = NULL;
  OutputFormat format = CSV_FORMAT;
  MaBEstEngine::init();

  // for debug
  if (getenv("MABOSS_VERBOSE") != NULL) {
#if USE_DYNAMIC_BITSET_STD_ALLOC
    std::cerr << "MaBoSS use dynamic_bitset [std allocator]\n";
#elif defined(USE_DYNAMIC_BITSET)
    std::cerr << "MaBoSS use MaBoSS dynamic bitset [experimental allocator]\n";
#elif defined(USE_STATIC_BITSET)
    std::cerr << "MaBoSS use standard bitset\n";
#else
    std::cerr << "MaBoSS use long long mask\n";
#endif
#ifdef USE_UNORDERED_MAP
    std::cerr << "MaBoSS use std::unordered_map\n";
#else
    std::cerr << "MaBoSS use standard std::map\n";
#endif
#ifdef STD_THREAD
    std::cerr << "MaBoSS uses std::thread" << std::endl;
#else
    std::cerr << "MaBoSS uses POSIX threads" << std::endl;
#endif
  }

  for (int nn = 1; nn < argc; ++nn) {
    const char* s = argv[nn];
    if (s[0] == '-') {
      if (!strcmp(s, "-version") || !strcmp(s, "--version") || !strcmp(s, "-V")) { // keep -version for backward compatibility
	std::cout << "MaBoSS version " + MaBEstEngine::VERSION << " [networks up to " << MAXNODES << " nodes]\n";
	return 0;
      } else if (!strcmp(s, "--config-vars") || !strcmp(s, "-v")) {
	if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
	runconfig_var_v.push_back(argv[++nn]);
      } else if (!strcmp(s, "--config-expr") || !strcmp(s, "-e")) {
	if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
	runconfig_file_or_expr_v.push_back(ConfigOpt(argv[++nn], true));
      } else if (!strcmp(s, "--dump-config") || !strcmp(s, "-d")) {
	dump_config = true;
      } else if (!strcmp(s, "--generate-bnd-file") || !strcmp(s, "-g")) {
	generate_bnd_file = true;
      } else if (!strcmp(s, "--generate-config-template") || !strcmp(s, "-t")) {
	generate_config_template = true;
      } else if (!strcmp(s, "--generate-logical-expressions") || !strcmp(s, "-l")) {
	generate_logical_expressions = true;
      } else if (!strcmp(s, "--dont-shrink-logical-expressions")) {
	dont_shrink_logical_expressions = true;
      } else if (!strcmp(s, "--ensemble")) {
	ensemble = true;
      } else if (!strcmp(s, "--single")) {
	single_simulation = true;
      } else if (!strcmp(s, "--final")) {
	final_simulation = true;
      } else if (!strcmp(s, "--save-individual")) {
        if (ensemble) {
          ensemble_save_individual_results = true;
        } else {
          std::cerr << "\n" << prog << ": --save-individual only usable if --ensemble is used" << std::endl;
        }
      } else if (!strcmp(s, "--format")) {
	const char* fmt = argv[++nn];
	if (!strcasecmp(fmt, "CSV")) {
	  format = CSV_FORMAT;
	} else if (!strcasecmp(fmt, "JSON")) {
	  format = JSON_FORMAT;
#ifdef HDF5_COMPAT
  } else if (!strcasecmp(fmt, "HDF5")) {
	  format = HDF5_FORMAT;
#endif
  } else {
          std::cerr << "\n" << prog << ": unknown format " << fmt << std::endl;
	  return usage();
	}	  
      } else if (!strcmp(s, "--random-sampling")) {
        if (ensemble) {
          ensemble_random_sampling = true;
        } else {
          std::cerr << "\n" << prog << ": --random-sampling only usable if --ensemble is used" << std::endl;
        }
      } else if (!strcmp(s, "--ensemble-istates")) {
        if (ensemble) {
          ensemble_istates = true;
        } else {
          std::cerr << "\n" << prog << ": --ensemble-istates only usable if --ensemble is used" << std::endl;
        }
      } else if (!strcmp(s, "-x") || !strcmp(s, "--export-sbml")) {
        if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
        sbml_file = argv[++nn];
      
      } else if (!strcmp(s, "--use-sbml-names")) {
        use_sbml_names = true;
        
      } else if (!strcmp(s, "--verbose")) {
        if ((nn+1) < argc) {
          const char* level_chr = argv[++nn];
          if (isdigit(*level_chr)) {
            const int level = atoi(level_chr);
            RunConfig::setVerbose(level);
          } else {
            std::cerr << "\n" << prog << ": unknown level for verbose " << level_chr << std::endl;
            return usage();
          }        
        } else {
          std::cerr << "\n" << prog << ": please choose a level of verbosity (0-3) " << std::endl;
          return usage();
        }
  	  
      } else if (!strcmp(s, "--load-user-functions")) {
	if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
	MaBEstEngine::loadUserFuncs(argv[++nn]);
      } else if (!strcmp(s, "-o") || !strcmp(s, "--output")) {
	if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
	output = argv[++nn];
      } else if (!strcmp(s, "-c") || !strcmp(s, "--config")) {
	if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
	runconfig_file_or_expr_v.push_back(ConfigOpt(argv[++nn], false));
      } else if (!strcmp(s, "-s") || !strcmp(s, "--sedml")) {
  if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
  sedml_file = argv[++nn];
      } else if (!strcmp(s, "--override")) {
	if (Node::isAugment()) {
	  std::cerr << '\n' << prog << ": --override and --augment are exclusive options\n"; return usage();
	}
	Node::setOverride(true);
      } else if (!strcmp(s, "--augment")) {
	if (Node::isOverride()) {
	  std::cerr << '\n' << prog << ": --override and --augment are exclusive options\n"; return usage();
	}
	Node::setAugment(true);
      } else if (!strcmp(s, "--check")) {
	check = true;
      } else if (!strcmp(s, "-q") || !strcmp(s, "--quiet")) {
	MaBoSS_quiet = true;
      } else if (!strcmp(s, "--hexfloat")) {
	hexfloat = true;
      } else if (!strcmp(s, "--help") || !strcmp(s, "-h")) {
	return help();
      } else {
	std::cerr << '\n' << prog << ": unknown option " << s << std::endl;
	return usage();
      }
    } else if (!ensemble && ctbndl_file == NULL) {
      ctbndl_file = argv[nn];
    } else if (ensemble) {
      ctbndl_files.push_back(argv[nn]);
    } else {
      std::cerr << '\n' << prog << ": boolean network file is already set to " << ctbndl_file << " [" << s << "]" << std::endl;
    }
  }

  if (!ensemble && sedml_file.empty() && NULL == ctbndl_file)
    {
      std::cerr << '\n'
		<< prog << ": boolean network file is missing\n";
      return usage();
    }

  if (ensemble && ctbndl_files.size() == 0) {
    std::cerr << '\n'
              << prog << ": ensemble networks are missing\n";
    return usage();
  }
    
  if (!dump_config && !generate_config_template && !generate_logical_expressions && !check && !generate_bnd_file && sbml_file == NULL && sedml_file.empty() && output == NULL) {
    std::cerr << '\n' << prog << ": ouput option is not set\n";
    return usage();
  }

  if (dump_config && generate_config_template) {
    std::cerr << '\n' << prog << ": --dump-config and --generate-config-template are exclusive options\n";
    return usage();
  }

  if (dump_config && output) {
    std::cerr << '\n' << prog << ": --dump-config and -o|--output are exclusive options\n";
    return usage();
  }

  if (generate_config_template && output) {
    std::cerr << '\n' << prog << ": --generate-config-template and -o|--output are exclusive options\n";
    return usage();
  }

  if (generate_config_template && ConfigOpt::getFileCount() > 0) {
    std::cerr<< '\n'  << prog << ": --generate-config-template and -c|--config are exclusive options\n";
    return usage();
  }

  if (generate_config_template && ConfigOpt::getExprCount() > 0) {
    std::cerr << '\n' << prog << ": --generate-config-template and --config-expr are exclusive options\n";
    return usage();
  }

  if (generate_config_template && runconfig_var_v.size() > 0) {
    std::cerr << '\n' << prog << ": --generate-config-template and --config-vars are exclusive options\n";
    return usage();
  }

  if (check && output) {
    std::cerr << '\n' << prog << ": --check and -o|--output are exclusive options\n";
    return usage();
  }

  std::ostream* output_run = NULL;
  std::ostream* output_traj = NULL;
  
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::init_pthread();
#endif

  try {
    time_t start_time, end_time;

    if (ensemble) {
      if (ensemble_istates) {
        run_ensemble_istates(
          ctbndl_files, runconfig_file_or_expr_v, output, format, hexfloat, 
          ensemble_save_individual_results, ensemble_random_sampling
        );
 
      } else {
        run_ensemble(
          ctbndl_files, runconfig_file_or_expr_v, output, format, hexfloat, 
          ensemble_save_individual_results, ensemble_random_sampling
        );
      }
        
    } else if (single_simulation) {
      run_single(
        ctbndl_file, runconfig_var_v, runconfig_file_or_expr_v, 
        output, generate_config_template
      ); 
#ifdef SEDML_COMPAT
    } else if (!sedml_file.empty()) {
      run_sedml(sedml_file);
#endif
    } else {
        
      Network* network = new Network();

      network->parse(ctbndl_file, NULL, false, use_sbml_names);

      RunConfig* runconfig = new RunConfig();

      if (generate_config_template) {
        IStateGroup::checkAndComplete(network);
        runconfig->generateTemplate(network, std::cout, MaBEstEngine::VERSION);
        return 0;
      }

      if (setConfigVariables(network, prog, runconfig_var_v)) {
        return 1;
      }      

      for (const auto & cfg : runconfig_file_or_expr_v) {
        if (cfg.isExpr()) {
	  runconfig->parseExpression(network, (cfg.getExpr() + ";").c_str());
        } else {
	  runconfig->parse(network, cfg.getFile().c_str());
        }
      }

      IStateGroup::checkAndComplete(network);

      network->getSymbolTable()->checkSymbols();

      if (check) {
        return 0;
      }

      if (generate_logical_expressions) {
        network->generateLogicalExpressions(std::cout);
        return 0;
      }

      if (generate_bnd_file) {
        network->display(std::cout);
        return 0;
      }

      if (sbml_file != NULL)
      {
#ifdef SBML_COMPAT
        SBMLExporter sbml_exporter(network, runconfig, sbml_file);
        return 0;
#else
        std::cerr << '\n' << prog << ": SBML support not enabled\n";
        return 1;
#endif
      }
      if (dump_config) {
        runconfig->dump(network, std::cout, MaBEstEngine::VERSION);
        return 0;
      }

      if (runconfig->displayTrajectories()) {
        if (runconfig->getThreadCount() > 1) {
	  if (!MaBoSS_quiet) {
	    std::cerr << '\n' << prog << ": warning: cannot display trajectories in multi-threaded mode\n";
	  }
        } else {
	  output_traj = new std::ofstream((std::string(output) + "_traj.txt").c_str());
        }
      }

      if (final_simulation) {
	std::ostream* output_final = new std::ofstream((std::string(output) + "_finalprob" + format_extension(format)).c_str());

#ifdef MPI_COMPAT
  FinalStateSimulationEngine engine(network, runconfig, world_size, world_rank);
#else
	FinalStateSimulationEngine engine(network, runconfig);
#endif

	engine.run(NULL);
  
  FinalStateDisplayer* final_displayer;
  if (format == CSV_FORMAT) {
    final_displayer = new CSVFinalStateDisplayer(network, *output_final, hexfloat);
  } else if (format == JSON_FORMAT) {
    final_displayer = new JsonFinalStateDisplayer(network, *output_final, hexfloat);
  } else {
    final_displayer = NULL;
  }
  engine.displayFinal(final_displayer);

	((std::ofstream*)output_final)->close();
	delete output_final;
      } else {
        output_run = new std::ofstream((std::string(output) + "_run.txt").c_str());
 
        time(&start_time);
#ifdef MPI_COMPAT
        MaBEstEngine mabest(network, runconfig, world_size, world_rank);
#else
        MaBEstEngine mabest(network, runconfig);
#endif
        mabest.run(output_traj);
        
        display(&mabest, network, output, format, hexfloat, -1);
        
        time(&end_time);

        mabest.displayRunStats(*output_run, start_time, end_time);
        
        ((std::ofstream*)output_run)->close();
        delete output_run;
        if (NULL != output_traj) {
          ((std::ofstream*)output_traj)->close();
          delete output_traj;
        }
      }
      delete runconfig;
      delete network;

      Function::destroyFuncMap();  
    }
  } catch(const BNException& e) {
    std::cerr << '\n' << prog << ": " << e;
    return 1;
  }

#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::end_pthread();
  MBDynBitset::stats();
#endif

#ifdef MPI_COMPAT
  MPI_Finalize();
#endif

  return 0;
}
