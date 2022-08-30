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

NS_LOG_COMPONENT_DEFINE ("Category_inference");

int main (int argc, char *argv[])
{
    bool logging = false;
    bool ran = false;
    if (logging)
    {
        LogComponentEnable ("netw", LOG_LEVEL_INFO);
    }
    name_input_files fd_setup_wrap;
    read_setup (fd_setup_wrap);

    netw netw_meta (fd_setup_wrap.netw_filename, fd_setup_wrap.demands_file, fd_setup_wrap.file_overlay_nodes, fd_setup_wrap.route_name);

    // set simulation time and mobility
    double simTime = 1; // seconds
    double udpAppStartTime = 0.4; //seconds
    double stop_time = 100.0; // seconds

    //RAN-related simulation parameters default values
    if (ran)
    {
        uint16_t numerology = 0;

        uint16_t gNbNum = 1;
        uint16_t ueNumPergNb = 1;

        double centralFrequency = 7e9;
        double bandwidth = 100e6;
        double txPower = 14;
        double lambda = 1000;
        uint32_t udpPacketSize = 1000;
        bool udpFullBuffer = true;
        uint8_t fixedMcs = 28;
        bool useFixedMcs = true;
        bool singleUeTopology = true;
        // Where we will store the output files.
        std::string simTag = "default";
        std::string outputDir = "./";
    }
    /**
     * Underlay Network
     *
     */
    NodeContainer underlayNodes;
    underlayNodes.Create(netw_meta.n_nodes);
    InternetStackHelper internet;
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    internet.Install(underlayNodes);
    /**
     * Install Applications
     **/
    std::vector<Ptr<overlayApplication>> vec_app(netw_meta.n_nodes);
    ObjectFactory fact;
    fact.SetTypeId("ns3::overlayApplication");
    fact.Set("RemotePort", UintegerValue(LISTENPORT));
    fact.Set("ListenPort", UintegerValue(LISTENPORT));
    fact.Set("probe_interval", TimeValue(MicroSeconds(200.0)));
    fact.Set("sandwich_interval", TimeValue(MicroSeconds(100.0))); // Interval between the first and second patch of the sandwich
    // fact.Set ("MaxPackets", UintegerValue (1));
    fact.Set("PacketSize", UintegerValue(netw_meta._AppPktSize));
    for (uint32_t i = 0; i < netw_meta.n_nodes; i++)
    {
        vec_app[i] = fact.Create<overlayApplication>();
        vec_app[i]->InitApp(&netw_meta, i, netw_meta._MAXPKTNUM);
        vec_app[i]->SetStartTime(Seconds(0));
        vec_app[i]->SetStopTime(Seconds(stop_time));
        underlayNodes.Get(i)->AddApplication(vec_app[i]);
        vec_app[i]->SetRecvSocket();
    }
}
