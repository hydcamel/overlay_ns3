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
#include "SDtag.h"
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
TypeId overlayApplication::GetInstanceTypeId (void) const
{
  	return overlayApplication::GetTypeId ();
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
        // m_rxTrace(packet);
        // m_rxTraceWithAddresses(packet, from, localAddress);

        // std::cout << "Node ID: " << m_local_ID << "; pkt received" << std::endl;
        SDtag tagPktRecv;
        packet->PeekPacketTag(tagPktRecv);
        std::string keys{std::to_string(tagPktRecv.GetSourceID()) + ' ' + std::to_string(tagPktRecv.GetDestID())};
        // NS_LOG_INFO("Node ID: " << m_local_ID << "; pkt received -- " << keys);

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
                        if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST)
                        {
                            std::cout << SRC << " - " << DEST << " -PktID=" << tagPktRecv.GetPktID() << " received at: " << Simulator::Now().GetMicroSeconds() << std::endl;
                            // std::cout << SRC << " - " << DEST << " with large: " << 4 << " -PktID=" << tagPktRecv.GetPktID() << " sandwithID = " << (uint32_t)tagPktRecv.GetSandWichID() << ": " << Simulator::Now().GetMicroSeconds() << std::endl;
                        }
                        // if (tagPktRecv.GetSandWichID() == 1){}
                        // else meta->update_log_sandwich_v1(tagPktRecv.GetSourceID(), tagPktRecv.GetDestID(), tagPktRecv.GetSandWichLargeID(), tagPktRecv.GetPktID());
                        break;
                    }
                    case ProbeType::calibration:
                    {
                        break;
                    }
                    default:
                        break;
                }
                // NS_LOG_INFO(m_local_ID << ": recv probe at " << Simulator::Now().As(Time::US) << " with " << keys);
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
            if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST)
            {
                std::cout << "Node ID: " << m_local_ID << " forward at: " << Simulator::Now().ToDouble(Time::US) << std::endl;
            }
            
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
bool overlayApplication::CheckCongestion(uint32_t deviceID, uint32_t src, uint32_t dest, uint16_t PktID)
{
    NS_LOG_FUNCTION(this);
    // Ptr<PointToPointNetDevice> net_device = StaticCast<PointToPointNetDevice>(GetNode()->GetDevice(deviceID));
    Ptr<NetDevice> net_raw = GetNode()->GetDevice(deviceID);
    Ptr<PointToPointNetDevice> net_device = DynamicCast<PointToPointNetDevice, NetDevice>(GetNode()->GetDevice(deviceID));
    Ptr<Queue<Packet>> net_queue = net_device->GetQueue();
    // std::cout << "src: " << src << " -dest: " << dest << " queue backlog: " << net_queue->GetNPackets() << std::endl;
    /* if (src == SRC && dest == DEST && net_queue->GetNPackets() > 0)
    {
        std::cout << "Node ID: " << m_local_ID << "PktID: " << PktID << "src: " << src << " -dest: " << dest << " queue backlog: " << net_queue->GetNPackets() << " limit = " << net_queue->GetMaxSize() << std::endl;
    } */

    if (net_queue->GetNPackets() > 0)
    {
        // std::cout << "congestion at " << m_local_ID << "from " << src << " to " << dest << "with " << net_queue->GetNPackets() << " " << net_queue->GetNBytes() << std::endl;
        meta->cnt_congestion[std::to_string(src) + ' ' + std::to_string(dest)] += net_queue->GetNPackets();
        return true;
    }
    else
    {
        return false;
    }
}

uint32_t overlayApplication::GMM_Pkt_Size(void)
{
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    double rng_val = rand->GetValue(0.0, 1.0);
    if (rng_val <= PrLBPkt) return LBPKTSIZE;
    else if (rng_val <= (PrLBPkt + PrUBPkt)) return UBPKTSIZE;
    else return MEDPKTSIZE;
}

void overlayApplication::ScheduleBackground(Time dt, uint32_t idx)
{
    m_sendEvent[idx] = Simulator::Schedule(dt, &overlayApplication::SendBackground, this, idx);
}
void overlayApplication::SendBackground(uint32_t idx)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_sendEvent[idx].IsExpired());
    SDtag tagToSend;
    tagToSend.SetSourceID(m_local_ID);
    tagToSend.SetDestID(idx);
    tagToSend.SetCurrentHop(1);
    tagToSend.SetPktID(0);
    tagToSend.SetIsProbe(0);
    tagToSend.SetIsQueued(0);
    if ( meta->background_type == CrossType::PktPoisson )
    {
        // pkt size
        uint32_t pkt_size = GMM_Pkt_Size();
        Ptr<Packet> p;
        p = Create<Packet>(pkt_size);
        // m_txTrace(p);
        p->AddPacketTag(tagToSend);
        tab_socket[idx]->Send(p);
        // inter-arrival time
        Ptr<ExponentialRandomVariable> rand = CreateObject<ExponentialRandomVariable> ();
        int edgeID = 0;
        if (meta->edges.count( std::to_string(m_local_ID) + ' ' + std::to_string(idx) ))
        {
            edgeID = meta->edges[std::to_string(m_local_ID) + ' ' + std::to_string(idx)];
        }
        else if (meta->edges.count( std::to_string(idx) + ' ' + std::to_string(m_local_ID) ))
        {
            edgeID = meta->edges[std::to_string(idx) + ' ' + std::to_string(m_local_ID)];
        }
        else
        {
            std::cout << "non-existing underlay edges" << std::endl;
            exit(-1);
        }
        
        rand->SetAttribute("Mean", DoubleValue(meta->background_interval[edgeID]));
        Time pkt_inter_arrival = MicroSeconds( rand->GetInteger() );
        //std::cout << "Node ID: " << m_local_ID << " background at " << Simulator::Now().As(Time::US) << " to " << idx << " next time " << pkt_inter_arrival.As(Time::US) << std::endl; 
        // ScheduleBackground(pkt_inter_arrival, idx);
        if (is_run == true)
        {
            ScheduleBackground(pkt_inter_arrival, idx);
        }
    }
    else
    {
        std::cout << "Wrong Background Type" << std::endl;
        exit(-1);
    }
}

void overlayApplication::StartApplication(void)
{
    NS_LOG_FUNCTION(this);
    /**
     * Set up background traffic
     **/
    if (meta->background_type == CrossType::PktPoisson)
    {
        for (uint32_t j = 0; j < meta->n_nodes; j++)
        {
            if (meta->adj_mat[m_local_ID][j] == false) continue;
            Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
            Time random_offset = MicroSeconds(rand->GetValue(0, 50));
            ScheduleBackground(random_offset, j);
        }
    }

    /**
     * Set up probing flows
     **/
    if (meta->loc_overlay_nodes[GetLocalID()] == true) // only if self is overlay, it can schedule probing
    {
        switch (meta->probe_type)
        {
            case ProbeType::naive:
            {
                    for (uint32_t i = 0; i < meta->n_nodes; i++)
                    {
                        if (meta->loc_overlay_nodes[i] == true) // target i
                        {
                            std::string keys_ = std::to_string(m_local_ID) + ' ' + std::to_string(i);
                            if (meta->tunnel_hashmap.count(keys_) == 0 || meta->old_E[meta->tunnel_hashmap[keys_]] == false)
                                continue; // no such tunnel or not currently probed
                            ScheduleProbing(Time(MicroSeconds(meta->probe_normal_interval[meta->tunnel_hashmap[keys_]])), i);
                        }
                    }
                break;
            }
            case ProbeType::sandwich_v1:
            {
                Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
                Time random_offset = MicroSeconds(rand->GetValue(10, 50));
                ScheduleProbing(random_offset, 0);
                break;
            }
            case ProbeType::calibration:
            {
                break;
            }
            default:
            {
                std::cout << "wrong probe types at StartApplication" << std::endl;
                break;
            }
        }
    }
}

void overlayApplication::ScheduleProbing(Time dt, uint32_t idx)
{
    switch (meta->probe_type)
    {
        case ProbeType::naive:
        {
            probe_event[idx] = Simulator::Schedule(dt, &overlayApplication::SendProbeNaive, this, idx);
            // ScheduleProbing(probe_interval, idx);
            break;
        }
        case ProbeType::sandwich_v1:
        {
            // Simulator::Schedule(dt, &overlayApplication::OrchestraSandWichV1, this);
            break;
        }
        case ProbeType::calibration:
        {
            break;
        }
        default:
        {
            std::cout << "Wrong Background Type" << std::endl;
            exit(-1);
            break;
        }
    };
}
void overlayApplication::SendProbeNaive(uint32_t idx)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(probe_event[idx].IsExpired());
    std::string keys_ {std::to_string(m_local_ID) + " " + std::to_string(idx)};
    std::vector<int> &routes = meta->routing_map[keys_];
    SDtag tagToSend;
    SetTag(tagToSend, m_local_ID, idx, 1, meta->m_sent[m_local_ID][idx], 1);
    ++meta->m_sent[m_local_ID][idx];
    Ptr<Packet> p;
    p = Create<Packet>(ProbeSizeNaive);
    // m_txTrace(p);
    p->AddPacketTag(tagToSend);
    tab_socket[routes[1]]->Send(p);
    if (is_run == true && meta->m_sent[m_local_ID][idx] < meta->_MAXPKTNUM)
    {
        ScheduleProbing(Time(MicroSeconds(meta->probe_normal_interval[meta->tunnel_hashmap[keys_]])), idx);
    }
}

void overlayApplication::StopApplication()
{
    NS_LOG_FUNCTION(this);
    is_run = false;
    /* if (m_local_ID == 0)
    {
        // meta->write_average_delay("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/CategoryQueue/average_delay.txt");
        // meta->write_congestion_cnt("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/CategoryQueue/congestion_cnt.txt");
        meta->write_queuing_cnt("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/CategoryQueue/queuing_cnt.txt");
    } */

    // std::cout << "Node ID: " << m_local_ID << " stop Application" << std::endl;
    for (uint32_t i = 0; i < tab_socket.size(); i++)
    {
        // std::cout << "iter Node ID: " << m_local_ID << " i" << i << std::endl;
        if (tab_socket[i] != 0)
        {
            tab_socket[i]->Close();
            tab_socket[i]->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }
    }
    if (recv_socket != 0)
    {
        // std::cout << "iter Node ID: " << m_local_ID << " recv_socket" << std::endl;
        recv_socket->Close();
        recv_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
    // std::cout << "iter Node ID: " << m_local_ID << " complete" << std::endl;
}

void overlayApplication::SetTag(SDtag& tagToUse, uint8_t SourceID, uint8_t DestID, uint8_t currentHop, uint32_t PktID, uint8_t IsProbe, uint8_t IsQueued, uint8_t SandWichID, uint8_t SandWichLargeID)
{
    tagToUse.SetSourceID(SourceID);
    tagToUse.SetDestID(DestID);
    tagToUse.SetCurrentHop(currentHop);
    tagToUse.SetPktID(PktID);
    tagToUse.SetIsQueued(IsQueued);
    // tagToUse.SetSandWichID(SandWichID);
    tagToUse.SetIsProbe(IsProbe);
    // tagToUse.SetSandWichLargeID(SandWichLargeID);
}

}