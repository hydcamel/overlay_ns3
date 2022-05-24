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

n_iter_demands = 1
n_iter_overlay = 1
overlay_generation_method = "LowDegree"
# overlay_generation_method = "stretch"
root_name = "/vagrant/Documents/vagrant_shared_folder/"
create_type='constant'

#idx_topology_list = [2]
delay_aware_log = np.array((3, len(idx_topology_list)))
delay_agonostic_log = np.array((3, len(idx_topology_list)))
congestion_log = np.array((2, len(idx_topology_list)))

idx_iter = 0
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
            os.system("../.././waf --run MinCostFixRate")
            delay_direct = py_utils.read_results(file_name="average_delay.txt", tunnel_list=tunnel_list)
            congestion_direct = py_utils.read_results(file_name="congestion_cnt.txt", tunnel_list=tunnel_list)

            route_aware_name = foldername+"route_table_" + overlay_generation_method+"_" + create_type+ "_" + "aware_"+ str(idx_overlay) + "_"+ str(idx_demand)+ ".txt"
            py_utils.write_setup("setup.txt", "graph_name " + graph_name, "name_overlay_nodes " + name_overlay_nodes, "name_demands " + name_demands, "route_name " + route_aware_name)
            os.system("../.././waf --run MinCostFixRate")
            delay_aware = py_utils.read_results(file_name="average_delay.txt", tunnel_list=tunnel_list)
            congestion_aware = py_utils.read_results(file_name="congestion_cnt.txt", tunnel_list=tunnel_list)
            delay_tmp = [delay_aware[k] / delay_direct[k] for k in range(len(tunnel_list))]
            delay_aware_log[0,idx_iter] = min(delay_tmp)
            delay_aware_log[2,idx_iter] = max(delay_tmp)
            delay_aware_log[1,idx_iter] = np.median(np.array(delay_tmp))

            
            route_agnostic_name = foldername+"route_table_" + overlay_generation_method+"_" + create_type+ "_" + "agnostic_"+ str(idx_overlay) + "_"+ str(idx_demand)+ ".txt"
            py_utils.write_setup("setup.txt", "graph_name " + graph_name, "name_overlay_nodes " + name_overlay_nodes, "name_demands " + name_demands, "route_name " + route_agnostic_name)
            os.system("../.././waf --run MinCostFixRate")
            delay_agonostic = py_utils.read_results(file_name="average_delay.txt", tunnel_list=tunnel_list)
            congestion_agonostic = py_utils.read_results(file_name="congestion_cnt.txt", tunnel_list=tunnel_list)
            delay_tmp = [delay_agonostic[k] / delay_direct[k] for k in range(len(tunnel_list))]
            delay_agonostic_log[0,idx_iter] = min(delay_tmp)
            delay_agonostic_log[2,idx_iter] = max(delay_tmp)
            delay_agonostic_log[1,idx_iter] = np.median(np.array(delay_tmp))
            idx_iter += 1
print("delay_aware_log, ", delay_aware_log)
print("delay_agonostic_log, ", delay_agonostic_log)