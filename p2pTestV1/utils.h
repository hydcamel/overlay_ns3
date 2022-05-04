#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"

namespace ns3{

class SDtag : public Tag{
public:
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (TagBuffer i) const;
    virtual void Deserialize (TagBuffer i);
    virtual void Print (std::ostream &os) const;

    void SetSourceID (uint8_t value);
    uint8_t GetSourceID (void) const;
    void SetDestID (uint8_t value);
    uint8_t GetDestID (void) const;
private:
    uint8_t SourceID;
    uint8_t DestID;
};

void receivePkt(Ptr<Socket> skt);
void SendPacket (Ptr<Socket> socket, uint32_t pktSize,uint32_t pktCount, Time pktInterval, uint8_t SourceID, uint8_t DestID );
void rxTraceIpv4(std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ptr_ipv4, uint32_t dontknow);
void txTraceIpv4(std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ptr_ipv4, uint32_t dontknow);
void LocalDeliver(std::string context, Ptr<const Packet> packet, Ipv4Header const &ip, uint32_t iif);
void p2pDevMacTx(std::string context, Ptr<const Packet> packet);
void p2pDevMacRx(std::string context, Ptr<const Packet> packet);
void trace_udpClient(std::string context, Ptr<const Packet> packet);
void trace_PhyTxBegin(std::string context, Ptr<const Packet> packet);
void trace_PhyTxEnd(std::string context, Ptr<const Packet> packet);
void trace_PhyRxBegin(std::string context, Ptr<const Packet> packet);
void trace_PhyRxEnd(std::string context, Ptr<const Packet> packet);
void trace_txrxPointToPoint(std::string context, Ptr<const Packet> packet, Ptr<NetDevice> src, Ptr<NetDevice> dest, Time trans, Time recv);
}

#endif