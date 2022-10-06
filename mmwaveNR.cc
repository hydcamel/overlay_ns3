#include "mmwaveNR.h"

namespace ns3
{

myNR::myNR(){}
void myNR::init_myNR(coordinate &gnb_coordinate, coordinate &ue_coordinate, uint32_t netw_base, overlayApplication &app_interface, InternetStackHelper &internet, Ptr<NrHelper> &nrHelper, Ptr<NrPointToPointEpcHelper> &epcHelper)
{
    // setup the nr simulation
    // nrHelper = CreateObject<NrHelper> ();
    nrHelper->set_cell_ID( app_interface.GetLocalID() );
    // std::cout << "app_interface.GetLocalID() = " << app_interface.GetLocalID() << std::endl;
    // nrHelper->SetHarqEnabled(false);
    NodeContainer gNbNodes;
    NodeContainer ueNodes;

    /*
   *  Create the gNB and UE nodes according to the network topology
   */
    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> bsPositionAlloc = CreateObject<ListPositionAllocator> ();
    Ptr<ListPositionAllocator> utPositionAlloc = CreateObject<ListPositionAllocator> ();

    const double gNbHeight = 10;
    const double ueHeight = 1.5;
    
    // mobility.Install (gNbNodes);
    // mobility.Install (ueNodes);

    if (singleUeTopology)
    {
      gNbNodes.Create (1);
      ueNodes.Create (1);
      gNbNum = 1;
      ueNumPergNb = 1;

      mobility.Install (gNbNodes);
      mobility.Install (ueNodes);
      bsPositionAlloc->Add (Vector (gnb_coordinate.x_val, gnb_coordinate.y_val, gNbHeight));
      utPositionAlloc->Add (Vector (ue_coordinate.x_val, ue_coordinate.y_val, ueHeight));
    }
    else
    {
        gNbNodes.Create (gNbNum);
        ueNodes.Create (ueNumPergNb * gNbNum);
        // mobility.Install (gNbNodes);
        // mobility.Install (ueNodes);

        bsPositionAlloc->Add (Vector (gnb_coordinate.x_val, gnb_coordinate.y_val, gNbHeight));
        // std::cout << "Node ID = " << app_interface.GetLocalID() << ":gnb = " << gnb_coordinate.x_val << ", gnb = " << gnb_coordinate.y_val << std::endl;
        for (uint16_t j = 0; j < ueNumPergNb; ++j)
        {
            double ut_x, ut_y;
            switch (app_interface.meta->pos_ue_x[j])
            {
                case 0:
                    ut_x = gnb_coordinate.x_val;
                    break;
                case 1:
                    ut_x = gnb_coordinate.x_val + app_interface.meta->distance_ue_from_gnb;
                    break;
                case -1:
                    ut_x = gnb_coordinate.x_val - app_interface.meta->distance_ue_from_gnb;
                    break;
                default:
                    break;
            }
            switch (app_interface.meta->pos_ue_y[j])
            {
                case 0:
                    ut_y = gnb_coordinate.y_val;
                    break;
                case 1:
                    ut_y = gnb_coordinate.y_val + app_interface.meta->distance_ue_from_gnb;
                    break;
                case -1:
                    ut_y = gnb_coordinate.y_val - app_interface.meta->distance_ue_from_gnb;
                    break;
                default:
                    break;
            }
            // std::cout << "app_interface.meta->pos_ue_x = " << app_interface.meta->pos_ue_x[j] << "ut_x = " << ut_x << "app_interface.meta->pos_ue_y = " << app_interface.meta->pos_ue_y[j] << ", ut_y = " << ut_y << std::endl;
            // ueNodes.Get (j)->GetObject<MobilityModel> ()->SetPosition (Vector (ut_x, ut_y, ueHeight)); // (x, y, z) in m
            utPositionAlloc->Add (Vector (ut_x, ut_y, ueHeight));
        }
    }
    mobility.SetPositionAllocator (bsPositionAlloc);
    mobility.Install (gNbNodes);

    mobility.SetPositionAllocator (utPositionAlloc);
    mobility.Install (ueNodes);

    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;
    OperationBandInfo band;
    if ( !multi_bwp )
    {
        /*
        * Spectrum division. We create one operation band with one component carrier
        * (CC) which occupies the whole operation band bandwidth. The CC contains a
        * single Bandwidth Part (BWP). This BWP occupies the whole CC band.
        * Both operational bands will use the StreetCanyon channel modeling.
        */
        const uint8_t numCcPerBand = 1;  // in this example, both bands have a single CC
        BandwidthPartInfo::Scenario scenario = BandwidthPartInfo::RMa_LoS;
        // if (ueNumPergNb  > 1) scenario = BandwidthPartInfo::InH_OfficeOpen;
        if (ueNumPergNb  > 1) scenario = BandwidthPartInfo::RMa_LoS;
        // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
        // a single BWP per CC
        CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequency, bandwidth, numCcPerBand, scenario);

        // By using the configuration created, it is time to make the operation bands
        
        band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

        /*
        * Initialize channel and pathloss, plus other things inside band1. If needed,
        * the band configuration can be done manually, but we leave it for more
        * sophisticated examples. For the moment, this method will take care
        * of all the spectrum initialization needs.
        */
        nrHelper->InitializeOperationBand (&band);

        allBwps = CcBwpCreator::GetAllBwps ({band});

    }
    else
    {
        //non-contiguous case
        double centralFrequencyCc0 = 28e9;
        double centralFrequencyCc1 = 29e9;
        double bandwidthCc0 = 100e6;
        double bandwidthCc1 = 100e6;
        std::string pattern = "F|F|F|F|F|F|F|F|F|F|";

        std::unique_ptr<ComponentCarrierInfo> cc0 (new ComponentCarrierInfo ());
        std::unique_ptr<BandwidthPartInfo> bwp0 (new BandwidthPartInfo ());

        std::unique_ptr<ComponentCarrierInfo> cc1 (new ComponentCarrierInfo ());
        std::unique_ptr<BandwidthPartInfo> bwp1 (new BandwidthPartInfo ());

        /*
        * The configured spectrum division is:
        * ----------------------------- Band ---------------------------------
        * ---------------CC0--------------|----------------CC1----------------
        * ------BWP0------|------BWP1-----|----------------BWP0---------------
        */
        const uint8_t numContiguousCcs = 2; // 4 CCs per Band
        CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequency, bandwidth, numContiguousCcs, BandwidthPartInfo::UMi_StreetCanyon_LoS);
        bandConf.m_numBwp = 1; // 1 BWP per CC

        // By using the configuration created, it is time to make the operation band
        band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);
        nrHelper->InitializeOperationBand (&band);
        allBwps = CcBwpCreator::GetAllBwps ({band});
    }
    


    /*
    * Old runnable backup for single BWP
    */
    // const uint8_t numCcPerBand = 1;  // in this example, both bands have a single CC
    // BandwidthPartInfo::Scenario scenario = BandwidthPartInfo::RMa_LoS;
    // if (ueNumPergNb  > 1) scenario = BandwidthPartInfo::InH_OfficeOpen;
    // // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
    // // a single BWP per CC
    // CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequency, bandwidth, numCcPerBand, scenario);

    // // By using the configuration created, it is time to make the operation bands
    // CcBwpCreator ccBwpCreator;
    // OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

    // /*
    // * Initialize channel and pathloss, plus other things inside band1. If needed,
    // * the band configuration can be done manually, but we leave it for more
    // * sophisticated examples. For the moment, this method will take care
    // * of all the spectrum initialization needs.
    // */
    // nrHelper->InitializeOperationBand (&band);

    // BandwidthPartInfoPtrVector allBwps = CcBwpCreator::GetAllBwps ({band});

    /*
    * Continue setting the parameters which are common to all the nodes, like the
    * gNB transmit power or numerology.
    */
    nrHelper->SetGnbPhyAttribute ("TxPower", DoubleValue (txPower));
    nrHelper->SetGnbPhyAttribute ("Numerology", UintegerValue (numerology));

    // Scheduler
    nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerTdmaRR"));
    // nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerOfdmaRR"));
    nrHelper->SetSchedulerAttribute ("FixedMcsDl", BooleanValue (useFixedMcs));
    nrHelper->SetSchedulerAttribute ("FixedMcsUl", BooleanValue (useFixedMcs));
    nrHelper->SetSchedulerAttribute ("FixedMcsUl", BooleanValue (useFixedMcs));
    // CqiTimerThreshold

    if (useFixedMcs == true)
        {
        nrHelper->SetSchedulerAttribute ("StartingMcsDl", UintegerValue (fixedMcs));
        nrHelper->SetSchedulerAttribute ("StartingMcsUl", UintegerValue (fixedMcs));
        }

    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));

    // Antennas for all the UEs
    nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
    nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
    nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

    // Antennas for all the gNbs
    nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
    nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
    nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));

    // Beamforming method
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));
    nrHelper->SetBeamformingHelper (idealBeamformingHelper);

    Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds (0)));
    //  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
    nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

    // Error Model: UE and GNB with same spectrum error model.
    nrHelper->SetUlErrorModel ("ns3::NrEesmIrT1");
    nrHelper->SetDlErrorModel ("ns3::NrEesmIrT1");

    // Both DL and UL AMC will have the same model behind.
    nrHelper->SetGnbDlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel
    nrHelper->SetGnbUlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel


    // Create EPC helper
    const std::string Addr_IPv4_Network_gNB {std::to_string(netw_base) + ".0.0.0"};
    // const std::string Addr_IPv4_Network_gNB {netw_base};
    epcHelper = CreateObject<NrPointToPointEpcHelper> (netw_base);
    nrHelper->SetEpcHelper (epcHelper);
    // Core latency
    epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));
    epcHelper->SetAttribute ("S1uLinkMtu", UintegerValue (2500));
    // epcHelper->SetAttribute ("S1uLinkMtu", UintegerValue (9000));

    // gNb routing between Bearer and bandwidh part
    uint32_t bwpIdForBearer = 0;
    nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForBearer));
    // nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VOICE_VIDEO_GAMING", UintegerValue (1));

    // Initialize nrHelper
    nrHelper->Initialize ();

    // Install nr net devices
    NetDeviceContainer gNbNetDev = nrHelper->InstallGnbDevice (gNbNodes, allBwps);

    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);

    // for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it) DynamicCast<NrNetDevice> (*it)->SetAttribute ("Mtu", UintegerValue (5000));
    // for (auto it = gNbNetDev.Begin (); it != gNbNetDev.End (); ++it) DynamicCast<NrNetDevice> (*it)->SetAttribute ("Mtu", UintegerValue (5000));

    int64_t randomStream = 1;
    randomStream += nrHelper->AssignStreams (gNbNetDev, randomStream);
    randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);


    // When all the configuration is done, explicitly call UpdateConfig ()

    for (auto it = gNbNetDev.Begin (); it != gNbNetDev.End (); ++it) DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();

    for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it) DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();

    // create the internet and install the IP stack on the UEs
    // get SGW/PGW and create a single RemoteHost 
    Ptr<Node> pgw = epcHelper->GetPgwNode ();
    Ptr<Node> remoteHost = app_interface.GetNode();
    // connect a remoteHost to pgw. Setup routing too
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("500Gb/s")));
    p2ph.SetDeviceAttribute ("Mtu", UintegerValue (9000));
    // p2ph.SetDeviceAttribute ("Mtu", UintegerValue (3000));
    p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
    NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    // ipv4h.SetBase ( "1.0.0.0", "255.0.0.0");
    const std::string p2pPgwEpc{std::to_string(netw_base+1) + ".0.0.0"};
    ipv4h.SetBase (Ipv4Address(p2pPgwEpc.data()), "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

    internet.Install (ueNodes);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address (Addr_IPv4_Network_gNB.data()), Ipv4Mask ("255.0.0.0"), remoteHost->GetNDevices()-1);
    

    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));
    // std::cout << "NR ID: " << app_interface.GetLocalID() << "-ueIpIface.size()=" << ueIpIface.GetN() << std::endl;

    // Set the default gateway for the UEs
    for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
    {
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (j)->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

    // attach UEs to the closest eNB
    nrHelper->AttachToClosestEnb (ueNetDev, gNbNetDev);

    // The bearer that will carry low latency traffic
    uint16_t dlPort = NRPORT;
    EpsBearer bearer (EpsBearer::GBR_CONV_VOICE);

    Ptr<EpcTft> tft = Create<EpcTft> ();
    EpcTft::PacketFilter dlpf;
    dlpf.localPortStart = dlPort;
    dlpf.localPortEnd = dlPort;
    tft->Add (dlpf);
    // uint32_t re_tftadd = tft->Add (dlpf);
    // std::cout << "NR ID: " << app_interface.GetLocalID() << " re_tftadd = " << re_tftadd << std::endl;

    /** set the Recv Listen Socket for UE **/
    ObjectFactory fact;
    fact.SetTypeId("ns3::ueApp");

    /** Set Connection between RemoteHost and UEs **/
    app_interface.nr_socket.resize(ueNodes.GetN ());
    vec_ue_app.resize(ueNodes.GetN ());
    for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
    {
        Ptr<Node> ue = ueNodes.Get (i);
        // std::cout << "UE ID for " << app_interface.GetLocalID() << " is " << ue->GetId() << std::endl;
        Ptr<NetDevice> ueDevice = ueNetDev.Get (i);
        // Address ueAddress = ueIpIface.GetAddress (i);
        vec_ue_app[i] = fact.Create<ueApp>();
        ue->AddApplication(vec_ue_app[i]);
        vec_ue_app[i]->initUeApp(app_interface);

        // The client, who is transmitting, is installed in the remote host,
        // with destination address set to the address of the UE
        /* dlClient.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
        clientApps.Add (dlClient.Install (remoteHost)); */
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        app_interface.nr_socket[i] = Socket::CreateSocket(app_interface.GetNode(), tid);
        if (app_interface.nr_socket[i]->Bind() == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        // app_interface.nr_socket[i]->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(ueAddress), dlPort));
        // if (app_interface.GetLocalID() == 3)
        //     std::cout << app_interface.GetLocalID() << "-ue->GetObject<Ipv4> (): " << ue->GetObject<Ipv4> ()->GetAddress( 1, 0 ).GetLocal() << std::endl;
        app_interface.nr_socket[i]->Connect(InetSocketAddress(ue->GetObject<Ipv4> ()->GetAddress( 1, 0 ).GetLocal(), dlPort));
        /* if (app_interface.GetLocalID() == 0 || app_interface.GetLocalID() == 3 || app_interface.GetLocalID() == 4)
        {
            std::cout << "NR ID: " << app_interface.GetLocalID() << " - " << ue->GetObject<Ipv4> ()->GetAddress( 1, 0 ).GetLocal() << std::endl;
        } */
        // Activate a dedicated bearer for the traffic type
        // uint32_t res_activateBearer = nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer, tft);
        // std::cout << "NR ID: " << app_interface.GetLocalID() << " ActivateDedicatedEpsBearer = " << res_activateBearer << std::endl;
    }
    

    

    
    

    // assign IP address to UEs, and install UDP downlink applications
    

    /* ApplicationContainer serverApps;

    // The sink will always listen to the specified ports
    UdpServerHelper dlPacketSinkHelper (dlPort);
    serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (0)));

    UdpClientHelper dlClient;
    dlClient.SetAttribute ("RemotePort", UintegerValue (dlPort));
    dlClient.SetAttribute ("PacketSize", UintegerValue (udpPacketSize));
    dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
    if (udpFullBuffer)
        {
        double bitRate =
            75000000; // 75 Mbps will saturate the NR system of 20 MHz with the NrEesmIrT1 error model
        bitRate /= ueNumPergNb; // Divide the cell capacity among UEs
        if (bandwidth > 20e6)
            {
            bitRate *= bandwidth / 20e6;
            }
        lambda = bitRate / static_cast<double> (udpPacketSize * 8);
        }
    dlClient.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambda))); */

    
    

    /*
    * Let's install the applications! Socket connection between UEs and remote host
    */
    

    /* ApplicationContainer clientApps;

    

    // start server and client apps
    serverApps.Start (Seconds (udpAppStartTime));
    clientApps.Start (Seconds (udpAppStartTime));
    serverApps.Stop (Seconds (simTime));
    clientApps.Stop (Seconds (simTime)); */
    
}
myNR::~myNR()
{
    vec_ue_app.clear();
    // nrHelper = nullptr;
}

} // namespace ns3