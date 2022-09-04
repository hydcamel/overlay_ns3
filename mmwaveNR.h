#ifndef MMWAVE_NR_H
#define MMWAVE_NR_H

#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/nr-module.h"
#include "ns3/config-store-module.h"
#include "ns3/antenna-module.h"
#include "utils.h"

namespace ns3
{

class myNR
{
public:
    // set simulation time and mobility

    //other simulation parameters default values
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

    /** NR **/
    NodeContainer gNbNodes;
    NodeContainer ueNodes;
    MobilityHelper mobility;
    Ptr<NrHelper> nrHelper;
    CcBwpCreator ccBwpCreator;
    BandwidthPartInfo::Scenario scenario;
    OperationBandInfo band;
    BandwidthPartInfoPtrVector allBwps;
    Ptr<NrPointToPointEpcHelper> epcHelper;
    Ptr<ListPositionAllocator> bsPositionAlloc;
    Ptr<ListPositionAllocator> utPositionAlloc;
    coordinate gnb_coordinate;
    coordinate ue_coordinate;
    NetDeviceContainer gNbNetDev;
    NetDeviceContainer ueNetDev;
    Ptr<Node> pgw;

    /**
     * functions
     **/
    myNR(coordinate &gnb_coordinate, coordinate &ue_coordinate, uint32_t netw_base, Ptr<Node> remoteHost, InternetStackHelper &internet);
    ~myNR();
};

}

#endif