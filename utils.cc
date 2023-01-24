#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "utils.h"
#include "SDtag.h"

namespace ns3
{

void read_setup(name_input_files &input_fd)
{
    char *cwd = get_current_dir_name();
    std::string pwd_tmp(cwd, cwd+strlen(cwd));
    std::cout << pwd_tmp << std::endl;
    // std::ifstream infile( pwd_tmp + "/scratch//Category_inference/setup.txt");
    // std::ifstream infile( pwd_tmp + "/scratch/CrossPathParam/setup.txt");
    std::ifstream infile( pwd_tmp + "/scratch/AttackTest/setup.txt");
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
        else if (temp.compare("gnb_coordinate_file") == 0)
        {
            iss >> input_fd.gnb_coordinate_files;
        }
        else if (temp.compare("hyper_param_filename") == 0)
        {
            iss >> input_fd.hyper_param_files;
        }
        else if (temp.compare("nUE_filename") == 0)
        {
            iss >> input_fd.nUE_filename;
        }
    }
}

void txTraceIpv4(std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ptr_ipv4, uint32_t dontknow){
    SDtag tagPktRecv;
    packet->PeekPacketTag(tagPktRecv);
    /* if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST && tagPktRecv.GetIsProbe() > 0)
    {
        std::cout << context << "\t" << Now() << ", PktID= " << tagPktRecv.GetPktID()  << ": src:" << (uint32_t)tagPktRecv.GetSourceID() << " dest:" << (uint32_t)tagPktRecv.GetDestID() << std::endl;
    } */
    if (tagPktRecv.GetIsProbe() > 0)
    {
        std::cout << context << "\t" << Now() << ", PktID= " << tagPktRecv.GetPktID()  << ": src:" << (uint32_t)tagPktRecv.GetSourceID() << " dest:" << (uint32_t)tagPktRecv.GetDestID() << std::endl;
    }
    // std::cout << context << "\t" << Now() << ": packet sent with size: " << packet->GetSize() << std::endl;
}
void p2pDevMacTx(std::string context, Ptr<const Packet> packet){
    SDtag tagPktRecv;
    packet->PeekPacketTag(tagPktRecv);
    /* if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST && tagPktRecv.GetIsProbe() > 0)
    {
        std::cout << context << "\t" << Now() << ", PktID= " << tagPktRecv.GetPktID()  << ": src:" << (uint32_t)tagPktRecv.GetSourceID() << " dest:" << (uint32_t)tagPktRecv.GetDestID() << std::endl;
    } */
    if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST &&tagPktRecv.GetIsProbe() > 0)
    {
        std::cout << context << "\t" << Now() << ", PktID= " << tagPktRecv.GetPktID()  << ": src:" << (uint32_t)tagPktRecv.GetSourceID() << " dest:" << (uint32_t)tagPktRecv.GetDestID() << std::endl;
    }
    /* if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST && tagPktRecv.GetIsProbe()>0 && tagPktRecv.GetSandWichLargeID() == 4)
    {
        std::cout << context << "\t" << Now() << ", PktID= " << tagPktRecv.GetPktID()  << ": src:" << (uint32_t)tagPktRecv.GetSourceID() << " dest:" << (uint32_t)tagPktRecv.GetDestID() << " sdID=" << (uint32_t)tagPktRecv.GetSandWichID() << std::endl;
    } */
    
    // std::cout << context << "\t" << Now() << ": packet sent from NetDev with size: " << packet->GetSize() << std::endl;
}
void p2pDevMacRx(std::string context, Ptr<const Packet> packet){
    SDtag tagPktRecv;
    packet->PeekPacketTag(tagPktRecv);
    /* if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST && tagPktRecv.GetIsProbe() > 0)
    {
        std::cout << context << "\t" << Now() << ", PktID= " << tagPktRecv.GetPktID()  << ": src:" << (uint32_t)tagPktRecv.GetSourceID() << " dest:" << (uint32_t)tagPktRecv.GetDestID() << std::endl;
    } */
    if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST && tagPktRecv.GetIsProbe() > 0)
    {
        std::cout << context << "\t" << Now() << ", PktID= " << tagPktRecv.GetPktID()  << ": src:" << (uint32_t)tagPktRecv.GetSourceID() << " dest:" << (uint32_t)tagPktRecv.GetDestID() << std::endl;
    }
    // std::cout << context << "\t" << Now() << ": packet received from NetDev with size:" << packet->GetSize() << std::endl;
}
void trace_PhyTxBegin(std::string context, Ptr<const Packet> packet){
    SDtag tagPktRecv;
    packet->PeekPacketTag(tagPktRecv);
    if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST && tagPktRecv.GetIsProbe() > 0)
    {
        std::cout << context << "\t" << Now() << ", PktID= " << tagPktRecv.GetPktID() << ": src:" << (uint32_t)tagPktRecv.GetSourceID() << " dest:" << (uint32_t)tagPktRecv.GetDestID() << std::endl;
    }
    // std::cout << context << "\t" << Now() << ": PHY sent begin with size: " << packet->GetSize() << std::endl;

    /* if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST && tagPktRecv.GetIsProbe()>0 && tagPktRecv.GetSandWichLargeID() == 4)
    {
        std::cout << context << "\t" << Now() << ", PktID= " << tagPktRecv.GetPktID()  << ": src:" << (uint32_t)tagPktRecv.GetSourceID() << " dest:" << (uint32_t)tagPktRecv.GetDestID() << " sdID=" << (uint32_t)tagPktRecv.GetSandWichID() << std::endl;
    } */
}
void trace_PhyTxEnd(std::string context, Ptr<const Packet> packet){
    SDtag tagPktRecv;
    packet->PeekPacketTag(tagPktRecv);
    if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST && tagPktRecv.GetIsProbe() > 0)
    {
        std::cout << context << "\t" << Now() << ", PktID= " << tagPktRecv.GetPktID()  << ": src:" << (uint32_t)tagPktRecv.GetSourceID() << " dest:" << (uint32_t)tagPktRecv.GetDestID() << std::endl;
    }
    // std::cout << context << "\t" << Now() << ": PHY sent end with size: " << packet->GetSize() << std::endl;
}
void trace_PhyRxEnd(std::string context, Ptr<const Packet> packet){
    SDtag tagPktRecv;
    packet->PeekPacketTag(tagPktRecv);
    if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST && tagPktRecv.GetIsProbe() > 0)
    {
        std::cout << context << "\t" << Now() << ", PktID= " << tagPktRecv.GetPktID()  << ": src:" << (uint32_t)tagPktRecv.GetSourceID() << " dest:" << (uint32_t)tagPktRecv.GetDestID() << std::endl;
    }
    // std::cout << context << "\t" << Now() << ": PHY received end with size: " << packet->GetSize() << std::endl;
}



}

