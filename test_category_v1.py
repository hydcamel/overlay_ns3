# -*- coding: utf-8 -*-
"""
Created on Mon Apr 25 13:06:58 2022

@author: robbe
"""

import numpy as np
import networkx as nx
import py_utils
import pickle


'''Obtain topology file name'''
is_load_topology_name = True
f_topology_name = "./Data/topology_name.pkl"
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
n_iter_demands = 1
idx_topology_list = [0, 1, 2, 18, 23, 39, 54, 127]

for idx_topo in idx_topology_list:
    D, edges_bw, edges_delay, edges_list, edges_dict = py_utils.create_incidence_from_graph( all_topology_filename[idx_topo] )

    # DG_under = nx.DiGraph()
    # DG_under.add_weighted_edges_from(edges_list)
    # nx.draw(DG_under, with_labels = True)

    spr_table = py_utils.routing_underlay_spr_delay(edges_list=edges_list)

    # Create mapping from overlay to underlay
    ratio_overlay_nodes = 0.3
    n_overlay = int( ratio_overlay_nodes * D.shape[0] )
    # idx_overlay_node = np.random.permutation(D.shape[0])[0:n_overlay]
    # idx_overlay_node = [5,12,18]
    idx_overlay_node = py_utils.create_overlay_nodes(n_overlay_nodes=n_overlay, spr_table=spr_table, n_nodes=D.shape[0])
    idx_overlay_node = [10, 5, 12, 1, 9]

    py_utils.wr_overlay_nodes(idx_overlay_node)

    tunnel_list = py_utils.create_overlay_node_tunnel( idx_overlay_node )
    map_tunnel2edge, tunnel_capacity, tunnel_delay = py_utils.create_mapping(tunnel_list=tunnel_list, spr_table=spr_table, edges_dict = edges_dict, n_edges = D.shape[1], edges_bw = edges_bw, edges_delay=edges_delay )
    category_dict = py_utils.category_perfect_given_tunnels( map_tunnel2edge )
    tunnel_delays = py_utils.create_tunnel_delay(tunnel_list=tunnel_list, map_tunnel2edge=map_tunnel2edge, edge_delay=edges_delay)

    delay_direct = np.zeros((3,n_iter_demands))
    for it_demand in range(n_iter_demands):
        tunnel_demands = py_utils.create_tunnel_demands(create_type='gravity', tunnel_capacity=tunnel_capacity, n_nodes=D.shape[0], idx_overlay_nodes=idx_overlay_node, tunnel_list=tunnel_list, demand_file=None)
        tunnel_demands = py_utils.scale_tunnel_demands(init_demands=tunnel_demands, map_tunnel2edge=map_tunnel2edge, edge_capacity=edges_bw, tunnel_list=tunnel_list, nodes_overlay=idx_overlay_node, tunnel_delays=tunnel_delays, alpha=alpha)

        with open("debug_demands.pkl", "rb") as f:
            tunnel_demands = pickle.load(f)
        
        py_utils.convert_route_to_file(np.identity(len(tunnel_list)), tunnel_list, spr_table)
        
            
        res_x_aware = py_utils.aware_routing_MinCostFixedRate(routing_type='aware', tunnel_capacity=tunnel_capacity, tunnel_demands=tunnel_demands, category_dict=category_dict, nodes_overlay=idx_overlay_node, tunnel_list=tunnel_list, tunnel_delay=tunnel_delay, edge_capacity=edges_bw, is_creation_cost = is_creation_cost)

        res_x_agonostic = py_utils.aware_routing_MinCostFixedRate(routing_type='agnostic', tunnel_capacity=tunnel_capacity, tunnel_demands=tunnel_demands, category_dict=category_dict, nodes_overlay=idx_overlay_node, tunnel_list=tunnel_list, tunnel_delay=tunnel_delay, edge_capacity=edges_bw)

        # res_x = py_utils.MinCostFixedRate_v2(routing_type='aware', tunnel_capacity=tunnel_capacity, tunnel_demands=tunnel_demands, category_dict=category_dict, nodes_overlay=idx_overlay_node, tunnel_list=tunnel_list, tunnel_delay=tunnel_delay, edge_capacity=edges_bw)

        '''Compare theoretical delays'''
        log_delay_theory = np.zeros( (len(tunnel_list), 3) ) # aware; agnostic; direct tunnel
        log_delay_theory[:,2] = tunnel_delays
        for i in range(len(tunnel_list)):
            log_delay_theory[i, 0] = np.sum(np.array( [res_x_aware[j,i] * tunnel_delays[j] for j in range(len(tunnel_list))] ))
            log_delay_theory[i, 1] = np.sum(np.array( [res_x_agonostic[j,i] * tunnel_delays[j] for j in range(len(tunnel_list))] ))

        py_utils.convert_route_to_file(res_x_aware, tunnel_list, spr_table)