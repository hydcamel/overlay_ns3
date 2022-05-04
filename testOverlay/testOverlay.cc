#include "ns3/wave-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "overlayApplication.h"
#include "utils.h"

//#include "wave-setup.h"
// token: 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
std::globalInfo meta;

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("testOverlay");

int main (int argc, char *argv[]){

    // Nodes creation
    uint32_t n_underlay = 4;
    uint32_t n_overlay = 3;

    meta.initDemand(n_overlay);

    NodeContainer underlayNodes;
    NodeContainer overlayNodes;

    underlayNodes.Create(n_underlay);
    overlayNodes.Create(n_overlay);


}