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
		.SetParent<Object> ();
		// .AddConstructor<netw> (name_input_files);
	return tid;
}
TypeId netw::GetInstanceTypeId (void) const
{
  	return netw::GetTypeId ();
}

netw::netw(std::string filename, std::string demands_file, std::string file_overlay_nodes, std::string route_name)
{
	set_background_type(CrossType::PktPoisson);
	// probe_type = ProbeType::naive;
	read_underlay(filename);
    read_overlay(file_overlay_nodes);
	read_routing_map(route_name);
	read_demands(demands_file);
}
netw::netw(name_input_files &fd_setup)
{
	set_background_type(CrossType::PktPoisson);
	// probe_type = ProbeType::naive;
	read_underlay(fd_setup.netw_filename);
    read_overlay(fd_setup.file_overlay_nodes);
	read_routing_map(fd_setup.route_name);
	read_demands(fd_setup.demands_file);
	read_probe_profile(fd_setup.probe_setup_filename);
	read_probe_intervals(fd_setup.probe_interval_files);
	read_gnb_coordinate(fd_setup.gnb_coordinate_files);
	read_hyper_param(fd_setup.hyper_param_files);
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
					m_sent.resize(n_nodes);
					for (uint32_t i = 0; i < n_nodes; i++)
					{
						adj_mat[i].resize(n_nodes, false);
						m_sent[i].resize(n_nodes, 0);
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
						/* adj_mat[src][dest] = true;
						adj_mat[dest][src] = true; */
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
	tunnel_vec.reserve(n_overlay_nodes*(n_overlay_nodes-1));
	/* switch (probe_type)
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
	} */
	
}

void netw::read_gnb_coordinate(std::string filename)
{
	vec_gnb_coordinate_.resize(n_overlay_nodes);
	std::ifstream infile(filename);
    std::string line;
	if (infile.is_open())
	{
		std::cout << "coordinate_file: " << filename << std::endl;
		for (uint32_t j = 0; j < 2; j++)
		{
			getline(infile, line);
			if (!line.empty() && j == 0)
			{
				std::istringstream iss(line);
				for (uint32_t i = 0; i < n_overlay_nodes; i++)
				{
					iss >> vec_gnb_coordinate_[i].x_val;
				}
			}
			if (!line.empty() && j == 1)
			{
				std::istringstream iss(line);
				for (uint32_t i = 0; i < n_overlay_nodes; i++)
				{
					iss >> vec_gnb_coordinate_[i].y_val;
				}
			}
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
			tunnel_vec.emplace_back( std::pair<int, int>(src, dest) );
			// probe_normal_interval[idx_iter] = 0;
			// cnt_pkt.insert( std::pair<std::string, uint32_t>(key, 0) );
			// cnt_congestion.insert( std::pair<std::string, uint32_t>(key, 0) );
			++idx_iter;
		}
		probe_normal_interval.resize(tunnel_hashmap.size());
		// cnt_queuing.resize(tunnel_vec.size());
		old_E.resize(tunnel_hashmap.size(), false);
		// for (uint32_t i = 0; i < tunnel_vec.size(); i++)
		// {
		// 	cnt_queuing[i].resize( _MAXPKTNUM, false );
		// }
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
	// int src, dest;
	// double probe_interval; //microsecond (us, 1e-6) 
	std::string temp;
	std::cout << "read_probe_setup: " << filename << std::endl;
	uint32_t n_old_E, e_idx;

	while (getline(infile, line))
	{
		std::istringstream iss(line);
		if (line.substr(0, 13).compare("E_cur_idxlist") == 0)
		{
			iss >> temp >> n_old_E;
			for (uint32_t i = 0; i < n_old_E; i++)
			{
				iss >> e_idx;
				if (e_idx == -1)
				{
					continue;
				}
				
				old_E[e_idx] = true;
				// std::cout << "n_old_E: " << n_old_E << "-" << temp << e_idx << std::endl;
				std::string keys_ = std::to_string(tunnel_vec[e_idx].first) + " " + std::to_string(tunnel_vec[e_idx].second);
				cnt_queuing.insert( std::pair<std::string, std::vector<bool>>(keys_, std::vector<bool>(_MAXPKTNUM)) );
				cnt_delays.insert( std::pair<std::string, std::vector<uint64_t>>(keys_, std::vector<uint64_t>(_MAXPKTNUM)) );
				cnt_true_delays.insert( std::pair<std::string, std::vector<uint64_t>>(keys_, std::vector<uint64_t>(_MAXPKTNUM)) );
				is_received.insert( std::pair<std::string, bool> ( keys_, true ) );
			}
		}
		else if (line.substr(0, 9).compare("e_new_idx") == 0)
		{
			iss >> temp >> e_idx;
			new_e = e_idx;
		}
		else if (line.substr(0, 10).compare("probe_type") == 0)
		{
			std::string str_probe_type;
			iss >> temp >> str_probe_type;
			if( str_probe_type.substr(0,5).compare("naive") == 0 ) probe_type = ProbeType::naive;
			else if ( str_probe_type.substr(0,11).compare("sandwich_v1") == 0 ) probe_type = ProbeType::sandwich_v1;
			else if ( str_probe_type.substr(0,11).compare("calibration") == 0 ) probe_type = ProbeType::calibration;
		}
		else if (line.substr(0, 14).compare("len_long_train") == 0)
		{
			iss >> temp >> len_long_train;
		}
		else if (line.substr(0, 10).compare("n_uePerGnb") == 0)
		{
			iss >> temp >> n_uePerGnb;
		}
		else if (line.substr(0, 8).compare("n_probes") == 0)
		{
			iss >> temp >> _MAXPKTNUM;
		}
	}
	// uint32_t s, t;
	for (auto it = cnt_delays.begin(); it != cnt_delays.end(); it ++)
	{
		// s = tunnel_vec[ tunnel_hashmap[it->first] ].first;
		// t = tunnel_vec[ tunnel_hashmap[it->first] ].second;
		std::vector<int> route = routing_map[it->first];
		// std::cout << "it->first:" << it->first;
		for (uint32_t i = 0; i < route.size()-1; i++)
		{
			// adj_mat[src][dest] = true;
			adj_mat[route[i]][route[i+1]] = true;
			// std::cout << route[i] << "-";
		}
		// std::cout <<  route[route.size()-1] << std::endl;
	}
	
}

void netw::read_probe_intervals(std::string filename)
{
	std::ifstream infile(filename);
    std::string line;
	int src, dest;
	double interval_val;
	std::cout << "read_probe_intervals: " << filename << std::endl;

	while (getline(infile, line))
	{
		std::istringstream iss(line);
		iss >> src >> dest >> interval_val;
		uint32_t idx_tunnel = tunnel_hashmap[std::to_string(src) + " " + std::to_string(dest)];
		probe_normal_interval[idx_tunnel] = round(interval_val);
	}
	uint32_t min_bw = *std::min_element(bw.begin(), bw.end());
	send_interval_probing = (long double)(ProbeSizeNaive*8*USTOS)/ (long double)(min_bw*1000) * 1000;
}

void netw::read_hyper_param(std::string filename)
{
	std::ifstream infile(filename);
    std::string line;
	// int src, dest;
	// double probe_interval; //microsecond (us, 1e-6) 
	std::string temp;
	std::cout << "read_hyper_param: " << filename << std::endl;
	uint32_t n_old_E, e_idx;

	while (getline(infile, line))
	{
		std::istringstream iss(line);
		if (line.substr(0, 13).compare("E_cur_idxlist") == 0)
		{
			iss >> temp >> n_old_E;
			for (uint32_t i = 0; i < n_old_E; i++)
			{
				iss >> e_idx;
				old_E[e_idx] = true;
				// std::cout << "n_old_E: " << n_old_E << "-" << temp << e_idx << std::endl;
				std::string keys_ = std::to_string(tunnel_vec[e_idx].first) + " " + std::to_string(tunnel_vec[e_idx].second);
				cnt_queuing.insert( std::pair<std::string, std::vector<bool>>(keys_, std::vector<bool>(_MAXPKTNUM)) );
				cnt_delays.insert( std::pair<std::string, std::vector<uint64_t>>(keys_, std::vector<uint64_t>(_MAXPKTNUM)) );
				cnt_true_delays.insert( std::pair<std::string, std::vector<uint64_t>>(keys_, std::vector<uint64_t>(_MAXPKTNUM)) );
				is_received.insert( std::pair<std::string, bool> ( keys_, true ) );
			}
		}
	}
}

void netw::set_background_type(CrossType type_name)
{
	background_type = type_name;
}

void netw::write_queuing_cnt(std::string filename)
{
	std::ofstream wrfile(filename);
	std::string key;
	for (auto it = cnt_queuing.begin(); it != cnt_queuing.end(); it++)
	{
		key = it->first;
		wrfile << std::to_string( tunnel_vec[ tunnel_hashmap[key] ].first ) << ", " << std::to_string( tunnel_vec[ tunnel_hashmap[key] ].second );
		for (auto val : cnt_queuing[key])
		{
			wrfile << ", " << std::to_string(val);
		}
		wrfile << std::endl;
	}
}
void netw::write_delays_cnt(std::string filename)
{
	std::ofstream wrfile(filename);
	std::string key;
	for (auto it = cnt_delays.begin(); it != cnt_delays.end(); it++)
	{
		key = it->first;
		wrfile << std::to_string( tunnel_vec[ tunnel_hashmap[key] ].first ) << ", " << std::to_string( tunnel_vec[ tunnel_hashmap[key] ].second );
		for (auto val : cnt_delays[key])
		{
			wrfile << ", " << std::to_string(val);
		}
		wrfile << std::endl;
	}
}
void netw::write_true_delays_cnt(std::string filename)
{
	std::ofstream wrfile(filename);
	std::string key;
	for (auto it = cnt_true_delays.begin(); it != cnt_true_delays.end(); it++)
	{
		key = it->first;
		wrfile << std::to_string( tunnel_vec[ tunnel_hashmap[key] ].first ) << ", " << std::to_string( tunnel_vec[ tunnel_hashmap[key] ].second );
		for (auto val : cnt_true_delays[key])
		{
			wrfile << ", " << std::to_string(val);
		}
		wrfile << std::endl;
	}
}

}