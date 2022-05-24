#include <fstream>
#include <sstream>
#include <iostream>
#include "netw.h"


namespace ns3
{

//extern netw netw_meta;

//netw netw_meta;

netw::netw(std::string filename, std::string demands_file)
{
	read_underlay(filename);
    read_overlay("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/overlay.txt");
	read_routing_map("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/route_table.txt");
	read_demands(demands_file);
}

netw::netw(std::string filename, std::string demands_file, std::string file_overlay_nodes, std::string route_name)
{
	read_underlay(filename);
    read_overlay(file_overlay_nodes);
	read_routing_map(route_name);
	read_demands(demands_file);
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
		}
	}
}

void netw::read_demands(std::string filename)
{
	std::ifstream infile(filename);
    std::string line;
	int src, dest;
	float demand_val;

	while (getline(infile, line))
	{
		std::istringstream iss(line);
		iss >> src >> dest >> demand_val;
		overlay_demands.insert( std::pair<std::string, float>(std::to_string(src) + " " + std::to_string(dest), demand_val) );
		average_delay.insert( std::pair<std::string, double>(std::to_string(src) + " " + std::to_string(dest), 0) );
		time_span_flows.insert( std::pair<std::string, double>(std::to_string(src) + " " + std::to_string(dest), 0) );
		demands_vec.emplace_back( std::pair<int, int>(src, dest) );
		cnt_pkt.insert( std::pair<std::string, uint32_t>(std::to_string(src) + " " + std::to_string(dest), 0) );
		cnt_congestion.insert( std::pair<std::string, uint32_t>(std::to_string(src) + " " + std::to_string(dest), 0) );
	}
}

void netw::write_average_delay(std::string filename)
{
	std::ofstream wrfile(filename);
	std::string key;
	//uint16_t n_bits = MACPktSize * MAXPKTNUM * 8;
	/* for (uint16_t i = 0; i < demands_vec.size(); i++)
	{
		key = std::to_string(demands_vec[i].first) + ' ' + std::to_string(demands_vec[i].second);
		std::cout << time_span_flows[key] << " ";
	} */
	std::cout << std::endl;
	for (uint16_t i = 0; i < demands_vec.size(); i++)
	{
		key = std::to_string(demands_vec[i].first) + ' ' + std::to_string(demands_vec[i].second);
		//wrfile << key << " " << std::to_string( double(n_bits) / (double(average_delay[key]) / 1000000000) ) << std::endl;
		//wrfile << key << " " << std::to_string( (double(average_delay[key]) / 1000000000) / double(n_bits) ) << std::endl;
		wrfile << key << " " << std::to_string( time_span_flows[key] ) << std::endl;
	}
	
}

void netw::write_congestion_cnt(std::string filename)
{
	std::ofstream wrfile(filename);
	std::string key;
	for (uint16_t i = 0; i < demands_vec.size(); i++)
	{
		key = std::to_string(demands_vec[i].first) + ' ' + std::to_string(demands_vec[i].second);
		//wrfile << key << " " << std::to_string( double(n_bits) / (double(average_delay[key]) / 1000000000) ) << std::endl;
		//wrfile << key << " " << std::to_string( (double(average_delay[key]) / 1000000000) / double(n_bits) ) << std::endl;
		wrfile << key << " " << std::to_string( cnt_congestion[key] ) << std::endl;
	}
	
}

// void netw::register_vecApp(std::vector<Ptr<overlayApplication>>* input)
// {
// 	vec_app = input;
// }

void netw::notify_pktLoss(uint32_t src, uint32_t dest, uint32_t valPktID)
{
	m_sent[src][dest] = valPktID;
	// (*vec_app)[src]->SetMSent(dest, valPktID);
}

}


