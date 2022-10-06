#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/udp-socket.h"
#include "ns3/log.h"
#include <math.h>
#include <unistd.h>

namespace ns3
{

#define SRC 17
#define DEST 3
#define ISNR true

typedef struct name_files
{
    std::string netw_filename;
    std::string demands_file;
    std::string file_overlay_nodes;
    std::string route_name;
    std::string probe_setup_filename;
    std::string probe_interval_files;
    std::string gnb_coordinate_files;
    std::string hyper_param_files;
    std::string nUE_filename;
}name_input_files;

typedef struct coordinate
{
    double x_val;
    double y_val;
}coordinate;

/**
 * read and write utils
 **/
void read_setup(name_input_files &input_fd);

void txTraceIpv4(std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ptr_ipv4, uint32_t dontknow);
void p2pDevMacTx(std::string context, Ptr<const Packet> packet);
void p2pDevMacRx(std::string context, Ptr<const Packet> packet);
// void trace_udpClient(std::string context, Ptr<const Packet> packet);
void trace_PhyTxBegin(std::string context, Ptr<const Packet> packet);
void trace_PhyTxEnd(std::string context, Ptr<const Packet> packet);
// void trace_PhyRxBegin(std::string context, Ptr<const Packet> packet);
void trace_PhyRxEnd(std::string context, Ptr<const Packet> packet);

}



#endif