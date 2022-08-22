import os
import py_utils
import numpy as np
import pickle

'''Obtain topology file name'''
f_topology_name = "topology_name.pkl"
with open(f_topology_name, "rb") as f:
    all_topology_filename = pickle.load(f)

with open("selected_topologies.pkl", "rb") as f:
    idx_topology_list = pickle.load(f)

overlay_generation_method = "LowDegree"
# overlay_generation_method = "stretch"
root_name = "/export/home/Yudi_Huang/topology_data/"
create_type='constant'

idx_topology_list = [53] # comment out when iterating over all the selected topologies


n_iter_demands = 1
n_iter_overlay = 1

for idx_topo in idx_topology_list:
    graph_name = root_name+all_topology_filename[idx_topo]
    foldername = root_name+all_topology_filename[idx_topo].split('.')[0] + '/'
    for idx_overlay in range(n_iter_overlay):
        name_overlay_nodes = foldername+ "overlay_"+ overlay_generation_method+ "_"+ str(idx_overlay)+ ".txt"
        idx_overlay_node = py_utils.read_overlay_nodes_from_file(name_overlay_nodes)
        tunnel_list = py_utils.create_overlay_node_tunnel( idx_overlay_node )
        '''iter over demands'''
        for idx_demand in range(n_iter_demands):
            '''Routing'''
            # Proposed:
            name_demands = foldername+"tunnel_demands_"+ overlay_generation_method+ "_" + create_type+  "_"+ str(idx_overlay) + "_"+ str(idx_demand)+ ".txt"
            route_aware_name = foldername+"route_table_" + overlay_generation_method+"_" + create_type+ "_" + "aware_"+ str(idx_overlay) + "_"+ str(idx_demand)+ ".txt"
            py_utils.write_setup("setup.txt", "graph_name " + graph_name, "name_overlay_nodes " + name_overlay_nodes, "name_demands " + name_demands, "route_name " + route_aware_name)
            # os.system("../.././waf --run MinCostFixRate")
            # delay_aware = py_utils.read_results(file_name="average_delay.txt", tunnel_list=tunnel_list)