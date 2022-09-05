#ifndef UEAPP_H
#define UEAPP_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-socket.h"
#include "SDtag.h"

namespace ns3
{

class ueApp : public Application
{
public:
    ueApp(uint32_t ID_associated);
    virtual ~ueApp();

    /** Functions **/
    void HandleRead(Ptr<Socket> socket);
private:
    uint32_t local_ID_;
    virtual void StartApplication(void);
    virtual void StopApplication(void);
};

}

#endif