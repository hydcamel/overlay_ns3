#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/application.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/nr-mac-scheduler-tdma-rr.h"
#include "ns3/nr-module.h"
#include <ns3/antenna-module.h>
#include "utils.h"
#include "netw.h"
#include "overlayApplication.h"
#include <vector>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Category_inference");

int main(int argc, char *argv[])
{
    name_input_files fd_setup_wrap;
    read_setup(fd_setup_wrap);

    netw netw_meta(fd_setup_wrap.netw_filename, fd_setup_wrap.demands_file, fd_setup_wrap.file_overlay_nodes, fd_setup_wrap.route_name);
}