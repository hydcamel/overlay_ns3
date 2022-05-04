#ifndef UTILS_H
#define UTILS_H

#include <vector>

namespace std{

class globalInfo{
    public:
    // variables
        vector<vector<int>> matDemands;
    // member functions
        globalInfo();
        ~globalInfo();
        void initDemand(int n_overlay);
};

}

#endif