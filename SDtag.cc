#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "SDtag.h"
namespace ns3
{


NS_LOG_COMPONENT_DEFINE("SDtag");
NS_OBJECT_ENSURE_REGISTERED(SDtag);

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
    return 3;
}
void SDtag::Serialize (TagBuffer i) const
{
    i.WriteU8 (SourceID);
    i.WriteU8 (DestID);
    i.WriteU8(currentHop);
}
void SDtag::Deserialize (TagBuffer i)
{
    SourceID = i.ReadU8 ();
    DestID = i.ReadU8 ();
    currentHop = i.ReadU8();
}
void SDtag::Print (std::ostream &os) const
{
    os << "source=" << (uint32_t)SourceID << ", Dest=" << (uint32_t)DestID << ", Cur_hop=" << (uint32_t)currentHop << std::endl;
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
void SDtag::SetCurrentHop (uint8_t value)
{
    currentHop = value;
}
void SDtag::AddCurrentHop (void)
{
    currentHop += 1;
}
uint8_t SDtag::GetCurrentHop (void) const
{
    return currentHop;
}



}