#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "utils.h"
namespace ns3{

NS_LOG_COMPONENT_DEFINE("utils");
//NS_OBJECT_ENSURE_REGISTERED(utils);

TypeId SDtag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SDtag")
    .SetParent<Tag> ()
    .AddConstructor<SDtag> ()
    .AddAttribute ("SourceID",
                   "ID of the Source",
                   EmptyAttributeValue (),
                   MakeUintegerAccessor (&SDtag::GetSourceID),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DestID",
                   "ID of the Destiny",
                   EmptyAttributeValue (),
                   MakeUintegerAccessor (&SDtag::GetDestID),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

TypeId SDtag::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}
uint32_t SDtag::GetSerializedSize (void) const
{
    return 2;
}
void SDtag::Serialize (TagBuffer i) const
{
    i.WriteU8 (SourceID);
    i.WriteU8 (DestID);
}
void SDtag::Deserialize (TagBuffer i)
{
    SourceID = i.ReadU8 ();
    DestID = i.ReadU8 ();
}
void SDtag::Print (std::ostream &os) const
{
    os << "source=" << (uint32_t)SourceID << ", Dest=" << (uint32_t)DestID << std::endl;
}
void SDtag::SetSourceID (uint8_t value)
{
    SourceID = value;
}
uint8_t SDtag::GetSourceID (void) const
{
    return SourceID;
}
void SDtag::SetDestID (uint8_t value)
{
    DestID = value;
}
uint8_t SDtag::GetDestID (void) const
{
    return DestID;
}

void receivePkt(Ptr<Socket> skt){
    NS_LOG_INFO ("Received one packet!");
    Ptr<Packet> packet = skt->Recv ();
    SDtag tagPktRecv;
    packet->PeekPacketTag( tagPktRecv );
    packet->PrintPacketTags(std::cout);
    std::cout << std::endl;
}

void SendPacket (Ptr<Socket> socket, uint32_t pktSize, uint32_t pktCount, Time pktInterval, uint8_t SourceID, uint8_t DestID ){
    std::cout << "pktCount = " << pktCount << std::endl;
    if (pktCount > 0){
        Ptr<Packet> pktToSend = Create<Packet> (pktSize);
        SDtag tagToSend;
        tagToSend.SetSourceID(SourceID);
        tagToSend.SetDestID(DestID);
        pktToSend->AddPacketTag( tagToSend );
        socket->Send(pktToSend);
        Simulator::Schedule( pktInterval, &SendPacket, socket, pktSize, pktCount-1, pktInterval, SourceID, DestID );
    }
    else{
        socket->Close();
    }
}

void rxTraceIpv4(std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ptr_ipv4, uint32_t dontknow){
    std::cout << context << "\t" << Now() << ": packet received with size: " << packet->GetSize() << std::endl;
}
void txTraceIpv4(std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ptr_ipv4, uint32_t dontknow){
    std::cout << context << "\t" << Now() << ": packet sent with size: " << packet->GetSize() << std::endl;
}
void p2pDevMacTx(std::string context, Ptr<const Packet> packet){
    std::cout << context << "\t" << Now() << ": packet sent from NetDev with size: " << packet->GetSize() << std::endl;
}
void p2pDevMacRx(std::string context, Ptr<const Packet> packet){
    std::cout << context << "\t" << Now() << ": packet received from NetDev with size:" << packet->GetSize() << std::endl;
}
void LocalDeliver(std::string context, Ptr<const Packet> packet, Ipv4Header const &ip, uint32_t iif){
    std::cout << ": packet received for me" << std::endl;
}
void trace_udpClient(std::string context, Ptr<const Packet> packet){
    std::cout << context << "\t" << Now() << ": packet received with size: " << packet->GetSize() << std::endl;
}
void trace_PhyTxBegin(std::string context, Ptr<const Packet> packet){
    std::cout << context << "\t" << Now() << ": PHY sent begin with size: " << packet->GetSize() << std::endl;
}
void trace_PhyTxEnd(std::string context, Ptr<const Packet> packet){
    std::cout << context << "\t" << Now() << ": PHY sent end with size: " << packet->GetSize() << std::endl;
}
void trace_PhyRxBegin(std::string context, Ptr<const Packet> packet){
    std::cout << context << "\t" << Now() << ": PHY received begin with size: " << packet->GetSize() << std::endl;
}
void trace_PhyRxEnd(std::string context, Ptr<const Packet> packet){
    std::cout << context << "\t" << Now() << ": PHY received end with size: " << packet->GetSize() << std::endl;
}
void trace_txrxPointToPoint(std::string context, Ptr<const Packet> packet, Ptr<NetDevice> src, Ptr<NetDevice> dest, Time trans, Time recv){
    std::cout << context << "src device: " << src->GetAddress() << ", dest device: " << dest->GetAddress() << ". trans: " << trans << "; recv: " << recv << std::endl;
}

}