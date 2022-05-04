#include "utils.h"

namespace std
{

    globalInfo::globalInfo(){
    }

    globalInfo::~globalInfo(){
    }

    void globalInfo::initDemand(int n_overlay){
        matDemands.resize(n_overlay);
        for (int i = 0; i < n_overlay; i++) {
            matDemands[i].resize( n_overlay );
        }
    }

}