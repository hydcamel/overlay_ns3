#include <fstream>
#include <sstream>
#include <iostream>
#include "netw.h"


namespace ns3
{


netw::netw(std::string filename)
{
    std::ifstream infile(filename);
    std::string temp;
    std::string line;
    int idx = 0;
	int src, dest;

	if (infile.is_open())
	{
		while (getline(infile, line))
		{
			if (!line.empty())
			{
				std::istringstream iss(line);
				if (line.substr(0, 5).compare("NODES") == 0)
				{
					iss >> temp >> n_nodes;
					adj_mat.resize(n_nodes);
					for (uint32_t i = 0; i < n_nodes; i++)
					{
						adj_mat[i].resize(n_edges, false);
					}
				}
				else if (line.substr(0, 5).compare("EDGES") == 0)
				{
					iss >> temp >> n_edges;
					/* src.resize(n_edges);
					dest.resize(n_edges); */
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
					/* iss >> temp >> src[idx] >> dest[idx] >> w[idx] >> bw[idx] >> delay[idx];
					adj_mat[src[idx]][dest[idx]] = true;
					adj_mat[dest[idx]][src[idx]] = true; */
					iss >> temp >> src >> dest >> w[idx] >> bw[idx] >> delay[idx];
					edges.insert( std::pair<std::pair<int, int>, int>(std::pair<int, int>(src, dest), idx) );
					edges_vec[idx] = std::pair<int, int> (src, dest);
					adj_mat[src][dest] = true;
					adj_mat[dest][src] = true;
					++idx;
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

	if (infile.is_open())
	{
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

}


