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

    void overlayApplication::InitApp(netw & meta, uint32_t localId)
    {
        m_sent.resize(meta.n_nodes, 0);
        //m_count.resize(meta.n_nodes, 0);
        tab_socket.resize(meta.n_nodes, 0);
        tab_peerAddress.resize(meta.n_nodes);
        m_interval.resize(meta.n_nodes);
        // m_socket = 0;
        m_peerPort = 9;
        recv_socket = 0;
        m_sendEvent = EventId();
        is_overlay = meta.loc_overlay_nodes[GetLocalID()];
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
    void overlayApplication::SetInterval(uint32_t idx, uint32_t Interval)
    {
        NS_LOG_FUNCTION(this);
        m_interval[idx] = Time( std::to_string(Interval) + 's' );
    }
    uint16_t overlayApplication::GetPort(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_peerPort;
    }
    void overlayApplication::SetRemote(Address ip, uint16_t idx)
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
    }
    void overlayApplication::StartApplication(void)
    {
        NS_LOG_FUNCTION(this);
        /**
         * Set up socket for initiating flows
         **/
        if (tab_socket.size() == 0)
        {
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            for (uint16_t i = 0; i < tab_peerAddress.size(); i++)
            {
                tab_socket.emplace_back(Socket::CreateSocket(GetNode(), tid));
                if (Ipv4Address::IsMatchingType(tab_peerAddress[i]) == true)
                {
                    if (tab_socket.back()->Bind() == -1)
                    {
                        NS_FATAL_ERROR("Failed to bind socket");
                    }
                    tab_socket.back()->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(tab_peerAddress[i]), m_peerPort));
                }
                else if (InetSocketAddress::IsMatchingType(tab_peerAddress[i]) == true)
                {
                    if (tab_socket.back()->Bind() == -1)
                    {
                        NS_FATAL_ERROR("Failed to bind socket");
                    }
                    tab_socket.back()->Connect(tab_peerAddress[i]);
                }
                else
                {
                    NS_ASSERT_MSG(false, "Incompatible address type: " << tab_peerAddress[i]);
                }
                tab_socket.back()->SetAllowBroadcast(false);
                ScheduleTransmit(Seconds(0.), i);
            }
        }
        /**
         * Set up socket for forwarding
         **/
        if (recv_socket == 0)
        {
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            recv_socket = Socket::CreateSocket(GetNode(), tid);
            InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), ListenPort);
            if (recv_socket->Bind(local) == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            recv_socket->SetRecvCallback(MakeCallback(&overlayApplication::HandleRead, this));
        }
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
            if (InetSocketAddress::IsMatchingType(from))
            {
                NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " server received " << packet->GetSize() << " bytes from " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(from).GetPort());
            }

            SDtag tagPktRecv;
            packet->PeekPacketTag(tagPktRecv);
            // packet->PrintPacketTags(std::cout);
            tagPktRecv.Print(std::cout);

            packet->RemoveAllPacketTags();
            packet->RemoveAllByteTags();

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
        m_sendEvent = Simulator::Schedule(dt, &overlayApplication::Send, this, idx);
    }
    void overlayApplication::Send(uint16_t idx)
    {
        NS_LOG_FUNCTION(this);
        NS_ASSERT(m_sendEvent.IsExpired());
        assert(idx <= tab_socket.size() - 1);

        Ptr<Packet> p;
        p = Create<Packet>(m_size);
        Address localAddress;
        tab_socket[idx]->GetSockName(localAddress);
        // call to the trace sinks before the packet is actually sent,
        // so that tags added to the packet can be sent as well
        m_txTrace(p);
        if (Ipv4Address::IsMatchingType(tab_peerAddress[idx]))
        {
            m_txTraceWithAddresses(p, localAddress, InetSocketAddress(Ipv4Address::ConvertFrom(tab_peerAddress[idx]), m_peerPort));
        }
        SDtag tagToSend;
        tagToSend.SetSourceID(0);
        tagToSend.SetDestID(1);
        tagToSend.Print(std::cout);
        p->AddPacketTag(tagToSend);
        SDtag tagCheck;
        p->PeekPacketTag(tagCheck);
        tagCheck.Print(std::cout);

        tab_socket[idx]->Send(p);
        ++m_sent[idx];

        if (Ipv4Address::IsMatchingType(tab_peerAddress[idx]))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size << " bytes to " << Ipv4Address::ConvertFrom(tab_peerAddress[idx]) << " port " << m_peerPort);
        }
        else if (InetSocketAddress::IsMatchingType(tab_peerAddress[idx]))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size << " bytes to " << InetSocketAddress::ConvertFrom(tab_peerAddress[idx]).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(tab_peerAddress[idx]).GetPort());
        }

        if (m_sent[idx] < m_count[idx])
        {
            ScheduleTransmit(m_interval[idx], idx);
        }
    }
    void overlayApplication::StopApplication()
    {
        NS_LOG_FUNCTION(this);

        if (tab_socket.size() != 0)
        {
            for (uint16_t i = 0; i < m_count[i]; i++)
            {
                tab_socket[i]->Close();
                tab_socket[i]->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
            }
        }
        if (recv_socket != 0)
        {
            recv_socket->Close();
            recv_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }
    }
}