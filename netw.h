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
    netw(std::string filename);
    ~netw();
    void read_routing_map(std::string filename);
    void read_underlay(std::string filename);
    void read_overlay();

    std::vector<int> w, bw, delay;
    //std::vector<int> src, dest;
    std::map<std::pair<int, int>, int> edges;
    std::vector<std::pair<int, int>> edges_vec;
    std::vector<std::vector<bool>> adj_mat;
    std::vector<bool> loc_overlay_nodes;
    uint32_t n_nodes,n_edges;
    std::map<std::string, std::vector<int>> routing_map;
};

//extern netw netw_meta;

}

#endif