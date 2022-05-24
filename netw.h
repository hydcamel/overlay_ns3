#ifndef NET_W_H
#define NET_W_H
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "ns3/ptr.h"
// #include "overlayApplication.h"

#define AppPktSize 1024
#define IPPktSize 1052
#define MACPktSize 1054
#define LISTENPORT 9
#define MAXPKTNUM 500
#define NSTOMS 1000000
#define NSTOUS 1000
#define MAXBACKLOG 200

namespace ns3
{


class netw
{
public:
    netw(std::string filename, std::string demands_file);
    netw(std::string filename, std::string demands_file, std::string file_overlay_nodes, std::string route_name);
    ~netw();
    void read_routing_map(std::string filename);
    void read_underlay(std::string filename);
    void read_overlay(std::string file_overlay_nodes);
    void read_demands(std::string filename);
    void write_average_delay(std::string filename);
    void write_congestion_cnt(std::string filename);
    // void register_vecApp(std::vector<Ptr<overlayApplication>>* input);
    void notify_pktLoss(uint32_t src, uint32_t dest, uint32_t valPktID);

    std::vector<int> w, bw, delay;
    // std::vector<Ptr<overlayApplication>>* vec_app;
    //std::vector<int> src, dest;
    //std::map<std::pair<int, int>, int> edges;
    std::map<std::string, int> edges;
    std::vector<std::pair<int, int>> edges_vec;
    std::vector<std::pair<int, int>> demands_vec;
    std::vector<std::vector<bool>> adj_mat;
    std::vector<bool> loc_overlay_nodes;
    std::vector<std::vector<uint32_t>> m_sent;
    uint32_t n_nodes,n_edges;
    std::map<std::string, std::vector<int>> routing_map;
    std::map<std::string, float> overlay_demands;
    std::map<std::string, double> average_delay;
    std::unordered_map<std::string, double> time_span_flows;
    std::unordered_map<std::string, int32_t> cnt_pkt;
    std::unordered_map<std::string, int32_t> cnt_congestion;
};

//extern netw netw_meta;

}

#endif