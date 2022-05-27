#ifndef SD_TAG_H
#define SD_TAG_H

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
    void SetCurrentHop (uint8_t value);
    void AddCurrentHop (void);
    uint8_t GetCurrentHop (void) const;
    uint8_t GetIsProbe (void) const;
    void SetIsProbe (uint8_t value);
    void SetPktID (uint32_t value);
    uint32_t GetPktID (void) const;
private:
    uint8_t SourceID;
    uint8_t DestID;
    uint8_t currentHop;
    uint8_t IsProbe;
    uint32_t PktID;
};

}

#endif