#ifndef OVERLAY_APPLICATION_H
#define OVERLAY_APPLICATION_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-socket.h"
//#include "SDtag.h"
#include <vector>
#include "utils.h"
#include "netw.h"

namespace ns3
{
    
    //extern netw netw_meta;
    class Socket;
    class Packet;

    class overlayApplication : public Application
    {
    public:
        static TypeId GetTypeId(void);

        // overlayApplication(int id);
        overlayApplication();

        virtual ~overlayApplication();
        void InitApp(netw* meta, uint32_t localId, uint32_t MaxPktSize);

        void SetRemote(Address ip, uint16_t idx);
        void AddRemote(Address ip);
        void SetSocket(Address ip, uint32_t idx, uint32_t deviceID);
        
        void SetLocalID(uint32_t localID);
        uint16_t GetLocalID(void) const;
        void SetCount(uint32_t MaxPackets);
        // void SetInterval(uint32_t idx, float Interval);
        void SetProbeInterval(float Interval);
        //std::vector<uint32_t> GetCount(void) const;

        void SetDataSize(uint32_t dataSize);
        void SetRecvSocket(void);
        void CheckCongestion(Ptr<Socket> skt, uint32_t src, uint32_t dest);
        bool CheckCongestion(uint32_t deviceID, uint32_t src, uint32_t dest, uint16_t PktID);

        uint32_t GetDataSize(void) const;
        uint16_t GetPort(void) const;
        bool is_overlay;

    protected:
        virtual void DoDispose(void);

    private:
        virtual void StartApplication(void);
        virtual void StopApplication(void);

        // void ScheduleTransmit(Time dt, uint16_t idx);
        // void Send(uint16_t idx);

        void SetTag(SDtag& tagToUse, uint8_t SourceID, uint8_t DestID, uint8_t currentHop = 1, uint32_t PktID = 0, uint8_t IsProbe = 0, uint8_t IsQueued = 0, uint8_t SandWichID = 10, uint8_t SandWichLargeID=0);

        void ScheduleBackground(Time dt, uint32_t idx);
        void SendBackground(uint32_t idx);
        uint32_t GMM_Pkt_Size(void);

        void ScheduleProbing(Time dt, uint32_t idx);
        void SendProbeNaive(uint32_t idx);
        void SendProbeSandWichV1(uint32_t idx, uint32_t idx_large);
        void OrchestraSandWichV1(void);

        void HandleRead(Ptr<Socket> socket);

        std::vector<uint32_t> m_count;
        Time probe_interval; // probe interval
        uint32_t m_size;
        bool is_run = true;

        // std::vector<uint32_t> m_sent;
        std::vector<Ptr<Socket>> tab_socket;
        Ptr<Socket> recv_socket;
        netw* meta;
        //std::vector<Address> tab_peerAddress;
        uint16_t m_peerPort;
        uint16_t ListenPort;
        std::vector<EventId> m_sendEvent; // background traffic
        std::vector<EventId> probe_event;
        uint16_t m_local_ID;
        uint32_t m_sandwich_sent = 0;
        std::unordered_map<uint32_t, uint32_t> map_neighbor_device;

        TracedCallback<Ptr<const Packet>> m_txTrace;

        TracedCallback<Ptr<const Packet>> m_rxTrace;

        TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_txTraceWithAddresses;

        TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses;
    };

} // namespace ns3

#endif