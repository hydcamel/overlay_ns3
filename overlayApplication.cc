#include "overlayApplication.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable-stream.h"
#include "ns3/trace-source-accessor.h"
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
        m_sent.resize(meta->n_nodes, 0);
        m_count.resize(meta->n_nodes, MaxPktSize);
        tab_socket.resize(meta->n_nodes, 0);
        // tab_peerAddress.resize(meta.n_nodes);
        m_interval.resize(meta->n_nodes);
        // m_socket = 0;
        m_peerPort = 9;
        recv_socket = 0;
        m_sendEvent.resize(meta->n_nodes, EventId());
        SetLocalID(localId);
        is_overlay = meta->loc_overlay_nodes[localId];
    }

    overlayApplication::~overlayApplication()
    {
        NS_LOG_FUNCTION(this);
        tab_socket.clear();
        recv_socket = 0;
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
    void overlayApplication::SetCount(uint32_t MaxPackets)
    {
        NS_LOG_FUNCTION(this);
        m_count.assign(tab_socket.size(), MaxPackets);
    }
    void overlayApplication::SetInterval(uint32_t idx, float Interval)
    {
        NS_LOG_FUNCTION(this);
        m_interval[idx] = Time(std::to_string(Interval) + 's');
    }
    uint16_t overlayApplication::GetPort(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_peerPort;
    }
    void overlayApplication::SetSocket(Address ip, uint32_t idx)
    {
        NS_LOG_FUNCTION(this);
        if (tab_socket[idx] == 0)
        {
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            std::cout << "Node ID:" << m_local_ID << "set skt for " << idx << ": " << ip << std::endl;
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
        }
        else
        {
            std::cout << "create an existing socket" << std::endl;
        }
    }
    /* void overlayApplication::SetRemote(Address ip, uint16_t idx)
    {
        if (tab_peerAddress.size() - 1 < idx)
        {
            std::cout << "index out of range" << std::endl;
            exit(-1);
        }
        tab_peerAddress[idx] = ip;
    }
    void overlayApplication::AddRemote(Address ip)
    {
        tab_peerAddress.emplace_back(ip);
    } */
    void overlayApplication::StartApplication(void)
    {
        NS_LOG_FUNCTION(this);
        /**
         * Set up socket for initiating flows
         **/
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
                    SetInterval(i, float(MACPktSize * 8) / it->second); // flow rate for the target i
                    // tab_socket[routes[1]]->SetAllowBroadcast(false);
                    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
                    Time random_offset = MicroSeconds(rand->GetValue(50, 200));
                    ScheduleTransmit(random_offset, i);
                }
            }
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
        uint64_t time_trans = 0;
        while ((packet = socket->RecvFrom(from)))
        {
            socket->GetSockName(localAddress);
            m_rxTrace(packet);
            m_rxTraceWithAddresses(packet, from, localAddress);
            if (InetSocketAddress::IsMatchingType(from))
            {
                NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " server received " << packet->GetSize() << " bytes from " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(from).GetPort());
            }
            //std::cout << "Node ID: " << m_local_ID << "; pkt received" << std::endl;
            SDtag tagPktRecv;
            packet->PeekPacketTag(tagPktRecv);
            std::vector<int> &routes = meta->routing_map[std::to_string(tagPktRecv.GetSourceID()) + ' ' + std::to_string(tagPktRecv.GetDestID())];
            // packet->PrintPacketTags(std::cout);
            // tagPktRecv.Print(std::cout);
            if (tagPktRecv.GetDestID() == GetLocalID())
            {
                std::cout << "Node ID: " << GetLocalID() << ": A packet received from " << (uint32_t)tagPktRecv.GetSourceID() << std::endl;
                time_trans += Simulator::Now().ToInteger(Time::NS) - tagPktRecv.GetStartTime();
                std::cout << tagPktRecv.GetSourceID() << " - " << tagPktRecv.GetDestID() << ": now " << Simulator::Now().ToInteger(Time::NS) << " start: " << tagPktRecv.GetStartTime() << " = " << time_trans << std::endl;
                meta->average_delay[std::to_string(tagPktRecv.GetSourceID()) + ' ' + std::to_string(tagPktRecv.GetDestID())] += time_trans;
                packet->RemoveAllPacketTags();
                packet->RemoveAllByteTags();
            }
            else
            {
                std::cout << "Source ID: " << (uint32_t)tagPktRecv.GetSourceID() << ", target ID: " << (uint32_t)tagPktRecv.GetDestID() << ", this hop" << m_local_ID << ", next hop" << routes[tagPktRecv.GetCurrentHop() + 1] << std::endl;
                assert(routes[tagPktRecv.GetCurrentHop()] == m_local_ID);
                tagPktRecv.AddCurrentHop();
                packet->ReplacePacketTag(tagPktRecv);
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
        std::cout << Simulator::Now().As(Time::NS) << std::endl;
        tagToSend.SetStartTime(Simulator::Now().ToInteger(Time::NS));
        
        // std::cout << "before add" << std::endl;
        // tagToSend.Print(std::cout);
        p->AddPacketTag(tagToSend);
        // std::cout << "after add" << std::endl;
        /* SDtag tagCheck;
        p->PeekPacketTag(tagCheck);
        tagCheck.Print(std::cout); */

        std::cout << "Source ID: " << m_local_ID << ", target ID: " << idx << ", next hop" << routes[1] << "at time: " << tagToSend.GetStartTime() << " " << Simulator::Now().As(Time::NS) << std::endl;

        tab_socket[routes[1]]->Send(p);
        ++m_sent[idx];

        /* if (Ipv4Address::IsMatchingType(tab_peerAddress[idx]))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size << " bytes to " << Ipv4Address::ConvertFrom(tab_peerAddress[idx]) << " port " << m_peerPort);
        }
        else if (InetSocketAddress::IsMatchingType(tab_peerAddress[idx]))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size << " bytes to " << InetSocketAddress::ConvertFrom(tab_peerAddress[idx]).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(tab_peerAddress[idx]).GetPort());
        } */

        if (m_sent[idx] < m_count[idx])
        {
            ScheduleTransmit(m_interval[idx], idx);
        }
    }
    void overlayApplication::StopApplication()
    {
        NS_LOG_FUNCTION(this);
        if (m_local_ID == 0)
        {
            meta->write_average_delay("/home/vagrant/ns3/ns-allinone-3.35/ns-3.35/scratch/MinCostFixRate/average_delay.txt");
        }
        
        //std::cout << "Node ID: " << m_local_ID << " stop Application" << std::endl;
        for (uint32_t i = 0; i < tab_socket.size(); i++)
        {
            //std::cout << "iter Node ID: " << m_local_ID << " i" << i << std::endl;
            if (tab_socket[i] != 0)
            {
                tab_socket[i]->Close();
                tab_socket[i]->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
            }
        }
        if (recv_socket != 0)
        {
            //std::cout << "iter Node ID: " << m_local_ID << " recv_socket" << std::endl;
            recv_socket->Close();
            recv_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }
        //std::cout << "iter Node ID: " << m_local_ID << " complete" << std::endl;
    }
}