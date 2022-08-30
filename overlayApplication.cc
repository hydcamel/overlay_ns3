#include "overlayApplication.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/queue.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable-stream.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/point-to-point-module.h"
// #include "SDtag.h"
#include <assert.h>
#include "netw.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("overlayApplication");
NS_OBJECT_ENSURE_REGISTERED(overlayApplication);

TypeId overlayApplication::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::overlayApplication")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<overlayApplication>()
                            .AddAttribute("probe_interval",
                                            "The interval for probing",
                                            TimeValue(MicroSeconds(100.0)),
                                            MakeTimeAccessor(&overlayApplication::probe_interval),
                                            MakeTimeChecker())
                            .AddAttribute("sandwich_interval",
                                            "The interval for sandwich probing",
                                            TimeValue(MicroSeconds(10.0)),
                                            MakeTimeAccessor(&overlayApplication::sandwich_interval),
                                            MakeTimeChecker())
                            /* .AddAttribute("RemoteAddress",
                                            "The destination Address of the outbound packets",
                                            AddressValue(),
                                            MakeAddressAccessor(&overlayApplication::m_peerAddress),
                                            MakeAddressChecker()) */
                            .AddAttribute("RemotePort",
                                            "The destination port of the outbound packets",
                                            UintegerValue(0),
                                            MakeUintegerAccessor(&overlayApplication::m_peerPort),
                                            MakeUintegerChecker<uint16_t>())
                            // .AddAttribute("LocalID",
                            //               "The ID of the node in underlay",
                            //               UintegerValue(0),
                            //               MakeUintegerAccessor(&overlayApplication::SetLocalID,
                            //                                    &overlayApplication::GetLocalID),
                            //               MakeUintegerChecker<uint16_t>())
                            .AddAttribute("ListenPort", "Port on which we listen for incoming packets.",
                                            UintegerValue(0),
                                            MakeUintegerAccessor(&overlayApplication::ListenPort),
                                            MakeUintegerChecker<uint16_t>());
    return tid;
}
TypeId netw::GetInstanceTypeId (void) const
{
  	return netw::GetTypeId ();
}

overlayApplication::overlayApplication()
{
    NS_LOG_FUNCTION(this);
}
overlayApplication::~overlayApplication()
{
    NS_LOG_FUNCTION(this);
    std::cout << m_local_ID << ": start deleting." << std::endl;
    tab_socket.clear();
    // m_interval.clear();
    recv_socket = 0;
    meta = 0;
    m_sendEvent.clear();
    probe_event.clear();
    map_neighbor_device.clear();
    // m_count.clear();
    // m_socket = 0;
}

void overlayApplication::InitApp(netw *netw_meta, uint32_t localId, uint32_t MaxPktSize)
{
    meta = netw_meta;
    tab_socket.resize(meta->n_nodes, 0);
    // tab_peerAddress.resize(meta.n_nodes);
    // m_interval.resize(meta->n_nodes);
    // m_socket = 0;
    m_peerPort = 9;
    recv_socket = 0;
    // probe_interval = 0;
    m_sendEvent.resize(meta->n_nodes, EventId());
    probe_event.resize(meta->n_nodes, EventId());
    SetLocalID(localId);
    is_overlay = meta->loc_overlay_nodes[localId];
}
void overlayApplication::SetLocalID(uint32_t localID)
{
    NS_LOG_FUNCTION(this);
    m_local_ID = localID;
}
uint32_t overlayApplication::GetLocalID(void) const
{
    NS_LOG_FUNCTION(this);
    return m_local_ID;
}


}