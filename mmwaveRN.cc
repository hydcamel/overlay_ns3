#include "mmwaveNR.h"

namespace ns3
{

myNR::myNR()
{
    nrHelper = CreateObject<NrHelper> ();
}
myNR::~myNR()
{
    nrHelper = nullptr;
}

} // namespace ns3