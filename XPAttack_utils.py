# -*- coding: utf-8 -*-
"""
Created on Tue Sep 13 12:43:05 2022

@author: robbe
"""
import numpy as np
import scipy.io as sio
import os


def adj2underlay(Adj, D, bw, delay, filename):
    n_nodes = len(Adj[0])
    n_edges = len(D[0])
    with open(filename, "w") as f:
        f.write("NODES " + str(n_nodes) + '\n')
        f.write('\n')
        f.write("EDGES " + str(n_edges*2) + '\n')
        for i in range(n_edges):
            s= np.where(D[:,i] == 1)[0][0]
            t = np.where(D[:,i] == -1)[0][0]
            f.write("edge_" + str(i*2) + ' ' + str(s) + ' ' + str(t) + ' ' + str(1) + ' ' + str(bw[i]) + ' ' + str(delay[i]) + '\n')
            f.write("edge_" + str(i*2+1) + ' ' + str(t) + ' ' + str(s) + ' ' + str(1) + ' ' + str(bw[i]) + ' ' + str(delay[i]) + '\n')
            
def SPR2routeTable(SPR, filename):
    with open(filename, "w") as f:
        for route in SPR:
            if len(route) == 0:
                continue
            s, t = route[0], route[-1]
            f.write(str(s) + ' ' + str(t) + ': ' + str(len(route)) + ':')
            for u in route:
                f.write(' ' + str(u))
            f.write('\n')

def wr_overlayNodes(overlay_node_list, filename):
    with open(filename, "w") as f:
        f.write(str(len(overlay_node_list)) + ': ' + ' '.join(str(u) for u in overlay_node_list))

def write_tunnel_demands(tunnel_demands:list, tunnel_list:list, filename="tunnel_demands.txt"):
    # Write into files
    # filename = "tunnel_demands.txt"
    with open(filename, "w") as fw:
        for i in range(len(tunnel_list)):
            fw.write( str(tunnel_list[i][0]) + ' ' + str(tunnel_list[i][1]) + ' ' + str(tunnel_demands[i]) + '\n' )

def wr_probe_setup(E_cur_idxlist: list, e_new_idx, probe_type:str, len_long_train, n_uePerGnb, n_probes, file_name = "probe_setup.txt"):
    with open(file_name, "w") as stf:
        stf.write("n_probes " + str(n_probes) + '\n')
        stf.write("E_cur_idxlist " + str(len(E_cur_idxlist)))
        for e in E_cur_idxlist:
            stf.write(" " + str(e))
        stf.write('\n' + "e_new_idx " + str(e_new_idx) + '\n' + "probe_type " + probe_type + '\n' + "len_long_train " + str(len_long_train) + '\n')
        stf.write("n_uePerGnb " + str(n_uePerGnb) + '\n')

def wr_probe_intervals(probe_intervals:list, tunnel_list:list, filename="probe_intervals.txt"):
    with open(filename, "w") as fw:
        for i in range(len(tunnel_list)):
            fw.write( str(tunnel_list[i][0]) + ' ' + str(tunnel_list[i][1]) + ' ' + str(probe_intervals[i]) + '\n' )
            
def wr_coordinate_gnb(node_xval, node_yval, filename):
    with open(filename, "w") as f:
        f.write( ' '.join(str(u) for u in node_xval) )
        f.write('\n')
        f.write( ' '.join(str(u) for u in node_yval) )
    
        
def write_setup(file_name, *para):
    with open(file_name, "w") as stf:
        for entry in para:
            stf.write(entry + '\n')
            
def run_simulation(para_from_matlab):
    # dir_name = "/export/home/Yudi_Huang/ns-3-dev/scratch/Category_inference/"
    dir_name = ""
    file_hyper_param = dir_name + "hyper_param.txt"
    E_cur_idxlist = para_from_matlab['E_cur_idxlist']
    '''Index Transformation'''
    E_cur_idxlist = [int(e-1) for e in E_cur_idxlist]
    
    '''Write to file'''
    str_old_E = "E_cur_idxlist " + str(len(E_cur_idxlist)) + " " + " ".join( [str(e) for e in E_cur_idxlist] )
    write_setup(file_hyper_param, str_old_E)
    os.system("../.././ns3 run Category_inference")
    
            
def init_setup(para_from_matlab):
    para_dict = sio.loadmat('para2py_hex3.mat', struct_as_record=False, squeeze_me=True)['para2py']
    Adj, D, SPR, sa, sb, ta, tb = para_dict.Adj, para_dict.D, para_dict.SPR, para_dict.sa, para_dict.sb, para_dict.ta, para_dict.tb
    node_xval, node_yval = para_dict.node_xval, para_dict.node_yval
    '''Index Transformation'''
    SPR = np.array( [route-1 for route in SPR if len(route) > 0] )
    sa, sb = sa - 1, sb - 1
    ta = np.array( [u for u in ta] )
    tb = tb - 1
    
    bw = [1000000]*len(D[0]) # bw in kbps, delay in microsec (us).
    delay = [20]*len(D[0]) # bw in kbps, delay in microsec (us).

    tunnel_list = [ (route[0], route[-1]) for route in SPR if len(route) > 0 ]
    n_tunnels = len(tunnel_list)
    tunnel_demands = [10] * n_tunnels
    probe_intervals = [10] * n_tunnels # microsec (us)

    dir_name = "/export/home/Yudi_Huang/ns-3-dev/scratch/Category_inference/"
    graph_name = dir_name + "Hex3.graph"
    name_overlay_nodes = dir_name + "overlay_node_Hex3.txt"
    name_demands = dir_name + "tunnel_demands.txt"
    route_name = dir_name + "route_table_Hex3.txt"
    probe_interval_files = dir_name + "probe_intervals.txt"
    probe_setup_name = dir_name + "probe_setup.txt"
    gnb_coordinate_file_name = dir_name + "coordinate_gnb.txt"
    hyper_param_filename = dir_name + "hyper_param.txt"
    
    adj2underlay(Adj=Adj, D=D, bw=bw, delay=delay, filename=graph_name)
    SPR2routeTable(SPR=SPR, filename=route_name)
    E_cur_idxlist, e_new_idx, n_calibrate_pkt, n_uePerGnb, n_probes = para_from_matlab['E_cur_idxlist'], para_from_matlab['e_new_idx'], para_from_matlab['n_calibrate_pkt'], para_from_matlab['n_uePerGnb'], para_from_matlab['n_probes']
    '''Index Transformation'''
    E_cur_idxlist = [int(e-1) for e in E_cur_idxlist]
    e_new_idx = int(e_new_idx - 1)
    n_calibrate_pkt = int(n_calibrate_pkt)
    n_uePerGnb, n_probes = int(n_uePerGnb), int(n_probes)
    
    wr_probe_setup(E_cur_idxlist=E_cur_idxlist, e_new_idx = e_new_idx, probe_type="naive", len_long_train=n_calibrate_pkt, n_uePerGnb=n_uePerGnb, n_probes=n_probes, file_name = probe_setup_name)
    wr_overlayNodes(overlay_node_list=[*range(len(Adj[0]))], filename=name_overlay_nodes)
    write_tunnel_demands(tunnel_demands=tunnel_demands, tunnel_list=tunnel_list, filename=name_demands)
    wr_probe_intervals(probe_intervals=probe_intervals, tunnel_list=tunnel_list, filename=probe_interval_files)
    wr_coordinate_gnb(node_xval=node_xval, node_yval=node_yval, filename=gnb_coordinate_file_name)
    write_setup("setup.txt", "graph_name " + graph_name, "name_overlay_nodes " + name_overlay_nodes, "name_demands " + name_demands, "route_name " + route_name, "probe_setup_filename " + probe_setup_name, "probe_interval_filename " + probe_interval_files, "gnb_coordinate_file " + gnb_coordinate_file_name, "hyper_param_filename " + hyper_param_filename)