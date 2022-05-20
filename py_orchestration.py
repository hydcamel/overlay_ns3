import os
import py_utils
import pickle

'''Obtain topology file name'''
f_topology_name = "topology_name.pkl"
with open(f_topology_name, "rb") as f:
    all_topology_filename = pickle.load(f)

with open("selected_topologies.pkl", "rb") as f:
    idx_topology_list = pickle.load(f)

n_iter_demands = 1
n_iter_overlay = 1
overlay_generation_method = "LowDegree"
# overlay_generation_method = "stretch"
root_name = "C:/Users/yxh5389/Documents/vagrant_shared_folder/"
create_type='constant'

for idx_topo in idx_topology_list:
    graph_name = root_name+all_topology_filename[idx_topo]
    foldername = root_name+all_topology_filename[idx_topo].split('.')[0] + '/'
    print("idx_topo = ", idx_topo)
    for idx_overlay in range(n_iter_overlay):
        name_overlay_nodes = foldername+ "overlay_"+ overlay_generation_method+ "_"+ str(idx_overlay)+ ".txt"
        '''iter over demands'''
        for idx_demand in range(n_iter_demands):
            name_demands = foldername+"tunnel_demands_"+ overlay_generation_method+ "_" + create_type+  "_"+ str(idx_overlay) + "_"+ str(idx_demand)+ ".txt"
            '''Routing'''
            # Direct Tunnel:
            route_direct_name = foldername+"route_table_" + overlay_generation_method+"_" + create_type+ "_" + "direct_"+ str(idx_overlay) + "_"+ str(idx_demand)+ ".txt"
            py_utils.write_setup("setup.txt", "graph_name " + graph_name, "name_overlay_nodes " + name_overlay_nodes, "name_demands " + name_demands, "route_name " + route_direct_name)
            route_aware_name = foldername+"route_table_" + overlay_generation_method+"_" + create_type+ "_" + "aware_"+ str(idx_overlay) + "_"+ str(idx_demand)+ ".txt"
            py_utils.write_setup("setup.txt", "graph_name " + graph_name, "name_overlay_nodes " + name_overlay_nodes, "name_demands " + name_demands, "route_name " + route_aware_name)
            route_agnostic_name = foldername+"route_table_" + overlay_generation_method+"_" + create_type+ "_" + "agnostic_"+ str(idx_overlay) + "_"+ str(idx_demand)+ ".txt"
            py_utils.write_setup("setup.txt", "graph_name " + graph_name, "name_overlay_nodes " + name_overlay_nodes, "name_demands " + name_demands, "route_name " + route_agnostic_name)