#include "ueApp.h"

namespace ns3
{

uint32_t ueTag::GetSerializedSize (void) const
{
    return 8;
}
void ueTag::Serialize (TagBuffer i) const
{
    i.WriteU64(StartTime);
}
void ueTag::Deserialize (TagBuffer i)
{
    StartTime = i.ReadU64();
}

ueApp::ueApp()
{
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

        // std::cout << "Node ID: " << m_local_ID << "; pkt received" << std::endl;
        ueTag tagPktRecv;
        packet->PeekPacketTag(tagPktRecv);
    }
}

} // namespace ns3
