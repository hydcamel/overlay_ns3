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

class Socket;
class Packet;

class overlayApplication : public Application
{
public:
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;

    /** Init **/
    overlayApplication();
    virtual ~overlayApplication();
    void InitApp(netw* meta, uint32_t localId, uint32_t MaxPktSize);
    void SetLocalID(uint32_t localID);
    uint32_t GetLocalID(void) const;

    /** Connection **/
    void SetSocket(Address ip, uint32_t idx, uint32_t deviceID);
    void SetRecvSocket(void);
    
    /** Functions **/
    void HandleRead(Ptr<Socket> socket);

    bool is_overlay;
protected:
    virtual void DoDispose(void);
private:
    /** probing **/
    Time probe_interval; // probe interval
    Time sandwich_interval; // Interval for the sandwich probing -- microsecond (us, 1e-6) 
    std::vector<EventId> m_sendEvent; // background traffic
    std::vector<EventId> probe_event;
    /** connection **/
    uint16_t m_peerPort;
    uint16_t ListenPort;
    std::vector<Ptr<Socket>> tab_socket;
    Ptr<Socket> recv_socket;
    std::unordered_map<uint32_t, uint32_t> map_neighbor_device; // <idx_neighbor, deviceID>
    /** Basic Meta **/
    netw* meta;
    bool is_run = true;
    uint16_t m_local_ID;
    
};


}

#endif