#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "overlayApplication.h"
#include "ns3/application.h"
#include "utils.h"

//#include "wave-setup.h"
// token: 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
// std::globalInfo meta;

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("p2pTestV1");

int main(int argc, char *argv[])
{
    // Log information
    LogComponentEnable ("p2pTestV1", LOG_LEVEL_INFO);
    LogComponentEnable ("overlayApplication", LOG_LEVEL_INFO);

    CommandLine cmd;
    cmd.Parse (argc, argv);
    //LogComponentEnable ("utils", LOG_LEVEL_INFO);
    // Nodes creation
    uint32_t n_underlay = 2;
    uint32_t n_overlay = 2;

    // meta.initDemand(n_overlay);

    NodeContainer underlayNodes;
    NodeContainer overlayNodes;

    underlayNodes.Create(n_underlay);
    overlayNodes.Create(n_overlay);

    // uint32_t packetSize = 1024;
    // uint32_t packetCount = 2;
    // double packetInterval = 1.0;

    // Socket options for IPv4, currently TOS, TTL, RECVTOS, and RECVTTL
    // uint32_t ipTos = 0;
    // bool ipRecvTos = true;
    // uint32_t ipTtl = 0;
    // bool ipRecvTtl = true;

    // std::cout << "before device setup" << std::endl;


    // Device setup
    PointToPointHelper p2pNode1;
    p2pNode1.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2pNode1.SetChannelAttribute("Delay", StringValue("0ms"));
    NetDeviceContainer netDevLan1;
    netDevLan1 = p2pNode1.Install(overlayNodes.Get(0), underlayNodes.Get(0));

    PointToPointHelper p2pNode2;
    p2pNode2.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2pNode2.SetChannelAttribute("Delay", StringValue("0ms"));
    NetDeviceContainer netDevLan2;
    netDevLan2 = p2pNode2.Install(overlayNodes.Get(1), underlayNodes.Get(1));

    PointToPointHelper p2pBridge;
    p2pBridge.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    p2pBridge.SetChannelAttribute("Delay", StringValue("5ms"));
    NetDeviceContainer netDevBridge;
    netDevBridge = p2pBridge.Install(underlayNodes.Get(1), underlayNodes.Get(0));

    PointerValue ptr;
    netDevBridge.Get(0)->GetAttribute ("TxQueue", ptr);
    Ptr<Queue<Packet> > txQueue = ptr.Get<Queue<Packet> > ();
    QueueSizeValue limit;
    txQueue->GetAttribute ("MaxSize", limit);
    std::cout << "txQueue = " << limit.Get() << std::endl;

    // IP address setup
    InternetStackHelper internet;
    internet.Install(underlayNodes);
    internet.Install(overlayNodes);
    Ipv4AddressHelper address;
    // For LAN 1
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer lan1interfaces;
    lan1interfaces = address.Assign(netDevLan1);
    // For LAN 2
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer lan2interfaces;
    lan2interfaces = address.Assign(netDevLan2);
    // For PointToPoint
    address.SetBase("10.1.100.0", "255.255.255.0");
    Ipv4InterfaceContainer routerInterfaces;
    routerInterfaces = address.Assign(netDevBridge);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    // Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>(&std::cout);
    // Ipv4RoutingHelper::PrintRoutingTableAllAt( Seconds(1), routingStream, Time::Unit::S);

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


    std::cout << "before socket" << std::endl;

    /**
     * @brief Compute interval from data rate
     * 
     */
    float app_DataRate = 5000000; // 5e6 = 5M bps
    float app_Interval = float(1024*8) / app_DataRate; // complete each packet within this time.
    std::cout << "app_Interval = " << app_Interval << std::endl;

    ObjectFactory fact;
    fact.SetTypeId ("ns3::overlayApplication");
    fact.Set ("Interval", TimeValue (Seconds (app_Interval)));
    fact.Set ("RemotePort", UintegerValue (9));
    fact.Set ("ListenPort", UintegerValue (9));
    fact.Set ("MaxPackets", UintegerValue (3));
    fact.Set ("PacketSize", UintegerValue (1024));
    Ptr<overlayApplication> appSrc = fact.Create <overlayApplication> ();
    appSrc->AddRemote( lan2interfaces.GetAddress (0) );
    appSrc->SetStartTime (Seconds (0));
    appSrc->SetStopTime (Seconds (1));
    overlayNodes.Get(0)->AddApplication( appSrc );

    //fact.Set ("MaxPackets", UintegerValue (1));
    /* Ptr<overlayApplication> appDest = fact.Create <overlayApplication> ();
    appDest->AddRemote( lan1interfaces.GetAddress (0) );
    appDest->SetStartTime (Seconds (0));
    appDest->SetStopTime (Seconds (1));
    overlayNodes.Get(1)->AddApplication( appDest ); */


    /* UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApps = echoServer.Install (overlayNodes.Get(1));
    serverApps.Start (Seconds (0));
    serverApps.Stop (Seconds (3));

    UdpEchoClientHelper echoClient (lan2interfaces.GetAddress (0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClient.SetAttribute ("Interval", TimeValue (MilliSeconds (100)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    //NodeContainer clientNodes (overlayNodes.Get(0));
    ApplicationContainer clientApps = echoClient.Install (overlayNodes.Get(0));
    clientApps.Start (Seconds (1));
    clientApps.Stop (Seconds (3)); */

    // /*Create sockets
    //  */
    // // Receiver socket on n1
    // TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    // Ptr<Socket> recvSink = Socket::CreateSocket(overlayNodes.Get(1), tid);
    // InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny (), 4477);
    // // recvSink->SetIpRecvTos(ipRecvTos);
    // // recvSink->SetIpRecvTtl(ipRecvTtl);
    // recvSink->Bind(local);
    // recvSink->SetRecvCallback(MakeCallback(&receivePkt));

    // // sender socket on n0
    // Ptr<Socket> source = Socket::CreateSocket(overlayNodes.Get(0), tid);
    // InetSocketAddress remote = InetSocketAddress(lan1interfaces.GetAddress(0), 4477);
    // // Set socket options, it is also possible to set the options after the socket has been created/connected.
    // // if (ipTos > 0)
    // // {
    // //     source->SetIpTos(ipTos);
    // // }

    // // if (ipTtl > 0)
    // // {
    // //     source->SetIpTtl(ipTtl);
    // // }
    // source->Connect(remote);

    // // std::cout << "before schedule" << std::endl;

    // // Schedule SendPacket
    // Time interPacketInterval = Seconds(packetInterval);
    // Simulator::ScheduleWithContext(source->GetNode()->GetId(),
    //                                Seconds(1.0), &SendPacket,
    //                                source, packetSize, packetCount, interPacketInterval, 0, 1);

    Config::Connect( "/NodeList/*/$ns3::Ipv4L3Protocol/Rx", MakeCallback(&rxTraceIpv4) );
    Config::Connect( "/NodeList/*/$ns3::Ipv4L3Protocol/Tx", MakeCallback(&txTraceIpv4) );
    Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx", MakeCallback(&p2pDevMacTx) );
    Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacRx", MakeCallback(&p2pDevMacRx) );
    // // Config::Connect( "/NodeList/*/ApplicationList/*/$ns3::UdpEchoClient/Tx", MakeCallback(&trace_udpClient) );

    Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyTxBegin", MakeCallback(&trace_PhyTxBegin) );
    Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyTxEnd", MakeCallback(&trace_PhyTxEnd) );
    Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyRxEnd", MakeCallback(&trace_PhyRxEnd) );

    //Config::Connect( "/ChannelList/*/$ns3::PointToPointChannel/TxRxPointToPoint", MakeCallback(&trace_txrxPointToPoint) );

    NS_LOG_INFO("Run Simulation.");
    // std::cout << "before run" << std::endl;
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");
}