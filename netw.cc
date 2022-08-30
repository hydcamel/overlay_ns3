#include <fstream>
#include <sstream>
#include <iostream>
#include "netw.h"
#include "utils.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("netw");
NS_OBJECT_ENSURE_REGISTERED(netw);

TypeId netw::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::netw")
		.SetParent<Object> ()
		.AddConstructor<netw> ();
	return tid;
}
TypeId netw::GetInstanceTypeId (void) const
{
  	return netw::GetTypeId ();
}

netw::netw(std::string filename, std::string demands_file, std::string file_overlay_nodes, std::string route_name)
{
	set_background_type(CrossType::PktPoisson);
	probe_type = ProbeType::naive;
	read_underlay(filename);
    read_overlay(file_overlay_nodes);
	read_routing_map(route_name);
	read_demands(demands_file);
}
netw::netw(name_input_files &fd_setup)
{
	set_background_type(CrossType::PktPoisson);
	probe_type = ProbeType::naive;
	read_underlay(fd_setup.netw_filename);
    read_overlay(fd_setup.file_overlay_nodes);
	read_routing_map(fd_setup.route_name);
	read_demands(fd_setup.demands_file);
	read_probe_profile(fd_setup.probe_setup_filename);
}

void netw::read_underlay(std::string filename)
{
	std::ifstream infile(filename);
    std::string temp;
    std::string line;
    int idx = 0, idx_true = 0;
	int src, dest;
	/**
	 * Prepare underlay information
	 * */
	if (infile.is_open())
	{
		std::cout << "underlay_file: " << filename << std::endl;
		while (getline(infile, line))
		{
			if (!line.empty())
			{
				std::istringstream iss(line);
				if (line.substr(0, 5).compare("NODES") == 0)
				{
					iss >> temp >> n_nodes;
					adj_mat.resize(n_nodes);
					// m_sent.resize(n_nodes);
					for (uint32_t i = 0; i < n_nodes; i++)
					{
						adj_mat[i].resize(n_nodes, false);
						// m_sent[i].resize(n_nodes, 0);
					}
				}
				else if (line.substr(0, 5).compare("EDGES") == 0)
				{
					iss >> temp >> n_edges;
					/* src.resize(n_edges);
					dest.resize(n_edges); */
					n_edges /= 2;
					w.resize(n_edges);
					bw.resize(n_edges);
					delay.resize(n_edges);
					edges_vec.resize(n_edges);
					background_interval.resize(n_edges);
				}
				else if (line.substr(0, 5).compare("label") == 0)
				{
					continue;
				}
				else if (line.substr(0, 5).compare("edge_") == 0)
				{
					if (idx_true % 2 == 0)
					{
						/* iss >> temp >> src[idx] >> dest[idx] >> w[idx] >> bw[idx] >> delay[idx];
						adj_mat[src[idx]][dest[idx]] = true;
						adj_mat[dest[idx]][src[idx]] = true; */
						iss >> temp >> src >> dest >> w[idx] >> bw[idx] >> delay[idx];
						edges.insert( std::pair<std::string, int>(std::to_string(src) + ' ' + std::to_string(dest), idx) );
						//edges.insert( std::pair<std::pair<int, int>, int>(std::pair<int, int>(src, dest), idx) );
						edges_vec[idx] = std::pair<int, int> (src, dest);
						adj_mat[src][dest] = true;
						adj_mat[dest][src] = true;
						background_interval[idx] = round( (avg_pktSize*8*USTOS) /(BGPERCENTAGE * bw[idx] * 1000) );
						++idx;
					}
					++idx_true;
				}
			}
		}
	}
}

void netw::read_overlay(std::string file_overlay_nodes)
{
	/**
	 * Prepare overlay information
	 * */
	loc_overlay_nodes.resize(n_nodes, false);
	// std::string file_overlay_nodes = "/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/overlay.txt";
	std::ifstream infile_onodes(file_overlay_nodes);
	std::string temp;
    std::string line;
	int n_onodes;
	getline(infile_onodes, line);
	std::istringstream iss(line);
	iss >> n_onodes >> temp;
	n_overlay_nodes = n_onodes;
	if (infile_onodes.is_open())
	{
		std::cout << "overlay_file: " << file_overlay_nodes << std::endl;
	}
	uint32_t idx_node;
	for (int i = 0; i < n_onodes; i++)
	{
		iss >> idx_node;
		loc_overlay_nodes[idx_node] = true;
	}
	tunnel_vec.resize(n_overlay_nodes*(n_overlay_nodes-1));
	switch (probe_type)
	{
		case ProbeType::naive:
		{
			// cnt_queuing.resize(n_overlay_nodes*(n_overlay_nodes-1));
			// for (uint32_t i = 0; i < n_overlay_nodes*(n_overlay_nodes-1); i++)
			// {
			// 	cnt_queuing[i].resize( _MAXPKTNUM, false );
			// }
			break;
		}
		case ProbeType::sandwich_v1:
		{
			break;
		}
		default:
		{
			std::cout << "Wrong Probe Type Setting" << std::endl;
			exit(-1);
			break;
		}
	}
	
}
	

netw::~netw()
{
    w.clear();
	bw.clear();
	delay.clear();
	edges.clear();
}

void netw::read_routing_map(std::string filename)
{
	std::ifstream infile(filename);
    std::string temp;
    std::string line;
	int src, dest, n_route_nodes;
	uint32_t idx_iter = 0;

	if (infile.is_open())
	{
		std::cout << "route_table: " << filename << std::endl;
		while (getline(infile, line))
		{
			std::istringstream iss(line);
			iss >> src >> dest >> temp >> n_route_nodes >> temp;
			std::string key {std::to_string(src) + " " + std::to_string(dest)};
			std::vector<int> val (n_route_nodes);
			for (int i = 0; i < n_route_nodes; i++)
			{
				iss >> val[i];
			}
			routing_map.insert( std::pair<std::string, std::vector<int>>(key, val) );

			tunnel_hashmap.insert( std::pair<std::string, uint32_t>(key, idx_iter) );
			tunnel_vec[idx_iter] = std::pair<int, int>(src, dest);
			probe_normal_interval[idx_iter] = 0;
			cnt_pkt.insert( std::pair<std::string, uint32_t>(key, 0) );
			cnt_congestion.insert( std::pair<std::string, uint32_t>(key, 0) );
			++idx_iter;
		}
		cnt_queuing.resize(tunnel_vec.size());
		for (uint32_t i = 0; i < tunnel_vec.size(); i++)
		{
			cnt_queuing[i].resize( _MAXPKTNUM, false );
		}
	}
}

void netw::read_demands(std::string filename)
{
	std::ifstream infile(filename);
    std::string line;
	int src, dest;
	float demand_val;
	std::cout << "read_demands: " << filename << std::endl;

	while (getline(infile, line))
	{
		std::istringstream iss(line);
		iss >> src >> dest >> demand_val;
		cnt_pkt.insert( std::pair<std::string, uint32_t>(std::to_string(src) + " " + std::to_string(dest), 0) );
		cnt_congestion.insert( std::pair<std::string, uint32_t>(std::to_string(src) + " " + std::to_string(dest), 0) );
	}
}
void netw::read_probe_profile(std::string filename)
{
	std::ifstream infile(filename);
    std::string line;
	int src, dest;
	double probe_interval; //microsecond (us, 1e-6) 
	std::cout << "read_probe_interval: " << filename << std::endl;
	uint32_t idx = 0;

	while (getline(infile, line))
	{
		std::istringstream iss(line);
		iss >> src >> dest >> probe_interval;
		probe_normal_interval[idx] = probe_interval;
		idx++;
	}
}

void netw::set_background_type(CrossType type_name)
{
	background_type = type_name;
}

}