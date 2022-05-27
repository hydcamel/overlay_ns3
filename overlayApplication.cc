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
                                /* .AddAttribute("MaxPackets",
                                              "The maximum number of packets the application will send",
                                              UintegerValue(10),
                                              MakeUintegerAccessor(&overlayApplication::m_count),
                                              MakeUintegerChecker<uint32_t>())
                                .AddAttribute("Interval",
                                              "The time to wait between packets",
                                              TimeValue(Seconds(1.0)),
                                              MakeTimeAccessor(&overlayApplication::m_interval),
                                              MakeTimeChecker())
                                .AddAttribute("RemoteAddress",
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
                                              MakeUintegerChecker<uint16_t>())
                                .AddAttribute("PacketSize", "Size of echo data in outbound packets",
                                              UintegerValue(100),
                                              MakeUintegerAccessor(&overlayApplication::SetDataSize,
                                                                   &overlayApplication::GetDataSize),
                                              MakeUintegerChecker<uint32_t>())
                                .AddTraceSource("Tx", "A new packet is created and is sent",
                                                MakeTraceSourceAccessor(&overlayApplication::m_txTrace),
                                                "ns3::Packet::TracedCallback")
                                .AddTraceSource("Rx", "A packet has been received",
                                                MakeTraceSourceAccessor(&overlayApplication::m_rxTrace),
                                                "ns3::Packet::TracedCallback")
                                .AddTraceSource("TxWithAddresses", "A new packet is created and is sent",
                                                MakeTraceSourceAccessor(&overlayApplication::m_txTraceWithAddresses),
                                                "ns3::Packet::TwoAddressTracedCallback")
                                .AddTraceSource("RxWithAddresses", "A packet has been received",
                                                MakeTraceSourceAccessor(&overlayApplication::m_rxTraceWithAddresses),
                                                "ns3::Packet::TwoAddressTracedCallback");
        return tid;
    }

    /* overlayApplication::overlayApplication(int id)
    {
        NS_LOG_FUNCTION(this);
        m_sent.resize(1, 0);
        // m_socket = 0;
        m_peerPort = 9;
        recv_socket = 0;
        m_sendEvent = EventId();
    } */
    overlayApplication::overlayApplication()
    {
        NS_LOG_FUNCTION(this);
    }

    void overlayApplication::InitApp(netw *netw_meta, uint32_t localId, uint32_t MaxPktSize)
    {
        meta = netw_meta;
        // m_sent.resize(meta->n_nodes, 0);
        m_count.resize(meta->n_nodes, MaxPktSize);
        m_interval.resize(meta->n_nodes);
        // m_socket = 0;
        m_peerPort = 9;
        m_sendEvent.resize(meta->n_nodes, EventId());
        SetLocalID(localId);
        is_overlay = meta->loc_overlay_nodes[localId];
    }

    overlayApplication::~overlayApplication()
    {
        NS_LOG_FUNCTION(this);
        m_count.clear();
        m_interval.clear();
        tab_neighborAddress.clear();
        map_neighbor_device.clear();
        pkt_num_table.clear();
        // recv_socket = 0;
        // m_socket = 0;
    }

    void overlayApplication::DoDispose(void)
    {
        NS_LOG_FUNCTION(this);
        Application::DoDispose();
    }
    uint32_t overlayApplication::GetDataSize(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_size;
    }
    void overlayApplication::SetDataSize(uint32_t dataSize)
    {
        NS_LOG_FUNCTION(this);
        m_size = dataSize;
    }
    void overlayApplication::SetLocalID(uint32_t localID)
    {
        NS_LOG_FUNCTION(this);
        m_local_ID = localID;
    }
    uint16_t overlayApplication::GetLocalID(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_local_ID;
    }
    void overlayApplication::SetNeighbor(Address macAddr, uint32_t idx)
    {
        tab_neighborAddress.insert( std::pair<uint32_t, Address>(idx, macAddr) );
    }
    Address overlayApplication::GetNeighbor(uint32_t idx)
    {
        if (tab_neighborAddress.count(idx) != 0)
        {
            return tab_neighborAddress[idx];
        }
        else
        {
            std::cout << "Get non-existing neighbor" << std::endl;
            exit(-1);
        }
    }
    void overlayApplication::SetInterval(uint32_t idx, float Interval)
    {
        NS_LOG_FUNCTION(this);
        m_interval[idx] = Time(std::to_string(Interval) + 's');
    }
    void overlayApplication::SetProbeInterval(float Interval)
    {
        NS_LOG_FUNCTION(this);
        probe_interval = Time(std::to_string(Interval) + 's');
    }
    uint16_t overlayApplication::GetPort(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_peerPort;
    }
    void overlayApplication::StartApplication(void)
    {
        NS_LOG_FUNCTION(this);
        /**
         * Set up socket for background flows: to each underlay neighbor
         **/
        std::unordered_map<uint32_t, Address>::iterator it;
        for (it = tab_neighborAddress.begin(); it != tab_neighborAddress.end(); it++)
        {
            /* code */
        }
        
        std::map<std::string, float>::iterator it;
        if (meta->loc_overlay_nodes[GetLocalID()] == true) // only if self is overlay, it can schedule flows
        {
            for (uint32_t i = 0; i < meta->n_nodes; i++)
            {
                if (meta->loc_overlay_nodes[i] == true) // target i
                {
                    it = meta->overlay_demands.find(std::to_string(GetLocalID()) + " " + std::to_string(i));
                    if (it == meta->overlay_demands.end())
                        continue; // no such demands
                    // set interval
                    SetInterval(i, float(MACPktSize * 8) / it->second); // flow rate for the target i: bps
                    // tab_socket[routes[1]]->SetAllowBroadcast(false);
                    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
                    Time random_offset = MicroSeconds(rand->GetValue(50, 200));
                    meta->time_span_flows[std::to_string(m_local_ID) + ' ' + std::to_string(i)] = random_offset.ToDouble(Time::US);
                    if (m_local_ID == SRC && i == DEST)
                    {
                        std::cout << "Node ID: " << m_local_ID << " start at " << random_offset.ToDouble(Time::US) << std::endl;
                    }

                    ScheduleTransmit(random_offset, i);
                }
            }
        }
    }

    void overlayApplication::HandleRead(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this << socket);

        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        double time_trans = 0;
        while ((packet = socket->RecvFrom(from)))
        {
            socket->GetSockName(localAddress);
            m_rxTrace(packet);
            m_rxTraceWithAddresses(packet, from, localAddress);
            /* if (InetSocketAddress::IsMatchingType(from))
            {
                NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " server received " << packet->GetSize() << " bytes from " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(from).GetPort());
            } */
            // std::cout << "Node ID: " << m_local_ID << "; pkt received" << std::endl;
            SDtag tagPktRecv;
            packet->PeekPacketTag(tagPktRecv);
            std::string keys{std::to_string(tagPktRecv.GetSourceID()) + ' ' + std::to_string(tagPktRecv.GetDestID())};
            /* if (meta->routing_map.count(keys) == 0)
            {
                std::cout << "Wrong key: " << keys << " at " << m_local_ID << std::endl;
            } */
            /* if ((uint32_t)tagPktRecv.GetSourceID() == 4 && (uint32_t)tagPktRecv.GetDestID() == 19)
            {
                std::cout << keys << " at " << m_local_ID << std::endl;
            } */

            std::vector<int> &routes = meta->routing_map[keys];
            // packet->PrintPacketTags(std::cout);
            // tagPktRecv.Print(std::cout);
            if (CheckPktSanity(keys, tagPktRecv.GetPktID()) == false)
            {
                NotifyRetransmission((uint32_t)tagPktRecv.GetSourceID(), (uint32_t)tagPktRecv.GetDestID(), pkt_num_table[keys]);
                continue;
            }
            
            if (tagPktRecv.GetDestID() == GetLocalID())
            {
                /* if (meta->cnt_pkt.count(keys) == 0)
                {
                    std::cout << "Wrong key: " << keys << " at " << m_local_ID << std::endl;
                } */
                // std::cout << "Node ID: " << GetLocalID() << ": A packet received from " << (uint32_t)tagPktRecv.GetSourceID() << std::endl;
                time_trans += double(Simulator::Now().ToInteger(Time::NS) - tagPktRecv.GetStartTime()) / NSTOUS;
                // std::cout << tagPktRecv.GetSourceID() << " - " << tagPktRecv.GetDestID() << ": now " << Simulator::Now().ToInteger(Time::NS) << " start: " << tagPktRecv.GetStartTime() << " = " << time_trans << std::endl;
                meta->average_delay[keys] += time_trans;
                meta->cnt_pkt[keys]++;
                if (meta->cnt_pkt[keys] == MAXPKTNUM)
                {
                    // std::cout << keys << ": " << meta->cnt_pkt[keys] << "; MAXPKTNUM = " << MAXPKTNUM << std::endl;
                    meta->time_span_flows[keys] = Simulator::Now().ToDouble(Time::US) - meta->time_span_flows[keys];
                }

                // packet->RemoveAllPacketTags();
                // packet->RemoveAllByteTags();
            }
            else
            {
                // std::cout << "Source ID: " << (uint32_t)tagPktRecv.GetSourceID() << ", target ID: " << (uint32_t)tagPktRecv.GetDestID() << ", this hop" << m_local_ID << ", next hop" << routes[tagPktRecv.GetCurrentHop() + 1] << std::endl;
                assert(routes[tagPktRecv.GetCurrentHop()] == m_local_ID);
                /* if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST)
                {
                    std::cout << "Node ID: " << m_local_ID << " forward at: " << Simulator::Now().ToDouble(Time::US) << std::endl;
                } */
                tagPktRecv.AddCurrentHop();
                packet->ReplacePacketTag(tagPktRecv);
                CheckCongestion(map_neighbor_device[routes[tagPktRecv.GetCurrentHop()]], (uint32_t)tagPktRecv.GetSourceID(), (uint32_t)tagPktRecv.GetDestID(), (uint16_t)tagPktRecv.GetPktID());
                tab_socket[routes[tagPktRecv.GetCurrentHop()]]->Send(packet);
            }
            // NS_LOG_LOGIC("Echoing packet");
            // socket->SendTo(packet, 0, from);

            // if (InetSocketAddress::IsMatchingType(from))
            // {
            //     NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " server sent " << packet->GetSize() << " bytes to " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(from).GetPort());
            // }
        }
    }
    void overlayApplication::ScheduleTransmit(Time dt, uint16_t idx)
    {
        NS_LOG_FUNCTION(this << dt);
        m_sendEvent[idx] = Simulator::Schedule(dt, &overlayApplication::Send, this, idx);
    }
    void overlayApplication::Send(uint16_t idx)
    {
        NS_LOG_FUNCTION(this);
        NS_ASSERT(m_sendEvent[idx].IsExpired());
        std::vector<int> &routes = meta->routing_map[std::to_string(m_local_ID) + " " + std::to_string(idx)];

        Ptr<Packet> p;
        p = Create<Packet>(m_size);
        // Address localAddress;
        // tab_socket[routes[1]]->GetSockName(localAddress);
        // call to the trace sinks before the packet is actually sent,
        // so that tags added to the packet can be sent as well
        m_txTrace(p);
        /* if (Ipv4Address::IsMatchingType(tab_peerAddress[idx]))
        {
            m_txTraceWithAddresses(p, localAddress, InetSocketAddress(Ipv4Address::ConvertFrom(tab_peerAddress[idx]), m_peerPort));
        } */
        SDtag tagToSend;
        tagToSend.SetSourceID(m_local_ID);
        tagToSend.SetDestID(idx);
        tagToSend.SetCurrentHop(1);
        tagToSend.SetPktID(meta->m_sent[m_local_ID][idx]);
        // tagToSend.SetPktID(m_sent[idx]);
        // std::cout << Simulator::Now().As(Time::NS) << std::endl;
        tagToSend.SetStartTime(Simulator::Now().ToInteger(Time::NS));

        // std::cout << "before add" << std::endl;
        // tagToSend.Print(std::cout);
        p->AddPacketTag(tagToSend);
        // std::cout << "after add" << std::endl;
        /* SDtag tagCheck;
        p->PeekPacketTag(tagCheck);
        tagCheck.Print(std::cout); */

        // std::cout << "Source ID: " << m_local_ID << ", target ID: " << idx << ", next hop" << routes[1] << "at time: " << tagToSend.GetStartTime() << " " << Simulator::Now().As(Time::NS) << std::endl;

        // CheckCongestion(map_neighbor_device[routes[1]], m_local_ID, idx, (uint16_t)m_sent[idx]);
        CheckCongestion(map_neighbor_device[routes[1]], m_local_ID, idx, meta->m_sent[m_local_ID][idx]);
        tab_socket[routes[1]]->Send(p);
        meta->m_sent[m_local_ID][idx]++;
        // ++m_sent[idx];

        /* if (Ipv4Address::IsMatchingType(tab_peerAddress[idx]))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size << " bytes to " << Ipv4Address::ConvertFrom(tab_peerAddress[idx]) << " port " << m_peerPort);
        }
        else if (InetSocketAddress::IsMatchingType(tab_peerAddress[idx]))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size << " bytes to " << InetSocketAddress::ConvertFrom(tab_peerAddress[idx]).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(tab_peerAddress[idx]).GetPort());
        } */

        if (meta->m_sent[m_local_ID][idx] < m_count[idx])
        {
            ScheduleTransmit(m_interval[idx], idx);
        }
        if (meta->m_sent[m_local_ID][idx] >= m_count[idx] && m_local_ID == SRC && tagToSend.GetDestID() == DEST)
        {
            std::cout << "Node ID: " << m_local_ID << " ends at " << Simulator::Now().ToDouble(Time::US) << std::endl;
        }
    }
    void overlayApplication::StopApplication()
    {
        NS_LOG_FUNCTION(this);
        if (m_local_ID == 0)
        {
            meta->write_average_delay("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/average_delay.txt");
            meta->write_congestion_cnt("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/congestion_cnt.txt");
        }

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

    void overlayApplication::CheckCongestion(uint32_t deviceID, uint32_t src, uint32_t dest, uint16_t PktID)
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
        }
        else
        {
            // std::cout << "No congestion at " << m_local_ID << "from " << src << " to " << dest << "with " << net_queue->GetNPackets() << " " << net_queue->GetNBytes() << std::endl;
        }
    }

    void overlayApplication::CheckCongestion(Ptr<Socket> skt, uint32_t src, uint32_t dest)
    {
        NS_LOG_FUNCTION(this);
        Ptr<NetDevice> net_raw = skt->GetBoundNetDevice();
        Ptr<PointToPointNetDevice> net_device = StaticCast<PointToPointNetDevice>(net_raw);
        Ptr<Queue<Packet>> net_queue = net_device->GetQueue();
        // Ptr<Queue<Packet>> net_queue = StaticCast<PointToPointNetDevice>(skt->GetBoundNetDevice())->GetQueue();
        std::cout << "src: " << src << " -dest: " << dest << " queue backlog: " << net_queue->GetNPackets() << std::endl;
        if (net_queue->GetNPackets() > 0)
        {
            std::cout << "congestion at " << m_local_ID << "from " << src << " to " << dest << "with " << std::endl;
            meta->cnt_congestion[std::to_string(src) + ' ' + std::to_string(dest)]++;
        }
    }

    void overlayApplication::NotifyRetransmission(uint32_t src, uint32_t dest, uint32_t valPktID)
    {
        /* if (src == SRC && dest == DEST)
        {
            std::cout << "At Node" << m_local_ID << " retransmission from " << src << " to " << dest << " as expecting " << valPktID << std::endl;
        } */
        
        meta->notify_pktLoss(src, dest, valPktID);
        //std::cout << "At" << Simulator::Now().As(Time::US) << " m_sent =  " << meta->m_sent[src][dest] << std::endl;
    }
    // void overlayApplication::SetMSent(uint32_t idx, uint32_t val)
    // {
    //     m_sent[idx] = val;
    // }
    bool overlayApplication::CheckPktSanity(std::string SDKey, uint32_t newID)
    {
        if (pkt_num_table.count(SDKey) == 0)
        {
            pkt_num_table.insert(std::pair<std::string, uint32_t> (SDKey, 0));
        }
        if (newID > pkt_num_table[SDKey])
        {
            return false;
        }
        else
        {
            pkt_num_table[SDKey] ++;
            return true;
        }
        
    }
}