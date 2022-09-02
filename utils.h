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
#include <unistd.h>

using namespace ns3;

#define SRC 17
#define DEST 3

typedef struct name_files
{
    std::string netw_filename;
    std::string demands_file;
    std::string file_overlay_nodes;
    std::string route_name;
    std::string probe_setup_filename;
    std::string probe_interval_files;
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


#endif