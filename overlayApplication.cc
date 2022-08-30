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
void overlayApplication::DoDispose(void)
{
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
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

void overlayApplication::SetSocket(Address ip, uint32_t idx, uint32_t deviceID)
{
    NS_LOG_FUNCTION(this);
    if (tab_socket[idx] == 0)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        // std::cout << "Node ID:" << m_local_ID << "set skt for " << idx << ": " << ip << std::endl;
        tab_socket[idx] = Socket::CreateSocket(GetNode(), tid);
        if (Ipv4Address::IsMatchingType(ip) == true)
        {
            if (tab_socket[idx]->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            tab_socket[idx]->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(ip), m_peerPort));
        }
        else if (InetSocketAddress::IsMatchingType(ip) == true)
        {
            if (tab_socket[idx]->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            tab_socket[idx]->Connect(ip);
        }
        else
        {
            NS_ASSERT_MSG(false, "Incompatible address type: " << ip);
        }
        tab_socket[idx]->SetAllowBroadcast(false);
        map_neighbor_device.insert(std::pair<uint32_t, uint32_t>(idx, deviceID));
    }
    else
    {
        std::cout << "create an existing socket" << std::endl;
    }
}
void overlayApplication::SetRecvSocket(void)
{
    /**
     * Set up socket for forwarding
     **/
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    recv_socket = Socket::CreateSocket(GetNode(), tid);
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), ListenPort);
    if (recv_socket->Bind(local) == -1)
    {
        NS_FATAL_ERROR("Failed to bind socket");
    }
    recv_socket->SetRecvCallback(MakeCallback(&overlayApplication::HandleRead, this));
}

void overlayApplication::HandleRead(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this << socket);

        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        while ((packet = socket->RecvFrom(from)))
        {
            socket->GetSockName(localAddress);
            m_rxTrace(packet);
            m_rxTraceWithAddresses(packet, from, localAddress);

            // std::cout << "Node ID: " << m_local_ID << "; pkt received" << std::endl;
            SDtag tagPktRecv;
            packet->PeekPacketTag(tagPktRecv);
            std::string keys{std::to_string(tagPktRecv.GetSourceID()) + ' ' + std::to_string(tagPktRecv.GetDestID())};

            std::vector<int> &routes = meta->routing_map[keys];
            
            if (tagPktRecv.GetDestID() == GetLocalID())
            {
                if (tagPktRecv.GetIsProbe() > 0)
                {
                    switch (meta->probe_type)
                    {
                        case ProbeType::naive:
                        {
                            uint32_t idx_tunnel = meta->tunnel_hashmap[keys];
                            meta->cnt_queuing[idx_tunnel][tagPktRecv.GetPktID()] = tagPktRecv.GetIsQueued();
                            break;
                        }
                        case ProbeType::sandwich_v1:
                        {
                            if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST && tagPktRecv.GetSandWichLargeID() == 4)
                            {
                                std::cout << SRC << " - " << DEST << " with large: " << 4 << " -PktID=" << tagPktRecv.GetPktID() << " sandwithID = " << (uint32_t)tagPktRecv.GetSandWichID() << ": " << Simulator::Now().GetMicroSeconds() << std::endl;
                            }
                            if (tagPktRecv.GetSandWichID() == 1){}
                            else meta->update_log_sandwich_v1(tagPktRecv.GetSourceID(), tagPktRecv.GetDestID(), tagPktRecv.GetSandWichLargeID(), tagPktRecv.GetPktID());
                            break;
                        }
                        default:
                            break;
                    }
                    
                    // std::cout << m_local_ID << ": recv probe at " << Simulator::Now().As(Time::US) << " with " << keys << std::endl; 
                }
                /* if (tagPktRecv.GetIsProbe() == 0)
                {
                    std::cout << m_local_ID << ": recv background at " << Simulator::Now().As(Time::US) << " with " << keys << std::endl; 
                } */
            }
            else
            {
                // std::cout << "Source ID: " << (uint32_t)tagPktRecv.GetSourceID() << ", target ID: " << (uint32_t)tagPktRecv.GetDestID() << ", this hop" << m_local_ID << ", next hop" << routes[tagPktRecv.GetCurrentHop() + 1] << std::endl;
                assert(routes[tagPktRecv.GetCurrentHop()] == m_local_ID);
                /* if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST)
                {
                    std::cout << "Node ID: " << m_local_ID << " forward at: " << Simulator::Now().ToDouble(Time::US) << std::endl;
                } */
                
                if ( CheckCongestion(map_neighbor_device[routes[tagPktRecv.GetCurrentHop()+1]], (uint32_t)tagPktRecv.GetSourceID(), (uint32_t)tagPktRecv.GetDestID(), (uint16_t)tagPktRecv.GetPktID()) )
                {
                    tagPktRecv.SetIsQueued(1);
                }
                tagPktRecv.AddCurrentHop();
                packet->ReplacePacketTag(tagPktRecv);
                tab_socket[routes[tagPktRecv.GetCurrentHop()]]->Send(packet);
            }
        }
    }


}