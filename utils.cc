#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "utils.h"


void read_setup(name_input_files &input_fd)
{
    char *cwd = get_current_dir_name();
    std::string pwd_tmp(cwd, cwd+strlen(cwd));
    std::cout << pwd_tmp << std::endl;
    std::ifstream infile( pwd_tmp + "/scratch//Category_inference/setup.txt");
    // std::ifstream infile("/export/home/Yudi_Huang/ns-allinone-3.36.1/ns-3.36.1/scratch/CategoryQueue/setup.txt");
    std::string line;
    std::string temp;

    while (getline(infile, line))
    {
        std::istringstream iss(line);
        iss >> temp ;
        if (temp.compare("graph_name") == 0)
        {
            iss >> input_fd.netw_filename;
        }
        else if (temp.compare("name_overlay_nodes") == 0)
        {
            iss >> input_fd.file_overlay_nodes;
        }
        else if (temp.compare("name_demands") == 0)
        {
            iss >> input_fd.demands_file;
        }
        else if (temp.compare("route_name") == 0)
        {
            iss >> input_fd.route_name;
        }
        else if (temp.compare("probe_setup_filename") == 0)
        {
            iss >> input_fd.probe_setup_filename;
        }
        else if (temp.compare("probe_interval_filename") == 0)
        {
            iss >> input_fd.probe_interval_files;
        }
    }
}