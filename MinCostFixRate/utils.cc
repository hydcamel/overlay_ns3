#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "utils.h"
namespace ns3
{


NS_LOG_COMPONENT_DEFINE("utils");
//NS_OBJECT_ENSURE_REGISTERED(utils);


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