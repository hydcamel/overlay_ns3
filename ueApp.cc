#include "ueApp.h"

namespace ns3
{


NS_LOG_COMPONENT_DEFINE("ueApp");
NS_OBJECT_ENSURE_REGISTERED(ueApp);

ueApp::ueApp()
{
    NS_LOG_FUNCTION(this);
}
void ueApp::initUeApp(overlayApplication &app_interface)
{
    local_ID_ = app_interface.GetLocalID();
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    recv_socket = Socket::CreateSocket(GetNode(), tid);
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), NRPORT);
    if (recv_socket->Bind(local) == -1)
    {
        // std::cout << "Failed to bind socket" << std::endl;
        NS_FATAL_ERROR("Failed to bind socket");
    }
    // else std::cout << "UE bind socket" << std::endl;
    recv_socket->SetRecvCallback(MakeCallback(&ueApp::HandleRead, this));
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
    NS_LOG_FUNCTION(this);
}
void ueApp::StopApplication(void)
{
    recv_socket->Close();
    recv_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
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
