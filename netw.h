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
enum CrossType
{
    PktPoisson = 1
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

    /**
     * underlay network
     **/
    std::vector<int> w, bw, delay;
    std::map<std::string, int> edges; // hash_map for links
    std::vector<std::pair<int, int>> edges_vec; // vector  for links
    uint32_t n_nodes,n_edges;

    /**
     * overlay network
     **/
    uint32_t n_overlay_nodes;
    std::vector<std::pair<uint32_t, uint32_t>> tunnel_vec;
    std::vector<std::vector<bool>> adj_mat;
    std::vector<bool> loc_overlay_nodes;
    std::map<std::string, std::vector<int>> routing_map;
    std::map<std::string, uint32_t> tunnel_hashmap; // hash_map for tunnels

    /**
     * probing 
     **/
    uint32_t _AppPktSize = 1024, _IPPktSize = 1052, _MACPktSize = 1054, _NormPktSize = 46, _MAXPKTNUM = 3, _MAXBACKLOG = 200000;
    uint16_t protocol_number = 150;
    std::vector<double> probe_normal_interval;
    double avg_pktSize = PrLBPkt*LBPKTSIZE + PrUBPkt*UBPKTSIZE + PrMEDPkt*MEDPKTSIZE;
    std::vector<uint32_t> background_interval; // microseconds
    void set_background_type(CrossType);
    // std::vector<std::vector<uint32_t>> m_sent;

    /**
     * data collection
     **/
    std::unordered_map<std::string, int32_t> cnt_pkt;
    std::unordered_map<std::string, int32_t> cnt_congestion; 
    std::vector<std::vector<bool>>  cnt_queuing; // # of queued

    ProbeType probe_type;
    CrossType background_type;
private:
};

}

#endif