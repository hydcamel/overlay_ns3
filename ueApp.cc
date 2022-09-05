#include "ueApp.h"

namespace ns3
{

ueApp::ueApp(uint32_t ID_associated)
{
    local_ID_ = ID_associated;
    NS_LOG_FUNCTION(this);
}
ueApp::~ueApp()
{
    NS_LOG_FUNCTION(this);
}
void ueApp::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet;
    Address localAddress;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        socket->GetSockName(localAddress);
        // m_rxTrace(packet);
        // m_rxTraceWithAddresses(packet, from, localAddress);

        ueTag tagPktRecv;
        packet->PeekPacketTag(tagPktRecv);
        std::cout << "UE ID: " << local_ID_ << "; pkt received with start-time: " << (uint64_t)(tagPktRecv.GetStartTime()) << std::endl;
    }
}

void ueApp::StartApplication(void)
{
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    recv_socket = Socket::CreateSocket(GetNode(), tid);
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), NRPORT);
    if (recv_socket->Bind(local) == -1)
    {
        NS_FATAL_ERROR("Failed to bind socket");
    }
    recv_socket->SetRecvCallback(MakeCallback(&ueApp::HandleRead, this));
    NS_LOG_FUNCTION(this);
}
void ueApp::StopApplication(void)
{
    NS_LOG_FUNCTION(this);
}

TypeId ueApp::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::ueApp")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<ueApp>();
    return tid;
}
TypeId ueApp::GetInstanceTypeId (void) const
{
  	return ueApp::GetTypeId ();
}

} // namespace ns3
