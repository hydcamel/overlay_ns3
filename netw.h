#ifndef NET_W_H
#define NET_W_H
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "utils.h"
// #include "overlayApplication.h"

// #define AppPktSize 1024
// #define IPPktSize 1052
// #define MACPktSize 1054
#define LISTENPORT 9
// #define MAXPKTNUM 500
#define NSTOMS 1000000
#define USTOS 1000000
#define NSTOUS 1000
// #define MAXBACKLOG 200
#define LBPKTSIZE (50.0)
#define UBPKTSIZE (1470.0)
#define MEDPKTSIZE (576.0)
// #define AVGPKTSIZE ((0.4)*LBPKTSIZE + (0.4)*UBPKTSIZE + (0.2)*MEDPKTSIZE)
#define BGPERCENTAGE (0.3)
#define PrLBPkt (0.4)
#define PrUBPkt (0.4)
#define PrMEDPkt (0.2)
#define ProbeSizeNaive (50)
#define ProbeSizeSWSmall (50)
#define ProbeSizeSWlarge (1450)

namespace ns3
{

enum ProbeType
{
    naive = 1,
    sandwich_v1 = 2
};

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
    void write_queuing_cnt(std::string filename);
    // void register_vecApp(std::vector<Ptr<overlayApplication>>* input);
    void notify_pktLoss(uint32_t src, uint32_t dest, uint32_t valPktID);
    void set_background_type(std::string type_name);
    void update_log_sandwich_v1(uint32_t SourceID, uint32_t DestID, uint32_t LargeID, uint32_t PktID);

    std::vector<int> w, bw, delay;
    uint32_t _AppPktSize = 1024, _IPPktSize = 1052, _MACPktSize = 1054, _MAXPKTNUM = 3, _MAXBACKLOG = 200000;
    uint16_t protocol_number = 150;
    double avg_pktSize = PrLBPkt*LBPKTSIZE + PrUBPkt*UBPKTSIZE + PrMEDPkt*MEDPKTSIZE;
    std::string background_type;
    ProbeType probe_type;
    // std::vector<Ptr<overlayApplication>>* vec_app;
    //std::vector<int> src, dest;
    //std::map<std::pair<int, int>, int> edges;
    std::map<std::string, int> edges;
    std::vector<uint32_t> background_interval; // microseconds
    std::vector<std::pair<int, int>> edges_vec;
    std::vector<std::pair<uint32_t, uint32_t>> tunnel_vec;
    std::vector<std::vector<bool>> adj_mat;
    std::vector<bool> loc_overlay_nodes;
    std::vector<std::vector<uint32_t>> m_sent;
    uint32_t n_nodes,n_edges, n_overlay_nodes;
    std::map<std::string, std::vector<int>> routing_map;
    std::map<std::string, uint32_t> tunnel_hashmap;
    std::unordered_map<std::string, int32_t> cnt_pkt;
    std::unordered_map<std::string, int32_t> cnt_congestion;

    std::vector<std::vector<bool>>  cnt_queuing;
    std::unordered_map<std::string, std::vector<uint64_t>> log_sandwich_v1; // the length of the vector should be __MAXPKTNUM+1. for sandwithID.
};

//extern netw netw_meta;

}

#endif