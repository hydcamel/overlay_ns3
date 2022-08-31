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

    std::vector<PointToPointHelper> links(netw_meta.delay.size());
    std::vector<NetDeviceContainer> NetDevices(netw_meta.delay.size());
    std::vector<Ptr<Ipv4>> linkIpv4(2);
    std::vector<Ipv4InterfaceAddress> linkIpv4Addr(2);
    std::vector<uint32_t> n_devices_perNode(2);
    std::vector<Ptr<Node>> endnodes(2);

    for (uint32_t i = 0; i < links.size(); i++)
    {
        links[i].SetChannelAttribute("Delay", StringValue(std::to_string(netw_meta.delay[i]) + "us"));
        links[i].SetDeviceAttribute("DataRate", StringValue(std::to_string(netw_meta.bw[i]) + "kbps"));
        links[i].SetQueue("ns3::DropTailQueue", "MaxSize", QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, netw_meta._MAXBACKLOG)));
        // links[i].;
        NetDevices[i] = links[i].Install(underlayNodes.Get(netw_meta.edges_vec[i].first), underlayNodes.Get(netw_meta.edges_vec[i].second));
        address.Assign(NetDevices[i]);
        address.NewNetwork();

        endnodes[0] = underlayNodes.Get(netw_meta.edges_vec[i].first);
        endnodes[1] = underlayNodes.Get(netw_meta.edges_vec[i].second);
        for (int k = 0; k < 2; k++)
        {
            linkIpv4[k] = endnodes[k]->GetObject<Ipv4>();
            n_devices_perNode[k] = endnodes[k]->GetNDevices();
            linkIpv4Addr[k] = linkIpv4[k]->GetAddress(n_devices_perNode[k] - 1, 0);
        }
        vec_app[netw_meta.edges_vec[i].first]->SetSocket(linkIpv4Addr[1].GetAddress(), netw_meta.edges_vec[i].second, n_devices_perNode[0] - 1);
        // std::cout << netw_meta.edges_vec[i].first << ": " << linkIpv4Addr[1].GetAddress() << std::endl;
        vec_app[netw_meta.edges_vec[i].second]->SetSocket(linkIpv4Addr[0].GetAddress(), netw_meta.edges_vec[i].first, n_devices_perNode[1] - 1);
        // std::cout << netw_meta.edges_vec[i].second << ": " << linkIpv4Addr[0].GetAddress() << std::endl;

        /* for (int k = 0; k < 2; k++)
        {
            std::cout << "Node ID = " << NetDevices[i].Get(k)->GetNode()->GetId() << "; ";
            linkIpv4[k] = NetDevices[i].Get(k)->GetNode()->GetObject<Ipv4> ();
            n_devices_perNode[k] = NetDevices[i].Get(k)->GetNode()->GetNDevices();
            linkIpv4Addr[k] = linkIpv4[k]->GetAddress( n_devices_perNode[k]-1, 0 );
            std::cout << "Address = " << linkIpv4Addr[k].GetLocal() << std::endl;
            for (uint32_t l = 0; l < n_devices_perNode[k]; l++)
            {
                std::cout << "device ID: " << l << " with address: " << linkIpv4[k]->GetAddress( l, 0 ).GetLocal() << std::endl;
            }
        } */
    }
}
