#ifndef NET_W_H
#define NET_W_H
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "utils.h"
#include <algorithm>    // std::min_element, std::max_element

#define LISTENPORT 9
// #define MAXPKTNUM 500
#define NSTOMS 1000000
#define NSTOS 1000000000
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
#define ProbeSizeNaive (1460)
// #define ProbeSizeNaive (600)
// #define ProbeSizeNaive (50)
#define ProbeSizeSWSmall (50)
#define ProbeSizeSWlarge (1450)

namespace ns3
{

enum ProbeType
{
    naive = 1,
    sandwich_v1 = 2,
    calibration = 3,
    poisson = 4
};
enum CrossType
{
    PktPoisson = 1,
    ParetoBurst = 2
};

class netw
{
public:
    static TypeId GetTypeId(void);
	virtual TypeId GetInstanceTypeId(void) const;
    netw(std::string filename, std::string demands_file, std::string file_overlay_nodes, std::string route_name);
    netw(name_input_files &fd_setup);
    ~netw();
    void read_routing_map(std::string filename);
    void read_underlay(std::string filename);
    void read_overlay(std::string file_overlay_nodes);
    void read_demands(std::string filename); 
    void read_probe_profile(std::string filename); // for probing, this will contain the probing rate profile: microsecond (us, 1e-6) 
    void read_probe_intervals(std::string filename); // for probing, this will contain the probing rate profile: microsecond (us, 1e-6) 
    void read_gnb_coordinate(std::string filename);
    void read_hyper_param(std::string filename);
    void read_n_UE(std::string filename);

    void write_queuing_cnt(std::string filename);
    void write_delays_cnt(std::string filename);
    void write_true_delays_cnt(std::string filename);

    /**
     * underlay network
     **/
    std::vector<int> w, bw, delay;
    std::map<std::string, int> edges; // hash_map for links
    std::vector<std::pair<int, int>> edges_vec; // vector  for links
    uint32_t n_nodes,n_edges;
    std::vector<coordinate> vec_gnb_coordinate_;
    uint32_t n_uePerGnb;
    uint32_t min_bw;
    uint32_t avg_pkt_transmission_delay;
    std::vector<uint32_t> n_perUE;
    uint32_t distance_ue_from_gnb = 20;
    std::vector<int> pos_ue_x {1, -1, 0, 0, 1, -1, -1, 1}; // 0: 0, 1: '+', -1: '-'
    std::vector<int> pos_ue_y {0, 0, -1, 1, 1, 1, -1, -1};

    /**
     * overlay network
     **/
    uint32_t n_overlay_nodes;
    std::vector<std::pair<uint32_t, uint32_t>> tunnel_vec;
    std::vector<std::vector<bool>> adj_mat;
    std::vector<bool> loc_overlay_nodes;
    std::unordered_map<std::string, std::vector<int>> routing_map;
    std::unordered_map<std::string, uint32_t> tunnel_hashmap; // hash_map for tunnels

    /**
     * probing 
     **/
    uint32_t _AppPktSize = 1024, _IPPktSize = 1052, _MACPktSize = 1054, _NormPktSize = 46, _MAXPKTNUM = 3, _MAXBACKLOG = 2000000;
    uint32_t _epoll_time = 10; // microsecond
    uint16_t protocol_number = 150;
    std::vector<double> probe_normal_interval;
    uint32_t probe_param_interval; //ns
    uint32_t target_interval = 80000; // 40000ns = 200 mb/s
    std::vector<bool> old_E;
    uint32_t new_e;
    uint32_t len_long_train;
    double avg_pktSize = PrLBPkt*LBPKTSIZE + PrUBPkt*UBPKTSIZE + PrMEDPkt*MEDPKTSIZE;
    std::vector<uint32_t> background_interval; // microseconds
    void set_background_type(CrossType);
    std::vector<std::vector<uint32_t>> m_sent;
    std::unordered_map<std::string, bool> is_received;
    uint32_t idx_orchestration = 1;
    uint32_t pkt_size_ran = 1420;
    uint32_t send_interval_probing;
    double prob_burst = 0.002;
    uint32_t n_burst_pkt = 50;
    double parato_scale = 12;
    double parato_shape = 2.04;
    uint32_t parato_bound = 300;
    uint32_t probe_pkt_size = ProbeSizeNaive;
    bool is_w_probing = true;
    bool is_param_probing = true;
    std::unordered_set<uint32_t> set_tb;
    uint32_t tau_attack;
    // std::vector<uint32_t> cnt_node_received_pkt;
    // std::vector<uint32_t> cnt_node_attack_pkt;


    /**
     * data collection
     **/
    std::unordered_map<std::string, int32_t> cnt_pkt;
    std::unordered_map<std::string, int32_t> cnt_congestion; 
    std::unordered_map<std::string, std::vector<bool>>  cnt_queuing; // # of queued
    std::unordered_map<std::string, std::vector<uint64_t>>  cnt_delays; // # of queued
    std::unordered_map<std::string, std::vector<uint64_t>>  cnt_true_delays; // # of queued
    // std::vector<std::vector<bool>>  cnt_queuing; // # of queued

    ProbeType probe_type;
    CrossType background_type;
private:
};

}

#endif