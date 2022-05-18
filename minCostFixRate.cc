#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
// #include "ns3/flow-monitor.h"
// #include "ns3/flow-monitor-helper.h"
#include "overlayApplication.h"
#include "ns3/application.h"
#include "utils.h"
#include "netw.h"
#include <vector>
#include <string>

//#include "wave-setup.h"
// token: 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
// std::globalInfo meta;

using namespace ns3;
// extern netw netw_meta;
// netw netw_meta;

NS_LOG_COMPONENT_DEFINE("p2pTestV1");

void read_setup(std::string& name_underlay)
{
    std::ifstream infile("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/setup.txt");
    std::string line;
    std::string temp;

    getline(infile, line);
    std::istringstream iss(line);
    iss >> temp;
    name_underlay = temp;
}

int main(int argc, char *argv[])
{
    // Log information
    LogComponentEnable("p2pTestV1", LOG_LEVEL_INFO);
    LogComponentEnable("overlayApplication", LOG_LEVEL_INFO);

    std::string name_shared_folder {"/vagrant/Documents/vagrant_shared_folder/"};
    std::string name_pwd {"/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/"};
    std::string name_underlay;

    // CommandLine cmd;
    // cmd.AddValue("name_underlay", "name of underlay in the form of .graph", name_underlay);
    // cmd.Parse (argc, argv);

    //std::string newt_filename{"/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/toy_one_junction.graph"};
    read_setup(name_underlay);
    std::string netw_filename = name_shared_folder + name_underlay;
    std::string demands_file = name_pwd + "tunnel_demands.txt";

    netw netw_meta(netw_filename, demands_file);

    
    InternetStackHelper internet;
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");

    /**
     * @brief Generate Underlay Network
     *
     */
    NodeContainer underlayNodes;
    underlayNodes.Create(netw_meta.n_nodes);
    internet.Install(underlayNodes);

    /**
     * Install Applications
     */
    std::vector<Ptr<overlayApplication>> vec_app(netw_meta.n_nodes);
    ObjectFactory fact;
    fact.SetTypeId("ns3::overlayApplication");
    fact.Set("RemotePort", UintegerValue(LISTENPORT));
    fact.Set("ListenPort", UintegerValue(LISTENPORT));
    // fact.Set ("MaxPackets", UintegerValue (1));
    fact.Set("PacketSize", UintegerValue(AppPktSize));
    for (uint32_t i = 0; i < netw_meta.n_nodes; i++)
    {
        vec_app[i] = fact.Create<overlayApplication>();
        vec_app[i]->InitApp(&netw_meta, i, MAXPKTNUM);
        vec_app[i]->SetStartTime(Seconds(0));
        vec_app[i]->SetStopTime(Seconds(1500));
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
        links[i].SetChannelAttribute("Delay", StringValue(std::to_string(netw_meta.delay[i]) + "ms"));
        links[i].SetDeviceAttribute("DataRate", StringValue(std::to_string(netw_meta.bw[i]) + "bps"));
        links[i].SetQueue("ns3::DropTailQueue", "MaxSize", QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, MAXBACKLOG)));
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
    /* for (uint32_t i = 0; i < netw_meta.n_nodes; i++)
    {
        std::cout << "Node ID: " << i << "=" << underlayNodes.Get(i)->GetId() << "; n_devices = " << underlayNodes.Get(i)->GetNDevices() << std::endl;
        for (uint32_t l = 0; l < underlayNodes.Get(i)->GetNDevices(); l++)
        {
            std::cout << "device ID: " << l << " with address: " << underlayNodes.Get(i)->GetObject<Ipv4>()->GetAddress(l, 0).GetLocal() << std::endl;
        }
    } */

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    /* Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>(&std::cout);
    Ipv4RoutingHelper::PrintRoutingTableAllAt( Seconds(0), routingStream, Time::Unit::S); */

    // for (ns3::ChannelList::Iterator it = ns3::ChannelList::Begin(); it != ns3::ChannelList::End(); it++){
    //     std::cout << it->
    // }
    // Ptr<Channel> it;
    // Time delay;
    // DataRateValue dr0, dr1;
    // //DataRate dr1 = 0, dr2 = 0;
    // for (int i = 0 ; i < 3; i++){
    //     it = ns3::ChannelList::GetChannel(i);
    //     // std::cout << "C_" << i << " delay: " << it->GetAttribute << "; src = " << std::endl;
    //     it->GetDevice(0)->GetAttribute("DataRate", dr0);
    //     it->GetDevice(1)->GetAttribute("DataRate", dr1);
    //     std::cout << "src = " << it->GetDevice(0)->GetAddress() << " DR = " << dr0.Get() << "; dest = " << it->GetDevice(1)->GetAddress() << " DR = " << dr1.Get() << std::endl;
    // }

    /**
     * Flow Monitor
     **/
    /* Ptr<FlowMonitor> flow_monitor;
    FlowMonitorHelper flow_helper;
    flow_monitor = flow_helper.InstallAll(); */

    // Config::Connect( "/NodeList/*/$ns3::Ipv4L3Protocol/Rx", MakeCallback(&rxTraceIpv4) );
    // Config::Connect( "/NodeList/*/$ns3::Ipv4L3Protocol/Tx", MakeCallback(&txTraceIpv4) );
    // Config::Connect( "/NodeList/*/$ns3::Ipv4L3Protocol/LocalDeliver", MakeCallback(&LocalDeliver) );

    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx", MakeCallback(&p2pDevMacTx) );
    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacRx", MakeCallback(&p2pDevMacRx) );

    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyTxBegin", MakeCallback(&trace_PhyTxBegin) );
    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyTxEnd", MakeCallback(&trace_PhyTxEnd) );
    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyRxEnd", MakeCallback(&trace_PhyRxEnd) );

    // Config::Connect( "/ChannelList/*/$ns3::PointToPointChannel/TxRxPointToPoint", MakeCallback(&trace_txrxPointToPoint) );
    // Config::Connect("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTxDrop", MakeCallback(&trace_NetDeviceMacTxDrop));
    // Config::Connect("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyTxDrop", MakeCallback(&trace_NetDevicePhyTxDrop));
    // Config::Connect("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyRxDrop", MakeCallback(&trace_NetDevicePhyRxDrop));
    // Config::Connect("/NodeList/*/$ns3::Ipv4L3Protocol/Drop", MakeCallback(&trace_Ipv4L3PDrop));

    NS_LOG_INFO("Run Simulation.");
    // std::cout << "before run" << std::endl;
    Simulator::Run();
    // flow_monitor->SerializeToXmlFile("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/flow_monitor_res.xml", true, true);
    Simulator::Destroy();
    NS_LOG_INFO("Done.");
}