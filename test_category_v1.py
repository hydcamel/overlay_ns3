# -*- coding: utf-8 -*-
"""
Created on Mon Apr 25 13:06:58 2022

@author: robbe
"""

import numpy as np
import networkx as nx
import py_utils
import pickle
import os


'''Obtain topology file name'''
is_load_topology_name = True
f_topology_name = "topology_name.pkl"
if is_load_topology_name == False:
    import os
    base_folder = "./Data"
    for root, ds, fs in os.walk(base_folder):
        all_topology_filename = [None] * len(fs)
        idx = 0
        for f in fs:
            #filename = os.path.join(root, f)
            filename_split = f.split('.')
            if filename_split[-1] == 'graph':
                all_topology_filename[idx] = f
                idx += 1
    all_topology_filename = all_topology_filename[0:idx]
    with open(f_topology_name, "wb") as f:
        pickle.dump(all_topology_filename, f)
else:
    with open(f_topology_name, "rb") as f:
        all_topology_filename = pickle.load(f)

alpha = 2
is_creation_cost = False
overlay_generation_method = "stretch"
n_iter_demands = 1
idx_topology_list = [0, 1, 2, 18, 23, 39, 54, 127]
idx_topology_list = [2]

for idx_topo in idx_topology_list:
    D, edges_bw, edges_delay, edges_list, edges_dict = py_utils.create_incidence_from_graph( all_topology_filename[idx_topo] )
    py_utils.write_setup(all_topology_filename[idx_topo])

    # DG_under = nx.DiGraph()
    # DG_under.add_weighted_edges_from(edges_list)
    # nx.draw(DG_under, with_labels = True)

    spr_table = py_utils.routing_underlay_spr_delay(edges_list=edges_list)

    # Create mapping from overlay to underlay
    ratio_overlay_nodes = 0.5
    n_overlay = int( ratio_overlay_nodes * D.shape[0] )
    # idx_overlay_node = np.random.permutation(D.shape[0])[0:n_overlay]
    # idx_overlay_node = [5,12,18]
    # idx_overlay_node = py_utils.create_overlay_nodes(n_overlay_nodes=n_overlay, spr_table=spr_table, n_nodes=D.shape[0], generate_method=overlay_generation_method, DG_under = DG_under)
    idx_overlay_node = py_utils.create_overlay_nodes(n_overlay_nodes=n_overlay, spr_table=spr_table, n_nodes=D.shape[0], generate_method=overlay_generation_method)
    #idx_overlay_node = [10, 5, 12, 1, 9]

    py_utils.wr_overlay_nodes(idx_overlay_node)

    tunnel_list = py_utils.create_overlay_node_tunnel( idx_overlay_node )
    map_tunnel2edge, tunnel_capacity, tunnel_delay = py_utils.create_mapping(tunnel_list=tunnel_list, spr_table=spr_table, edges_dict = edges_dict, n_edges = D.shape[1], edges_bw = edges_bw, edges_delay=edges_delay )
    category_dict = py_utils.create_category_perfect_given_tunnels( map_tunnel2edge )
    tunnel_delays = py_utils.create_tunnel_delay(tunnel_list=tunnel_list, map_tunnel2edge=map_tunnel2edge, edge_delay=edges_delay)

    # delay_direct = np.zeros((3,n_iter_demands))
    # congestion_direct = np.zeros((3,n_iter_demands))
    # delay_aware = np.zeros((3,n_iter_demands))
    # congestion_aware = np.zeros((3,n_iter_demands))
    # delay_agonostic = np.zeros((3,n_iter_demands))
    # congestion_agonostic = np.zeros((3,n_iter_demands))
    delay_log = np.zeros((2,n_iter_demands))
    congestion_log = np.zeros((2,n_iter_demands))
    for it_demand in range(n_iter_demands):
        tunnel_demands = py_utils.create_tunnel_demands(create_type='constant', tunnel_capacity=tunnel_capacity, n_nodes=D.shape[0], idx_overlay_nodes=idx_overlay_node, tunnel_list=tunnel_list, demand_file=None)
        tunnel_demands = py_utils.scale_tunnel_demands(init_demands=tunnel_demands, map_tunnel2edge=map_tunnel2edge, edge_capacity=edges_bw, tunnel_list=tunnel_list, nodes_overlay=idx_overlay_node, tunnel_delays=tunnel_delays, alpha=alpha)
        py_utils.write_tunnel_demands(tunnel_demands=tunnel_demands, tunnel_list=tunnel_list)

        py_utils.convert_route_to_file(np.identity(len(tunnel_list)), tunnel_list, spr_table)
        os.system("../.././waf --run MinCostFixRate")
        delay_direct = py_utils.read_results(file_name="average_delay.txt", tunnel_list=tunnel_list)
        congestion_direct = py_utils.read_results(file_name="congestion_cnt.txt", tunnel_list=tunnel_list)
        print("congestion_direct: ", congestion_direct)


        res_x_aware = py_utils.aware_routing_MinCostFixedRate(routing_type='aware', tunnel_capacity=tunnel_capacity, tunnel_demands=tunnel_demands, category_dict=category_dict, nodes_overlay=idx_overlay_node, tunnel_list=tunnel_list, tunnel_delay=tunnel_delay, edge_capacity=edges_bw, is_creation_cost = is_creation_cost)
        py_utils.convert_route_to_file(res_x_aware, tunnel_list, spr_table)
        os.system("../.././waf --run MinCostFixRate")
        delay_aware = py_utils.read_results(file_name="average_delay.txt", tunnel_list=tunnel_list)
        congestion_aware = py_utils.read_results(file_name="congestion_cnt.txt", tunnel_list=tunnel_list)
        print("delay_aware: ", [delay_aware[k] / delay_direct[k] for k in range(len(tunnel_list))])
        print("congestion_aware: ", congestion_aware)

        res_x_agonostic = py_utils.aware_routing_MinCostFixedRate(routing_type='agnostic', tunnel_capacity=tunnel_capacity, tunnel_demands=tunnel_demands, category_dict=category_dict, nodes_overlay=idx_overlay_node, tunnel_list=tunnel_list, tunnel_delay=tunnel_delay, edge_capacity=edges_bw)
        py_utils.convert_route_to_file(res_x_agonostic, tunnel_list, spr_table)
        os.system("../.././waf --run MinCostFixRate")
        delay_agonostic = py_utils.read_results(file_name="average_delay.txt", tunnel_list=tunnel_list)
        congestion_agonostic = py_utils.read_results(file_name="congestion_cnt.txt", tunnel_list=tunnel_list)
        print("delay_agonostic: ", [delay_agonostic[k] / delay_direct[k] for k in range(len(tunnel_list))])
        print("congestion_agonostic: ", congestion_agonostic)

        # delay_per_run = np.zeros((2,len(tunnel_list)))
        # congestion_per_run = np.zeros((2,len(tunnel_list)))
        # for k in range(len(tunnel_list)):
        #     delay_per_run[0,k] = delay_aware[k] / delay_direct[k]
        #     delay_per_run[1,k] = delay_agonostic[k] / delay_direct[k]
        #     congestion_per_run[0,k] = congestion_aware[k] / congestion_direct[k]
        #     congestion_per_run[1,k] = congestion_agonostic[k] / congestion_direct[k]

        # res_x = py_utils.MinCostFixedRate_v2(routing_type='aware', tunnel_capacity=tunnel_capacity, tunnel_demands=tunnel_demands, category_dict=category_dict, nodes_overlay=idx_overlay_node, tunnel_list=tunnel_list, tunnel_delay=tunnel_delay, edge_capacity=edges_bw)

        '''Compare theoretical delays'''
        log_delay_theory = np.zeros( (len(tunnel_list), 3) ) # aware; agnostic; direct tunnel
        log_delay_theory[:,2] = tunnel_delays
        for i in range(len(tunnel_list)):
            log_delay_theory[i, 0] = np.sum(np.array( [res_x_aware[j,i] * tunnel_delays[j] for j in range(len(tunnel_list))] ))
            log_delay_theory[i, 1] = np.sum(np.array( [res_x_agonostic[j,i] * tunnel_delays[j] for j in range(len(tunnel_list))] ))
        print("log_delay_theory: ", log_delay_theory)
        '''Compare theoretical congestion'''
        traffic_direct = py_utils.traffic_theoretical(tunnel_demands=tunnel_demands, map_tunnel2edge=map_tunnel2edge, res_x=np.identity(len(tunnel_list)))
        traffic_aware = py_utils.traffic_theoretical(tunnel_demands=tunnel_demands, map_tunnel2edge=map_tunnel2edge, res_x=res_x_aware)
        traffic_agnositc = py_utils.traffic_theoretical(tunnel_demands=tunnel_demands, map_tunnel2edge=map_tunnel2edge, res_x=res_x_agonostic)
        log_congestion_theory = np.zeros( (D.shape[1], 3) ) # aware; agnostic; direct tunnel
        log_congestion_theory[:, 0] = traffic_aware - edges_bw
        log_congestion_theory[:, 1] = traffic_agnositc - edges_bw
        log_congestion_theory[:, 2] = traffic_direct - edges_bw
        print("log_congestion_theory: ", log_congestion_theory)