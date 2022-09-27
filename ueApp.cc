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
    oa_interface = &app_interface;
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    recv_socket = Socket::CreateSocket(GetNode(), tid);
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), NRPORT);
    if (recv_socket->Bind(local) == -1)
    {
        std::cout << "Failed to bind socket" << std::endl;
        // NS_FATAL_ERROR("Failed to bind socket");
    }
    // else std::cout << "UE bind socket" << std::endl;
    recv_socket->SetRecvCallback(MakeCallback(&ueApp::HandleRead, this));
    /* if (local_ID_ == 3 || local_ID_ == 4 || local_ID_ == 6)
    {
        Address tmpAddr;
        recv_socket->GetSockName(tmpAddr);
        std::cout << "UE ID: " << local_ID_ << " - "  << tmpAddr << std::endl;
    } */
    max_probes = app_interface.meta->_MAXPKTNUM;
    // std::cout << "UE node Init: " << local_ID_ << std::endl;
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

        // ueTag tagPktRecv;
        SDtag tagPktRecv;
        packet->PeekPacketTag(tagPktRecv);
        // std::cout << "UE ID: " << local_ID_ << "; pkt received with start-time: " << (uint64_t)(tagPktRecv.GetStartTime()) << std::endl;
        // std::cout << GetNode()->GetId() << "-Received: " << uint32_t(tagPktRecv.GetSourceID()) << " to " << uint32_t(tagPktRecv.GetDestID()) << " with ID " << tagPktRecv.GetPktID() << " at " << "\t" << Now() << " with start time " << tagPktRecv.GetStartTime() << std::endl;
        std::string keys_ = std::to_string(tagPktRecv.GetSourceID()) + " " + std::to_string(tagPktRecv.GetDestID());
        oa_interface->meta->cnt_delays[keys_][tagPktRecv.GetPktID()] = Simulator::Now().GetNanoSeconds() - (uint64_t)(tagPktRecv.GetStartTime());
        oa_interface->meta->is_received[keys_] = true;
        cnt_probes ++;
        // if(cnt_probes >= max_probes) StopApplication();
    }
}

void ueApp::StartApplication(void)
{
    // std::cout << "UE_App Start at: " << local_ID_ << " " << std::endl;
    NS_LOG_FUNCTION(this);
}
void ueApp::StopApplication(void)
{
    // std::cout << "UE ID: " << local_ID_ << "; Stopped" << std::endl;
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
