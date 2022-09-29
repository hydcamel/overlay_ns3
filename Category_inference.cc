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
#include "mmwaveNR.h"
#include <vector>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Category_inference");

void stop_NR(std::vector<Ptr<NrHelper>> &vec_nr)
{
    std::cout << "NR dispose" << std::endl;
    for (uint32_t i = 0; i < vec_nr.size(); i++)
    {
        vec_nr[i]->Dispose();
    }
    
};

int main (int argc, char *argv[])
{
    bool logging = false;
    if (logging)
    {
        LogComponentEnable ("netw", LOG_LEVEL_INFO);
    }
    name_input_files fd_setup_wrap;
    read_setup (fd_setup_wrap);

    // netw netw_meta (fd_setup_wrap.netw_filename, fd_setup_wrap.demands_file, fd_setup_wrap.file_overlay_nodes, fd_setup_wrap.route_name);
    netw netw_meta (fd_setup_wrap);

    // set simulation time and mobility
    // double simTime = 1; // seconds
    // double udpAppStartTime = 0.4; //seconds
    // double stop_time = 4200.0; // microseconds
    double stop_time = 400.0; // microseconds
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
    // fact.Set("PacketSize", UintegerValue(netw_meta._AppPktSize));
    double AppStartTime = 21200;
    for (uint32_t i = 0; i < netw_meta.n_nodes; i++)
    {
        vec_app[i] = fact.Create<overlayApplication>();
        vec_app[i]->InitApp(&netw_meta, i, netw_meta._MAXPKTNUM);
        vec_app[i]->SetStartTime(MicroSeconds(AppStartTime));
        // vec_app[i]->SetStopTime(MilliSeconds(stop_time));
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
        links[i].DisableFlowControl();
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
            for (uint32_t l = 0; l < n_devices_perNode[k]; l++) NS_LOG_INFO( "device ID: " << l << " with address: " << linkIpv4[k]->GetAddress( l, 0 ).GetLocal() );
        } */
    }
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint32_t network_base_number = 20;
    // std::vector<Ptr<myNR>> vec_nr_app( vec_app.size() );
    std::vector<Ptr<NrHelper>> vec_NrHelper(vec_app.size());
    std::vector<Ptr<NrPointToPointEpcHelper>> vec_EpcHelper(vec_app.size());
    for (uint32_t i = 0; i < vec_NrHelper.size(); i++)
    {
        vec_NrHelper[i] = CreateObject<NrHelper> ();
    }
    
    std::vector<myNR> vec_nr_app(vec_app.size());
    // std::vector<coordinate> vec_gnb_coordinate( vec_app.size() );
    std::vector<coordinate> vec_ue_coordinate( vec_app.size() );
    // myNR testNR(vec_gnb_coordinate[2], vec_ue_coordinate[2], network_base_number, *(vec_app[2]), internet);
    for (uint32_t i = 0; i < vec_app.size(); i++)
    {
        // netw_meta.vec_gnb_coordinate_[i].x_val = i*100;
        // netw_meta.vec_gnb_coordinate_[i].y_val = i*10;
        vec_ue_coordinate[i].x_val = netw_meta.vec_gnb_coordinate_[i].x_val + 20;
        vec_ue_coordinate[i].y_val = netw_meta.vec_gnb_coordinate_[i].y_val;
        // myNR tmpNR(vec_gnb_coordinate[i], vec_ue_coordinate[i], network_base_number, *(vec_app[i]), internet);
        // vec_nr_app.push_back( tmpNR );
        vec_nr_app[i].init_myNR(netw_meta.vec_gnb_coordinate_[i], vec_ue_coordinate[i], network_base_number, *(vec_app[i]), internet, vec_NrHelper[i], vec_EpcHelper[i]);
        network_base_number += 7;
        // vec_nr_app[i].vec_ue_app[0]->SetStopTime(MilliSeconds(stop_time*5));
    }
    // Config::SetDefault ("/NodeList/*/DeviceList/*/$ns3::NrNetDevice/Mtu", UintegerValue (1500));
    
    // uint32_t NR_ID = 3;
    // std::cout << "NR ID: " << NR_ID << "-PGW ID: " << vec_nr_app[NR_ID].pgw->GetId() << std::endl;
    // NR_ID = 4;
    // std::cout << "NR ID: " << NR_ID << "-PGW ID: " << vec_nr_app[NR_ID].pgw->GetId() << std::endl;
    // Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), underlayNodes.Get(0), routingStream, Time::Unit::NS);
    /* uint32_t node_idx = 3;
    Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), underlayNodes.Get(node_idx), routingStream, Time::Unit::NS);
    for (uint32_t i = 0; i < underlayNodes.Get(node_idx)->GetNDevices(); i++)
    {
        std::cout << "device ID: " << i << " with address: " << underlayNodes.Get(node_idx)->GetObject<Ipv4> ()->GetAddress( i, 0 ).GetLocal() << std::endl;
    }
    Ptr<Node> tmpUE =  vec_nr_app[node_idx].ueNodes.Get(0);
    for (uint32_t i = 0; i < tmpUE->GetNDevices(); i++)
    {
        std::cout << "device ID: " << i << " with address: " << tmpUE->GetObject<Ipv4> ()->GetAddress( i, 0 ).GetLocal() << std::endl;
    }
    
    node_idx = 4;
    Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), underlayNodes.Get(node_idx), routingStream, Time::Unit::NS);
    for (uint32_t i = 0; i < underlayNodes.Get(node_idx)->GetNDevices(); i++)
    {
        std::cout << "device ID: " << i << " with address: " << underlayNodes.Get(node_idx)->GetObject<Ipv4> ()->GetAddress( i, 0 ).GetLocal() << std::endl;
    }
    tmpUE =  vec_nr_app[node_idx].ueNodes.Get(0);
    for (uint32_t i = 0; i < tmpUE->GetNDevices(); i++)
    {
        std::cout << "device ID: " << i << " with address: " << tmpUE->GetObject<Ipv4> ()->GetAddress( i, 0 ).GetLocal() << std::endl;
    } */

    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), vec_nr_app[3].pgw, routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), vec_nr_app[3].gNbNodes.Get(0), routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), vec_nr_app[3].epcHelper->GetSgwNode(), routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), vec_nr_app[4].pgw, routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), vec_nr_app[4].gNbNodes.Get(0), routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), vec_nr_app[4].epcHelper->GetSgwNode(), routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAllAt (Seconds (0.5), routingStream, Time::Unit::S);

    

    // Config::Connect( "/NodeList/*/$ns3::Ipv4L3Protocol/Tx", MakeCallback(&txTraceIpv4) );
    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx", MakeCallback(&p2pDevMacTx) );
    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacRx", MakeCallback(&p2pDevMacRx) );
    // // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTxDrop", MakeCallback(&p2pDevMacRx) );

    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyTxBegin", MakeCallback(&trace_PhyTxBegin) );
    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyTxEnd", MakeCallback(&trace_PhyTxEnd) );
    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyRxEnd", MakeCallback(&trace_PhyRxEnd) );


    NS_LOG_INFO("Run Simulation.");
    std::cout << "before run" << std::endl;
    Time time_stop_simulation = MilliSeconds(stop_time*1.2);
    Simulator::Schedule(time_stop_simulation, stop_NR, vec_NrHelper);
    Simulator::Stop(time_stop_simulation);
    Simulator::Run();
    // flow_monitor->SerializeToXmlFile("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/flow_monitor_res.xml", true, true);
    
    std::cout << "Before Destroy." << std::endl;
    char *cwd = get_current_dir_name();
    std::string pwd_tmp(cwd, cwd+strlen(cwd));
    // netw_meta.write_queuing_cnt(pwd_tmp + "/scratch/Category_inference/queuing_cnt.csv");
    netw_meta.write_delays_cnt(pwd_tmp + "/scratch/Category_inference/delays_cnt.csv");
    // netw_meta.write_true_delays_cnt(pwd_tmp + "/scratch/Category_inference/true_delays_cnt.csv");
    std::cout << "start Destroy." << std::endl;
    Simulator::Destroy();
    NS_LOG_INFO("Done.");
}
