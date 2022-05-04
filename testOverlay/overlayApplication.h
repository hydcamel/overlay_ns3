#ifndef OVERLAY_APPLICATION_H
#define OVERLAY_APPLICATION_H

#include "ns3/application.h"

namespace ns3{

class overlayApplication: public ns3::Application{
    public:
        static TypeId GetTypeId (void);
        virtual TypeId GetInstanceTypeId (void) const;
        overlayApplication();
        ~overlayApplication();
        
        /**
         * @brief : two main functions for the overlay nodes. 
         * If this node is the sink, return true with no more actions
         * Otherwise, forward pkt to the next overlay hop
         * Both functions will be used as Callback
         */
        bool receivePkt (Ptr<NetDevice> device,Ptr<const Packet> packet,uint16_t protocol, const Address &sender);
        bool forwardOverlay();
    private:
        /**
         * @brief Inherited function to start up the application
         * 
         * 1) Register Callback;
         * 2) Setup flows for each target
         * 
         */
        void StartApplication();

};

}

#endif