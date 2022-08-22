#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/udp-socket.h"
#include "ns3/log.h"

namespace ns3
{

#define SRC 17
#define DEST 3

typedef struct name_files
{
    std::string netw_filename;
    std::string demands_file;
    std::string file_overlay_nodes;
    std::string route_name;
    std::string probe_setup_filename;
}name_input_files;

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

void trace_NetDeviceMacTxDrop(std::string context, Ptr<const Packet> packet);
void trace_NetDevicePhyTxDrop(std::string context, Ptr<const Packet> packet);
void trace_NetDevicePhyRxDrop(std::string context, Ptr<const Packet> packet);
void trace_NetDeviceQueueDrop(std::string context, Ptr<const Packet> packet);
void trace_NetDeviceDropBeforeEnqueue(std::string context, Ptr<const Packet> packet);
void trace_NetDeviceQueueEnqueue(std::string context, Ptr<const Packet> packet);

void trace_TCDrop(Ptr<const Packet> packet);



void trace_Ipv4L3PDrop(std::string context, const Ipv4Header &header, Ptr< const Packet > packet, Ipv4L3Protocol::DropReason reason, Ptr< Ipv4 > ipv4, uint32_t interface);


}

#endif