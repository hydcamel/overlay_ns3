#ifndef OVERLAY_APPLICATION_H
#define OVERLAY_APPLICATION_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
//#include "SDtag.h"
#include <vector>
#include "utils.h"

namespace ns3
{

    class Socket;
    class Packet;

    class overlayApplication : public Application
    {
    public:
        static TypeId GetTypeId(void);

        overlayApplication();

        virtual ~overlayApplication();

        void SetRemote(Address ip, uint16_t idx);
        void AddRemote(Address ip);

        void SetDataSize(uint32_t dataSize);

        uint32_t GetDataSize(void) const;
        uint16_t GetPort(void) const;

    protected:
        virtual void DoDispose(void);

    private:
        virtual void StartApplication(void);
        virtual void StopApplication(void);

        void ScheduleTransmit(Time dt, uint16_t idx);
        void Send(uint16_t idx);

        void HandleRead(Ptr<Socket> socket);

        uint32_t m_count;
        Time m_interval;
        uint32_t m_size;

        uint32_t m_sent;
        std::vector<Ptr<Socket>> tab_socket;
        Ptr<Socket> recv_socket;
        std::vector<Address> tab_peerAddress;
        uint16_t m_peerPort;
        uint16_t ListenPort;
        EventId m_sendEvent;

        TracedCallback<Ptr<const Packet>> m_txTrace;

        TracedCallback<Ptr<const Packet>> m_rxTrace;

        TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_txTraceWithAddresses;

        TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses;
    };

} // namespace ns3

#endif