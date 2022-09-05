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
        std::cout << "Node ID: " << local_ID_ << "; pkt received with start-time: " << (uint64_t)(tagPktRecv.GetStartTime()) << std::endl;
    }
}

} // namespace ns3
