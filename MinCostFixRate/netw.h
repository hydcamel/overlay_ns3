#ifndef NET_W_H
#define NET_W_H
#include <string>
#include <vector>
#include <map>

namespace ns3
{


class netw
{
public:
    netw();
    netw(std::string filename);
    ~netw();
    void read_routing_map(std::string filename);

    std::vector<int> src, dest, w, bw, delay;
    std::vector<std::vector<bool>> adj_mat;
    std::vector<int> loc_overlay_nodes;
    uint32_t n_nodes,n_edges;
    std::map<std::string, std::vector<int>> routing_map;
};



}

#endif