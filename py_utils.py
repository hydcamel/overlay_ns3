# -*- coding: utf-8 -*-
"""
Created on Sun Apr 24 11:39:41 2022

@author: robbe
"""
from matplotlib import lines
import numpy as np
import random
import networkx as nx
import gurobipy as gp
from gurobipy import GRB
import numpy.matlib as npm


'''Create Functions'''
def create_incidence_from_graph(file_name:str):
    file_to_read = '/vagrant/Documents/vagrant_shared_folder/' + file_name
    graphFile = open(file_to_read, 'r')
    Lines = graphFile.readlines()
    # Read number of nodes and number of edges
    n_node = int( Lines[0].strip('\n').split(' ')[1] )
    n_edges = int( Lines[n_node+3].strip('\n').split(' ')[1] )

    # Read edges to form incidence matrix
    D = np.zeros((n_node, n_edges), dtype=int)
    edges_bw = np.zeros((n_edges,))
    edges_delay = np.zeros((n_edges,))
    edges_list = []
    edges_dict = {}
    for i in range(n_edges):
        line = Lines[n_node+5+i]
        tmp = line.strip('\n').strip('edge_').split(' ')
        idx, src, dest, weight, bw, delay = int(tmp[0]), int(tmp[1]), int(tmp[2]), int(tmp[3]), int(tmp[4]), int(tmp[5])
        D[src][i], D[dest][i] = 1, -1
        edges_bw[i], edges_delay[i] = bw*1e3, float(delay) / 1e6 # dataset: bw in kbps, delay in microsec (us).
        #edges_list.append((src, dest, delay))
        edges_list.append((src, dest, 1))
        edges_dict[(src, dest)] = i
        # print("idx = ", idx, " src = ", src, " dest = ", dest, " w = ", weight, " bw = ", bw, " d = ", delay)
    return D, edges_bw, edges_delay, edges_list, edges_dict

def create_overlay_nodes(n_overlay_nodes, spr_table, n_nodes, generate_method="stretch", DG_under = None):
    spr_mat = create_spr_mat(spr_table=spr_table, n_node=n_nodes)
    if generate_method == "stretch":
        idx_overlay_node = [None] * n_overlay_nodes
        idx_overlay_node[0] = random.randint(0, n_nodes-1)
        for i in range(1,n_overlay_nodes):
            min_id, min_val = 0, 0
            tmp_it = 0
            for j in range(n_nodes):
                tmp_it = np.amin( spr_mat[idx_overlay_node[0:i], j] )
                if tmp_it > min_val:
                    min_id, min_val = j, tmp_it
            idx_overlay_node[i] = min_id
    elif generate_method == "LowDegree":
        degree_list_sorted = sorted( list(DG_under.degree(range(n_nodes))), key=lambda tup: tup[1] )
        idx_overlay_node = [degree_list_sorted[i][0] for i in range(n_overlay_nodes)]
    return idx_overlay_node

def create_tunnel_delay(tunnel_list:list, map_tunnel2edge, edge_delay):
    tunnel_delays = [None] * len(tunnel_list)
    for i in range(len(tunnel_list)):
        tunnel_delays[i] = np.sum( edge_delay[np.nonzero(map_tunnel2edge[:, i])[0].tolist()] )
    return tunnel_delays
    
def routing_underlay_spr_delay(edges_list):
    DG_under = nx.DiGraph()
    DG_under.add_weighted_edges_from(edges_list)
    spr_all = dict(nx.all_pairs_shortest_path(DG_under))
    return spr_all

def create_spr_mat(spr_table, n_node):
    spr_mat = np.zeros((n_node, n_node))
    for u in range(n_node):
        for v in range(n_node):
            spr_mat[u][v] = len(spr_table[u][v]) - 1
    return spr_mat

def create_topology_list(all_topology_filename:list, n_topology):
    density_list = [None] * len(all_topology_filename)
    idx_topology_list = [None] * n_topology
    for idx_topo in range(len(all_topology_filename)):
        D, *_ = create_incidence_from_graph( all_topology_filename[idx_topo] )
        density_list[idx_topo] = float(D.shape[1]) / float(D.shape[0] * (D.shape[0]-1))
    sort_idx = np.argsort( np.array(density_list) )
    len_win = int( len(all_topology_filename) / n_topology )
    for i in range(n_topology):
        idx_topology_list[i] = sort_idx[ int(np.floor(np.random.uniform(low=i*len_win, high=(i+1)*len_win, size=1)[0])) ]
    return idx_topology_list
    

def create_overlay_node_tunnel(loc_nodes):
    # create overlay tunnels
    edges_list = [(i,j) for i in loc_nodes for j in loc_nodes if i != j]
    return edges_list

def create_mapping(tunnel_list, spr_table, edges_dict, n_edges, edges_bw, edges_delay):
    map_mat = np.zeros((n_edges, len(tunnel_list))) # underlay * overlay
    tunnel_capacity = [1e20] * len(tunnel_list) # overlay
    tunnel_delay = [None] * len(tunnel_list) # overlay
    idx_tunnel = 0
    for (i,j) in tunnel_list:
        path = spr_table[i][j]
        delay_per = 0
        for k in range(len(path)-1):
            idx_edge = edges_dict[(path[k], path[k+1])]
            map_mat[idx_edge][idx_tunnel] = 1
            if tunnel_capacity[idx_tunnel] > edges_bw[idx_edge]:
                tunnel_capacity[idx_tunnel] = edges_bw[idx_edge]
            delay_per += edges_delay[idx_edge]
        tunnel_delay[idx_tunnel] = delay_per
        idx_tunnel += 1
    return map_mat, tunnel_capacity, tunnel_delay

def create_category_perfect_given_tunnels(map_tunnel2edge):
    category_dict = {} # key = list of tunnels; val = edges in the category
    n_edges = map_tunnel2edge.shape[0]
    for i in range(n_edges):
        tunnels_per_edge = np.nonzero( map_tunnel2edge[i,:] )[0].tolist()
        if len(tunnels_per_edge) == 0:
            continue
        key_str = str(tunnels_per_edge)
        if key_str in category_dict:
            category_dict[key_str].append(i)
        else:
            category_dict[key_str] = [i]
    return category_dict

def create_tunnel_demands(create_type:str, tunnel_capacity:list, n_nodes, idx_overlay_nodes, tunnel_list:list, demand_file=None):
    n_tunnel = len(tunnel_capacity)
    lb, ub = 0.5, 0.8
    tunnel_demands = np.zeros((n_tunnel,))
    if create_type == 'random':
        for i in range(n_tunnel):
            tunnel_demands[i] = round( np.random.uniform(lb*tunnel_capacity[i], ub*tunnel_capacity[i], 1)[0], 1 ) 
    elif create_type == 'constant':
        tunnel_demands = np.array([ub*tunnel_capacity[k] for k in range(n_tunnel)])
    elif create_type == 'read':
        tunnel_demands = read_demands_file(demand_file)
    elif create_type == 'gravity':
        tunnel_demands = gravity_model_demands(n_nodes=n_nodes, idx_overlay_nodes=idx_overlay_nodes, tunnel_list=tunnel_list)
    return tunnel_demands.tolist()


'''Optimization: Routing'''
def aware_routing_MinCostFixedRate(routing_type:str, tunnel_capacity:list, tunnel_demands:list, category_dict:dict, nodes_overlay:list, tunnel_list:list, tunnel_delay:list, edge_capacity:list, is_creation_cost = False):
    min_cost_model = gp.Model()
    # min_cost_model.setParam("OutputFlag", 0)
    n_tunnel = len(tunnel_capacity)
    xijh = min_cost_model.addVars(n_tunnel, n_tunnel, vtype = GRB.BINARY, name = 'x_bin') # tunnels * demands
    '''Capacity Constraint'''
    if routing_type == 'aware':
        for tunnel_str, edges in category_dict.items(): # iterate over categories
            tunnels = decode_tunnel_str(tunnel_str)
            if not tunnels:
                continue
            Ce = min( [edge_capacity[i] for i in edges] )
            min_cost_model.addLConstr( gp.quicksum([tunnel_demands[i] * xijh[j,i] for i in range(n_tunnel) for j in tunnels]) <= Ce, name = 'capacity' )
    elif routing_type == 'agnostic':
        for i in range(n_tunnel):
            min_cost_model.addLConstr( gp.quicksum( [tunnel_demands[j] * xijh[i,j] for j in range(n_tunnel)] ) <= tunnel_capacity[i] )
    '''Flow Conservation Constraint'''        
    for i in range(n_tunnel): #iterate over demands
        (src, dest) = tunnel_list[i]
        for node in nodes_overlay: # iterate over overlay nodes
            tunnel_out = [tunnel_list.index((node, j)) for j in nodes_overlay if node != j]
            tunnel_in = [tunnel_list.index((j, node)) for j in nodes_overlay if node != j]
            lhs_list = [xijh[j,i] for j in tunnel_out]
            rhs_list = [xijh[j,i] for j in tunnel_in]
            min_cost_model.addLConstr( gp.quicksum(lhs_list) == gp.quicksum(rhs_list) + 1*(src==node) - 1*(dest==node), name = 'flowConservation'  )
    '''transmission cost'''
    flow_all_tunnel = gp.quicksum( [tunnel_delay[j] * tunnel_demands[i] * xijh[j,i] for i in range(n_tunnel) for j in range(n_tunnel)] )
    '''Creation Cost Constraint'''
    if is_creation_cost:
        yij = min_cost_model.addVars(n_tunnel, vtype = GRB.BINARY, name = 'y_bin') # tunnels
        for i in range(n_tunnel):
            rhs_list = [xijh[i,j] for j in range(n_tunnel)]
            min_cost_model.addLConstr( yij[i] >= 1/n_tunnel * gp.quicksum( rhs_list ) )
        cost_creation = 5
        min_cost_model.setObjective( cost_creation * gp.quicksum( [yij[k] for k in range(n_tunnel)] ) + flow_all_tunnel, GRB.MINIMIZE )
        min_cost_model.optimize()
        res_x = np.zeros((n_tunnel, n_tunnel))
        res_y = np.zeros((n_tunnel, ))
        for i in range(n_tunnel):
            res_y[i] = 1*(yij[i].X > 0.8)
            for j in range(n_tunnel):
                res_x[i,j] = 1*(xijh[i,j].X > 0.8)
        return res_x, res_y
    else:
        # min_cost_model.write("debug_aware.lp")
        min_cost_model.setObjective( flow_all_tunnel, GRB.MINIMIZE )
        min_cost_model.optimize()
        res_x = np.zeros((n_tunnel, n_tunnel))
        for i in range(n_tunnel):
            for j in range(n_tunnel):
                res_x[i,j] = 1*(xijh[i,j].X > 0.8)
        return res_x

def MinCostFixedRate_v2(routing_type:str, tunnel_capacity:list, tunnel_demands:list, category_dict:dict, nodes_overlay:list, tunnel_list:list, tunnel_delay:list, edge_capacity:list):
    min_cost_model = gp.Model()
    M_oe = 1e6
    n_tunnel = len(tunnel_capacity)
    xijh = min_cost_model.addVars(n_tunnel, n_tunnel, vtype = GRB.BINARY, name = 'x_bin') # tunnels * demands
    Oe = min_cost_model.addVars(len(category_dict), lb=0, name = 'Oe') #
    '''Capacity Constraint'''
    if routing_type == 'aware':
        idx = 0
        for tunnel_str, edges in category_dict.items(): # iterate over categories
            tunnels = decode_tunnel_str(tunnel_str)
            if not tunnels:
                continue
            Ce = min( [edge_capacity[i] for i in edges] )
            min_cost_model.addLConstr( gp.quicksum([tunnel_demands[i] * xijh[j,i] for i in range(n_tunnel) for j in tunnels]) <= Ce + Oe[idx], name = 'capacity' )
            idx += 1
    elif routing_type == 'agnostic':
        for i in range(n_tunnel):
            min_cost_model.addLConstr( gp.quicksum( [tunnel_demands[j] * xijh[i,j] for j in range(n_tunnel)] ) <= tunnel_capacity[i] )
    '''Flow Conservation Constraint'''        
    for i in range(n_tunnel): #iterate over demands
        (src, dest) = tunnel_list[i]
        for node in nodes_overlay: # iterate over overlay nodes
            tunnel_out = [tunnel_list.index((node, j)) for j in nodes_overlay if node != j]
            tunnel_in = [tunnel_list.index((j, node)) for j in nodes_overlay if node != j]
            lhs_list = [xijh[j,i] for j in tunnel_out]
            rhs_list = [xijh[j,i] for j in tunnel_in]
            min_cost_model.addLConstr( gp.quicksum(lhs_list) == gp.quicksum(rhs_list) + 1*(src==node) - 1*(dest==node) )
    flow_all_tunnel = gp.quicksum( [tunnel_delay[j] * tunnel_demands[i] * xijh[j,i] for i in range(n_tunnel) for j in range(n_tunnel)] )
    min_cost_model.setObjective( flow_all_tunnel + M_oe * gp.quicksum(Oe), GRB.MINIMIZE )
    min_cost_model.optimize()
    res_x = np.zeros((n_tunnel, n_tunnel))
    for i in range(n_tunnel):
        for j in range(n_tunnel):
            res_x[i,j] = 1*(xijh[i,j].X > 0.8)
    return res_x



'''Demands Model'''
def scale_tunnel_demands(init_demands:list, map_tunnel2edge, edge_capacity:list, tunnel_list:list, nodes_overlay:list, tunnel_delays:list, alpha):
    gp_model = gp.Model()
    gp_model.setParam("OutputFlag", 0)
    n_tunnel = len(init_demands)
    xijh = gp_model.addVars(n_tunnel, n_tunnel, vtype = GRB.BINARY, name = 'x_bin') # tunnels * demands
    omega = gp_model.addVars(1, lb = 0, name = 'omega')
    '''Capacity Constraint'''
    for i in range(len(edge_capacity)):
        tunnels_per_edge = np.nonzero( map_tunnel2edge[i,:] )[0].tolist()
        tmp = [init_demands[j]*xijh[k, j] for k in range(len(tunnel_list)) if k in tunnels_per_edge for j in range(len(init_demands))]
        gp_model.addLConstr( gp.quicksum(tmp) <= omega[0] * edge_capacity[i])
    '''Flow Conservation Constraint'''        
    for i in range(n_tunnel): #iterate over demands
        (src, dest) = tunnel_list[i]
        for node in nodes_overlay: # iterate over overlay nodes
            tunnel_out = [tunnel_list.index((node, j)) for j in nodes_overlay if node != j]
            tunnel_in = [tunnel_list.index((j, node)) for j in nodes_overlay if node != j]
            lhs_list = [xijh[j,i] for j in tunnel_out]
            rhs_list = [xijh[j,i] for j in tunnel_in]
            gp_model.addLConstr( gp.quicksum(lhs_list) == gp.quicksum(rhs_list) + 1*(src==node) - 1*(dest==node) )
        '''For each demand, we constrain the quality'''
        # gp_model.addLConstr( gp.quicksum([tunnel_delays[k] * xijh[k,i] for k in range(n_tunnel)]) <= alpha * tunnel_delays[i] )
    gp_model.setObjective( omega[0], GRB.MINIMIZE )
    gp_model.optimize()
    return [iter_demand / omega[0].X for iter_demand in init_demands]

def gravity_model_demands(n_nodes, idx_overlay_nodes:list, tunnel_list:list):
    in_traffic, out_traffic = np.zeros((n_nodes,)), np.zeros((n_nodes,))
    in_traffic[idx_overlay_nodes]  = np.random.exponential(scale=1.0, size=len(idx_overlay_nodes))
    out_traffic[idx_overlay_nodes] = np.random.exponential(scale=1.0, size=len(idx_overlay_nodes))
    sum_in, sum_out = np.sum(in_traffic), np.sum(out_traffic)
    sum_product = sum_in * sum_out
    total_Mbps = sum_in + sum_out
    overlay_demands = np.zeros(len(tunnel_list))
    for i in range(len(tunnel_list)):
        src, dest = tunnel_list[i][0], tunnel_list[i][1]
        overlay_demands[i] = in_traffic[src] * out_traffic[dest] / sum_product * total_Mbps
    return overlay_demands

'''Write Functions'''
def convert_route_to_file(res_x, tunnel_list, spr_table, file_route_name = "route_table.txt"):
    # tunnels * demands
    # file_route_name = "route_table.txt"
    with open(file_route_name, "w") as file_route:
        for i in range(len(tunnel_list)): # iterate over all SD pairs (demands)
            src, dest = tunnel_list[i][0], tunnel_list[i][1]
            list_tunnel_route = np.nonzero(res_x[:,i]) # used tunnels
            origin = src
            write_odpair = str(src)+ ' ' + str(dest) + ': '
            write_route = str(src)
            list_underlay_route = [src] # a list to store underlay routing path node, which can be used to calculate the size!
            # file_route.write(str(src)+ ' ' + str(dest) + ': ')
            # file_route.write(str(src))
            while origin != dest: # iterate over each tunnel for routing this pair
                for tunnel in [tunnel_list[k] for k in list_tunnel_route[0].tolist()]: # find the used tunnel whose origin is correct
                    if (origin == tunnel[0]):
                        underlay_route = spr_table[origin][tunnel[1]] # underlay path for this specific tunnel within the overlay routing
                        for j in range(1, len(underlay_route)): # write all underlay path node
                            write_route += ' ' + str(underlay_route[j])
                            list_underlay_route.append(underlay_route[j])
                            # file_route.write(' ' + str(underlay_route[j]))
                        origin = underlay_route[-1]
                        break
            file_route.write(write_odpair + str(len(list_underlay_route)) + ': ' + write_route)
            file_route.write('\n')

def write_tunnel_demands(tunnel_demands:list, tunnel_list:list, filename="tunnel_demands.txt"):
    # Write into files
    # filename = "tunnel_demands.txt"
    with open(filename, "w") as fw:
        for i in range(len(tunnel_list)):
            fw.write( str(tunnel_list[i][0]) + ' ' + str(tunnel_list[i][1]) + ' ' + str(tunnel_demands[i]) + '\n' )

def wr_overlay_nodes(loc_overlay_nodes:list, file_name = "overlay.txt"):
    # file_name = "overlay.txt"
    str_wr = str(len(loc_overlay_nodes)) + ': ' + ' '.join(str(idx) for idx in loc_overlay_nodes)
    with open(file_name, "w") as f:
        f.write( str_wr )
        
def write_setup(file_name, *para):
    with open(file_name, "w") as stf:
        for entry in para:
            stf.write(entry + '\n')
        

'''Read Functions'''
def read_demands_file(demand_file:str):
    file_to_read = './Data/' + demand_file
    demandFile = open(file_to_read, 'r')
    Lines = demandFile.readlines()
    
def read_results(file_name, tunnel_list:list):
    res_file = open(file_name, 'r')
    Lines = res_file.readlines()
    res = [None] * len(tunnel_list)
    for line in Lines:
        tmp = line.split(' ')
        src, dest, val = int(tmp[0]), int(tmp[1]), float(tmp[2])
        idx = tunnel_list.index((src, dest))
        res[idx] = val
    return res

    

'''Miscellaneous'''
def decode_tunnel_str(tunnel_str):
    tmp = tunnel_str.strip('[').strip(']').split(', ')
    tunnel_list = [int(str_to_decode) for str_to_decode in tmp]
    # tunnel_list = []
    # for str_to_decode in tmp:
    #     tunnel_list = [tunnel_list, int(str_to_decode)]
    return tunnel_list

def traffic_theoretical(tunnel_demands, map_tunnel2edge, res_x):
    # res_x: tunnels * demands
    tunnel_traffic = np.sum( res_x * npm.repmat( np.array(tunnel_demands), len(tunnel_demands), 1 ), axis=1 )
    edge_traffic = np.sum( map_tunnel2edge * npm.repmat(tunnel_traffic, map_tunnel2edge.shape[0], 1), axis=1 )
    return edge_traffic

def mkdir(foldername):
    if os.path.exists(foldername): # folder exists
        pass
    else:
        os.makedirs(foldername)
    return foldername + '/'
