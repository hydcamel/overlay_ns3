#include "ns3/log.h"
#include "ns3/simulator.h"
#include "overlayApplication.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE("overlayApplication");
NS_OBJECT_ENSURE_REGISTERED(overlayApplication);

TypeId overlayApplication::GetTypeId(){
    static TypeId tid = TypeId("ns3::overlayApplication")
        .SetParent <Application> ()
        .AddConstructor<overlayApplication> ()
        /* .AddAttribute( "flowRate", "Flow rate to each sink",
            VectorValue(),
            MakeVectorAccessor(),
            MakeVectorChecker()
            ) */
        ;
    return tid;
}

TypeId overlayApplication::GetInstanceTypeId() const
{
    return overlayApplication::GetTypeId();
}

overlayApplication::overlayApplication(){

}

overlayApplication::~overlayApplication(){

}

void overlayApplication::StartApplication(){
    NS_LOG_FUNCTION (this);
    // Register callback for overlayApplication::receivePkt()
    Ptr<Node> nd = GetNode();
    Ptr<NetDevice> dev = nd->GetDevice(0);
    dev->SetReceiveCallback( MakeCallback (&overlayApplication::receivePkt, this) );

    // Setup flows 
}

bool overlayApplication::receivePkt(Ptr<NetDevice> device,Ptr<const Packet> packet,uint16_t protocol, const Address &sender){
    return true;
}

}