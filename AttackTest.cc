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

enum ProjName
{
    WInf = 1,
    Param = 2,
    Attack = 3
};

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
    ProjName cur_prj = ProjName::Attack;
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
    double stop_time = 700.0; // ms
    // double stop_time = 400.0; // microseconds
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
        std::cout << netw_meta.edges_vec[i].first << ": " << linkIpv4Addr[1].GetAddress() << std::endl;
        vec_app[netw_meta.edges_vec[i].second]->SetSocket(linkIpv4Addr[0].GetAddress(), netw_meta.edges_vec[i].first, n_devices_perNode[1] - 1);
        std::cout << netw_meta.edges_vec[i].second << ": " << linkIpv4Addr[0].GetAddress() << std::endl;

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
    uint32_t network_base_number = 7;
    // myNR proxy_NR;
    // proxy_NR.init_myNR(netw_meta.vec_gnb_coordinate_, network_base_number, &netw_meta, vec_app, internet);
    double centralFrequencyCc0 = 28e9;
    double centralFrequencyCc1 = 30e9;
    double bandwidthCc0 = 2e9;
    double bandwidthCc1 = 2e9;
    // double centralFrequencyCc1 = 29e9;
    // double bandwidthCc0 = 400e6;
    // double bandwidthCc1 = 400e6;
    // double bandwidthBand = 3e9;
    std::string pattern = "F|F|F|F|F|F|F|F|F|F|";

    NodeContainer gNbNodes;
    NodeContainer ueNodes;
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> bsPositionAlloc = CreateObject<ListPositionAllocator> ();
    Ptr<ListPositionAllocator> utPositionAlloc = CreateObject<ListPositionAllocator> ();
    const double gNbHeight = 10;
    const double ueHeight = 1.5;
    // gNbNodes.Create (netw_meta.n_nodes);
    gNbNodes.Create(netw_meta.set_tb.size());
    std::vector<NodeContainer> vec_UE(netw_meta.n_nodes);
    for (uint32_t j = 0; j < netw_meta.n_nodes; j++)
    {
        if (!netw_meta.is_w_probing && netw_meta.set_tb.count(j) == 0) continue;
        vec_UE[j].Create(netw_meta.n_perUE[j]);
        ueNodes.Add(vec_UE[j]);
    }

    for (uint32_t i = 0; i < netw_meta.n_nodes; i++)
    {
        if (!netw_meta.is_w_probing && netw_meta.set_tb.count(i) == 0) continue;
        bsPositionAlloc->Add (Vector (netw_meta.vec_gnb_coordinate_[i].x_val, netw_meta.vec_gnb_coordinate_[i].y_val, gNbHeight));
        // if (!netw_meta.is_w_probing && netw_meta.set_tb.count(i) == 0) continue;
        // std::cout << "Node ID = " << vec_app[i]->GetLocalID() << ":gnb = " << netw_meta.vec_gnb_coordinate_[i].x_val << ", gnb = " << netw_meta.vec_gnb_coordinate_[i].y_val << std::endl;
        for (uint16_t j = 0; j < vec_UE[i].GetN(); ++j)
        {
            double ut_x = netw_meta.vec_gnb_coordinate_[i].x_val + netw_meta.distance_ue_from_gnb, ut_y = 0;
            
            if (j == 0)
            {
                ut_y = netw_meta.vec_gnb_coordinate_[i].y_val + netw_meta.distance_ue_from_gnb;
            }
            else
            {
                ut_y = netw_meta.vec_gnb_coordinate_[i].y_val - netw_meta.distance_ue_from_gnb + 4 * j;
            }
            
            // std::cout << "ut_x = " << ut_x << ", ut_y = " << ut_y << std::endl;
            utPositionAlloc->Add (Vector (ut_x, ut_y, ueHeight));
        }
    }
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator (bsPositionAlloc);
    mobility.Install (gNbNodes);

    mobility.SetPositionAllocator (utPositionAlloc);
    mobility.Install (ueNodes);
    

    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> (network_base_number);
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
    nrHelper->SetBeamformingHelper (idealBeamformingHelper);
    nrHelper->SetEpcHelper (epcHelper);

    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;

    OperationBandInfo band;
    double centralFrequency = 29e9;
    double bandwidth = 5e9;
    double txPower = 30;
    uint16_t numerology = 5;

    //For the case of manual configuration of CCs and BWPs 
    std::unique_ptr<ComponentCarrierInfo> cc0 (new ComponentCarrierInfo ());
    std::unique_ptr<BandwidthPartInfo> bwp0 (new BandwidthPartInfo ());

    std::unique_ptr<ComponentCarrierInfo> cc1 (new ComponentCarrierInfo ());
    std::unique_ptr<BandwidthPartInfo> bwp1 (new BandwidthPartInfo ());

    band.m_centralFrequency  = centralFrequency;
    band.m_channelBandwidth = bandwidth;
    band.m_lowerFrequency = band.m_centralFrequency - band.m_channelBandwidth / 2;
    band.m_higherFrequency = band.m_centralFrequency + band.m_channelBandwidth / 2;

    // Component Carrier 0
    cc0->m_ccId = 0;
    cc0->m_centralFrequency = centralFrequencyCc0;
    cc0->m_channelBandwidth = bandwidthCc0;
    cc0->m_lowerFrequency = cc0->m_centralFrequency - cc0->m_channelBandwidth / 2;
    cc0->m_higherFrequency = cc0->m_centralFrequency + cc0->m_channelBandwidth / 2;

    // BWP 0
    bwp0->m_bwpId = 0;
    bwp0->m_centralFrequency = cc0->m_centralFrequency;
    bwp0->m_channelBandwidth = cc0->m_channelBandwidth;
    bwp0->m_lowerFrequency = bwp0->m_centralFrequency - bwp0->m_channelBandwidth / 2;
    bwp0->m_higherFrequency = bwp0->m_centralFrequency + bwp0->m_channelBandwidth / 2;

    cc0->AddBwp (std::move (bwp0));

    // Component Carrier 1
    cc1->m_ccId = 1;
    cc1->m_centralFrequency = centralFrequencyCc1;
    cc1->m_channelBandwidth = bandwidthCc1;
    cc1->m_lowerFrequency = cc1->m_centralFrequency - cc1->m_channelBandwidth / 2;
    cc1->m_higherFrequency = cc1->m_centralFrequency + cc1->m_channelBandwidth / 2;

    // BWP 2
    bwp1->m_bwpId = 1;
    bwp1->m_centralFrequency = cc1->m_centralFrequency;
    bwp1->m_channelBandwidth = cc1->m_channelBandwidth;
    bwp1->m_lowerFrequency = cc1->m_lowerFrequency;
    bwp1->m_higherFrequency = cc1->m_higherFrequency;

    cc1->AddBwp (std::move (bwp1));

    // Add CC to the corresponding operation band.
    band.AddCc (std::move (cc1));
    band.AddCc (std::move (cc0));
    // nrHelper->InitializeOperationBand (&band);
    // allBwps = CcBwpCreator::GetAllBwps ({band});

    nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));
    epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));
    epcHelper->SetAttribute ("S1uLinkDataRate", DataRateValue (DataRate ("1000Gb/s")));
    epcHelper->SetAttribute ("S1uLinkMtu", UintegerValue (10000));
    // nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerTdmaRR"));
    nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerOfdmaPF"));
    idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));
    nrHelper->InitializeOperationBand (&band);
    allBwps = CcBwpCreator::GetAllBwps ({band});

    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));

    // double x = pow (10, txPower / 10);

    // Antennas for all the UEs
    nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
    nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (2));
    nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

    // Antennas for all the gNbs
    nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (8));
    nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
    nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));


    uint32_t bwpIdForLowLat = 0;
    uint32_t bwpIdForVoice = 1;
    // uint32_t bwpIdForVideo = 2;
    // uint32_t bwpIdForVideoGaming = 3;

    nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
    nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));
    // nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (bwpIdForVideo));
    // nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VOICE_VIDEO_GAMING", UintegerValue (bwpIdForVideoGaming));

    //Install and get the pointers to the NetDevices
    NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gNbNodes, allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);

    int64_t randomStream = 1;
    randomStream += nrHelper->AssignStreams (enbNetDev, randomStream);
    randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);

    // Set the attribute of the netdevice (enbNetDev.Get (0)) and bandwidth part (0), (1), ...
    for (uint32_t j = 0; j < enbNetDev.GetN(); j++)
    {
        // if (!netw_meta.is_w_probing && netw_meta.set_tb.count(j) == 0) continue;
        for (uint32_t u = 0; u < allBwps.size(); u++)
        {
            nrHelper->GetGnbPhy (enbNetDev.Get (j), u)->SetAttribute ("Numerology", UintegerValue (numerology));
            nrHelper->GetGnbPhy (enbNetDev.Get (j), u)->SetAttribute ("TxPower", DoubleValue (txPower));
            nrHelper->GetGnbPhy (enbNetDev.Get (j), u)->SetAttribute ("Pattern", StringValue (pattern));
        }
    }

    for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it) DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it) DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();

    Ptr<Node> pgw = epcHelper->GetPgwNode ();
    std::vector<PointToPointHelper> vec_p2ph(vec_app.size());
    NetDeviceContainer internetDevices;
    Ipv4AddressHelper ipv4h;
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    const std::string p2pPgwEpc{std::to_string(network_base_number+1) + ".0.0.0"};
    ipv4h.SetBase (Ipv4Address(p2pPgwEpc.data()), "255.0.0.0");
    const std::string Addr_IPv4_Network_gNB {std::to_string(network_base_number) + ".0.0.0"};
    for (uint32_t j = 0; j < vec_app.size(); j++)
    {
        if (!netw_meta.is_w_probing && netw_meta.set_tb.count(j) == 0) continue;
        Ptr<Node> remoteHost = vec_app[j]->GetNode();
        // connect a remoteHost to pgw. Setup routing too
        vec_p2ph[j].SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("1000Gb/s")));
        vec_p2ph[j].SetDeviceAttribute ("Mtu", UintegerValue (10000));
        vec_p2ph[j].SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
        vec_p2ph[j].DisableFlowControl();
        NetDeviceContainer tmp_NetDeviceContainer = vec_p2ph[j].Install (pgw, remoteHost);
        // std::cout << tmp_NetDeviceContainer.Get(0) << " " << tmp_NetDeviceContainer.Get(1) << std::endl;
        internetDevices.Add( tmp_NetDeviceContainer );
        Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
        // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
        remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address (Addr_IPv4_Network_gNB.data()), Ipv4Mask ("255.0.0.0"), remoteHost->GetNDevices()-1);
    }
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

    internet.Install (ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));
    // Set the default gateway for the UEs
    for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
    {
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (j)->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
    // attach UEs to the closest eNB before creating the dedicated flows
    nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

    // The bearer that will carry CONV_VOICE traffic
    uint16_t dlPort_bwp0 = NRPORT;
    EpsBearer bearer_0 (EpsBearer::NGBR_LOW_LAT_EMBB);

    Ptr<EpcTft> tft_0 = Create<EpcTft> ();
    EpcTft::PacketFilter dlpf_0;
    dlpf_0.localPortStart = dlPort_bwp0;
    dlpf_0.localPortEnd = dlPort_bwp0;
    tft_0->Add (dlpf_0);

    // The bearer that will carry low latency traffic
    uint16_t dlPort_bwp1 = NRPORT + 1;
    EpsBearer bearer_1 (EpsBearer::GBR_CONV_VOICE);

    Ptr<EpcTft> tft_1 = Create<EpcTft> ();
    EpcTft::PacketFilter dlpf_1;
    dlpf_1.localPortStart = dlPort_bwp1;
    dlpf_1.localPortEnd = dlPort_bwp1;
    tft_1->Add (dlpf_1);

    /** set the Recv Listen Socket for UE **/
    // ObjectFactory fact;
    fact.SetTypeId("ns3::ueApp");
    std::vector<Ptr<ueApp>> vec_ue_app(ueNodes.GetN ());
    // vec_ue_app.resize(ueNodes.GetN ());
    uint32_t idx = 0;
    for (uint32_t i = 0; i < netw_meta.n_nodes; i++)
    {
        if (!netw_meta.is_w_probing && netw_meta.set_tb.count(i) == 0) continue;
        /** Set Connection between RemoteHost and UEs **/
        vec_app[i]->nr_socket.resize(vec_UE[i].GetN());
        for (uint32_t j = 0; j < vec_UE[i].GetN(); j++)
        {
            Ptr<Node> ue = vec_UE[i].Get (j);
            vec_ue_app[idx] = fact.Create<ueApp>();
            // std::cout << "UE ID for " << app_interface.GetLocalID() << " is " << ue->GetId() << std::endl;
            ue->AddApplication(vec_ue_app[idx]);
            vec_ue_app[idx]->initUeApp(*vec_app[i]);
            Ptr<NetDevice> ueDevice = ueNetDev.Get (idx++);
            
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            vec_app[i]->nr_socket[j] = Socket::CreateSocket(vec_app[i]->GetNode(), tid);
            if (vec_app[i]->nr_socket[j]->Bind() == -1) NS_FATAL_ERROR("Failed to bind socket");
            vec_app[i]->nr_socket[j]->Connect(InetSocketAddress(ue->GetObject<Ipv4> ()->GetAddress( 1, 0 ).GetLocal(), dlPort_bwp0));
            nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer_0, tft_0);

            /* if (j == 0)
            {
                vec_app[i]->nr_socket[j]->Connect(InetSocketAddress(ue->GetObject<Ipv4> ()->GetAddress( 1, 0 ).GetLocal(), dlPort_bwp0));
                nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer_0, tft_0);
            }
            else
            {   
                // std::cout << "activate for " << i << " with port = " << uint32_t(dlPort_bwp1) << std::endl;
                vec_app[i]->nr_socket[j]->Connect(InetSocketAddress(ue->GetObject<Ipv4> ()->GetAddress( 1, 0 ).GetLocal(), dlPort_bwp1));
                nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer_1, tft_1);
            } */

            // nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer_0, tft_0);
            // nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer_1, tft_1);
        }
    }
    

    // uint32_t network_base_number = 20;
    // // std::vector<Ptr<myNR>> vec_nr_app( vec_app.size() );
    // std::vector<Ptr<NrHelper>> vec_NrHelper(vec_app.size());
    // std::vector<Ptr<NrPointToPointEpcHelper>> vec_EpcHelper(vec_app.size());
    // for (uint32_t i = 0; i < vec_NrHelper.size(); i++)
    // {
    //     vec_NrHelper[i] = CreateObject<NrHelper> ();
    // }
    
    // std::vector<myNR> vec_nr_app(vec_app.size());
    // // std::vector<coordinate> vec_gnb_coordinate( vec_app.size() );
    // std::vector<coordinate> vec_ue_coordinate( vec_app.size() );
    // // myNR testNR(vec_gnb_coordinate[2], vec_ue_coordinate[2], network_base_number, *(vec_app[2]), internet);
    // for (uint32_t i = 0; i < 1; i++)
    // {
    //     // netw_meta.vec_gnb_coordinate_[i].x_val = i*100;
    //     // netw_meta.vec_gnb_coordinate_[i].y_val = i*10;
    //     vec_ue_coordinate[i].x_val = netw_meta.vec_gnb_coordinate_[i].x_val + 20;
    //     vec_ue_coordinate[i].y_val = netw_meta.vec_gnb_coordinate_[i].y_val;
    //     if (netw_meta.n_perUE[i] > 1)
    //     {
    //         vec_nr_app[i].singleUeTopology = false;
    //         vec_nr_app[i].ueNumPergNb = netw_meta.n_perUE[i];
    //     }
        
    //     // myNR tmpNR(vec_gnb_coordinate[i], vec_ue_coordinate[i], network_base_number, *(vec_app[i]), internet);
    //     // vec_nr_app.push_back( tmpNR );
    //     vec_nr_app[i].init_myNR(netw_meta.vec_gnb_coordinate_[i], vec_ue_coordinate[i], network_base_number, *(vec_app[i]), internet, vec_NrHelper[i], vec_EpcHelper[i]);
    //     network_base_number += 7;
    //     // vec_nr_app[i].vec_ue_app[0]->SetStopTime(MilliSeconds(stop_time*5));
    // }
    
    // uint32_t NR_ID = 3;
    // std::cout << "NR ID: " << NR_ID << "-PGW ID: " << vec_nr_app[NR_ID].pgw->GetId() << std::endl;
    // NR_ID = 4;
    // std::cout << "NR ID: " << NR_ID << "-PGW ID: " << vec_nr_app[NR_ID].pgw->GetId() << std::endl;
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
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
    // std::cout << gNbNodes.GetN() << " " << ueNodes.GetN() << std::endl;
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), pgw, routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), gNbNodes.Get(0), routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), ueNodes.Get(0), routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), epcHelper->GetSgwNode(), routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), vec_nr_app[4].gNbNodes.Get(0), routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAt (MicroSeconds (5), vec_nr_app[4].epcHelper->GetSgwNode(), routingStream, Time::Unit::NS);
    // Ipv4RoutingHelper::PrintRoutingTableAllAt (Seconds (0.5), routingStream, Time::Unit::S);

    

    // Config::Connect( "/NodeList/*/$ns3::Ipv4L3Protocol/Tx", MakeCallback(&txTraceIpv4) );
    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx", MakeCallback(&p2pDevMacTx) );
    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacRx", MakeCallback(&p2pDevMacRx) );
    // // // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTxDrop", MakeCallback(&p2pDevMacRx) );

    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyTxBegin", MakeCallback(&trace_PhyTxBegin) );
    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyTxEnd", MakeCallback(&trace_PhyTxEnd) );
    // Config::Connect( "/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/PhyRxEnd", MakeCallback(&trace_PhyRxEnd) );

    // Config::SetDefault ("/NodeList/*/DeviceList/*/$ns3::LteNetDevice/Mtu", UintegerValue (9000));

    // FlowMonitorHelper flowmonHelper;
    // NodeContainer endpointNodes;
    // endpointNodes.Add(underlayNodes);
    // // for (auto it = netw_meta.set_tb.begin(); it != netw_meta.set_tb.end(); it++)
    // // {
    // //     endpointNodes.Add (underlayNodes.Get(*it));
    // //     endpointNodes.Add (vec_UE[*it]);
    // // }

    // Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
    // monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
    // monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
    // monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));


    NS_LOG_INFO("Run Simulation.");
    std::cout << "before run" << std::endl;
    Time time_stop_simulation = MilliSeconds(stop_time*1.2);
    // Simulator::Schedule(time_stop_simulation, stop_NR, vec_NrHelper);
    Simulator::Stop(time_stop_simulation);
    Simulator::Run();
    // flow_monitor->SerializeToXmlFile("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/flow_monitor_res.xml", true, true);
    
    std::cout << "Before Destroy." << std::endl;
    char *cwd = get_current_dir_name();
    std::string pwd_tmp(cwd, cwd+strlen(cwd));
    switch (cur_prj)
    {
        case ProjName::WInf:
            netw_meta.write_delays_cnt(pwd_tmp + "/scratch/Category_inference/delays_cnt.csv");
            break;
        case ProjName::Param:
            netw_meta.write_delays_cnt(pwd_tmp + "/scratch/CrossPathParam/delays_cnt.csv");
            break;
        case ProjName::Attack:
            netw_meta.write_delays_cnt(pwd_tmp + "/scratch/AttackTest/delays_cnt.csv");
            break;
        default:
            break;
    }
    
    // netw_meta.write_queuing_cnt(pwd_tmp + "/scratch/Category_inference/queuing_cnt.csv");
    // netw_meta.write_delays_cnt(pwd_tmp + "/scratch/Category_inference/delays_cnt.csv");
    // netw_meta.write_true_delays_cnt(pwd_tmp + "/scratch/Category_inference/true_delays_cnt.csv");
    std::cout << "start Destroy." << std::endl;


    // // Print per-flow statistics
    // monitor->CheckForLostPackets ();
    // Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
    // FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

    // double averageFlowThroughput = 0.0;
    // double averageFlowDelay = 0.0;

    // std::ofstream outFile;
    // std::string simTag = "default";
    // std::string outputDir = "./";
    // std::string filename = outputDir + "/" + simTag;
    // outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
    // if (!outFile.is_open ())
    // {
    //     std::cerr << "Can't open file " << filename << std::endl;
    //     return 1;
    // }

    // outFile.setf (std::ios_base::fixed);

    // double flowDuration = (time_stop_simulation - Time(MicroSeconds(AppStartTime))).GetSeconds ();
    // for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    // {
    //     Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
    //     std::stringstream protoStream;
    //     protoStream << (uint16_t) t.protocol;
    //     if (t.protocol == 6) protoStream.str ("TCP"); 
    //     if (t.protocol == 17) protoStream.str ("UDP");
    //     outFile << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> " << t.destinationAddress << ":" << t.destinationPort << ") proto " << protoStream.str () << "\n";
    //     outFile << "  Tx Packets: " << i->second.txPackets << "\n";
    //     outFile << "  Tx Bytes:   " << i->second.txBytes << "\n";
    //     outFile << "  TxOffered:  " << i->second.txBytes * 8.0 / flowDuration / 1000.0 / 1000.0  << " Mbps\n";
    //     outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
    //     outFile << "  Pkts Dropped:   " << i->second.lostPackets << "\n";
    //     if (i->second.rxPackets > 0)
    //     {
    //         // Measure the duration of the flow from receiver's perspective
    //         averageFlowThroughput += i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000;
    //         averageFlowDelay += 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets;

    //         outFile << "  Throughput: " << i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000  << " Mbps\n";
    //         outFile << "  Mean delay:  " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms\n";
    //         //outFile << "  Mean upt:  " << i->second.uptSum / i->second.rxPackets / 1000/1000 << " Mbps \n";
    //         outFile << "  Mean jitter:  " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << " ms\n";
    //     }
    //     else
    //     {
    //         outFile << "  Throughput:  0 Mbps\n";
    //         outFile << "  Mean delay:  0 ms\n";
    //         outFile << "  Mean jitter: 0 ms\n";
    //     }
    //     outFile << "  Rx Packets: " << i->second.rxPackets << "\n";
    // }

    // outFile << "\n\n  Mean flow throughput: " << averageFlowThroughput / stats.size () << "\n";
    // outFile << "  Mean flow delay: " << averageFlowDelay / stats.size () << "\n";

    // outFile.close ();

    // std::ifstream f (filename.c_str ());

    // if (f.is_open ())
    // {
    //     std::cout << f.rdbuf ();
    // }


    Simulator::Destroy();
    NS_LOG_INFO("Done.");
}
