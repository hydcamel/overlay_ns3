# -*- coding: utf-8 -*-
"""
Created on Tue Sep 13 12:43:05 2022

@author: robbe
"""
import numpy as np
import scipy.io as sio
import os
from math import floor


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

def wr_probe_setup(E_cur_idxlist: list, e_new_idx, probe_type:str, len_long_train, n_uePerGnb, n_probes, is_w_probing, bg_filename, attack_interval_file = "", file_name = "probe_setup.txt"):
    with open(file_name, "w") as stf:
        stf.write("n_probes " + str(n_probes) + '\n')
        stf.write("E_cur_idxlist " + str(len(E_cur_idxlist)))
        for e in E_cur_idxlist:
            stf.write(" " + str(e))
        stf.write('\n' + "e_new_idx " + str(e_new_idx) + '\n' + "probe_type " + probe_type + '\n' + "len_long_train " + str(len_long_train) + '\n')
        stf.write("n_uePerGnb " + str(n_uePerGnb) + '\n')
        stf.write("is_w_probing " + str(is_w_probing) + '\n')
        if len(attack_interval_file) > 0:
            stf.write("attack_interval_file " + attack_interval_file + '\n')
        if len(bg_filename) > 0:
            stf.write("bd_ratio_file " + str(bg_filename) + '\n')

def wr_probe_intervals(probe_intervals:list, tunnel_list:list, filename="probe_intervals.txt"):
    with open(filename, "w") as fw:
        for i in range(len(tunnel_list)):
            fw.write( str(tunnel_list[i][0]) + ' ' + str(tunnel_list[i][1]) + ' ' + str(probe_intervals[i]) + '\n' )
            
def wr_coordinate_gnb(node_xval, node_yval, filename):
    with open(filename, "w") as f:
        f.write( ' '.join(str(u) for u in node_xval) )
        f.write('\n')
        f.write( ' '.join(str(u) for u in node_yval) )
        
def wr_nUE(n_UE, filename):
    n_UE = [int(u) for u in n_UE]
    with open(filename, "w") as f:
        f.write( ' '.join(str(u) for u in n_UE) )
    
        
def write_setup(file_name, *para):
    with open(file_name, "w") as stf:
        for entry in para:
            stf.write(entry + '\n')
            
def write_attack_rate(file_name, attack_rate):
    with open(file_name, "w") as f:
        f.write( ' '.join(str(int(u)) for u in attack_rate) )
            
def py_run_attack(aDict:dict):
    E_cur_idxlist = aDict['E_cur_idxlist']
    tb = aDict['tb']
    probe_param_interval = 0
    tau = -1
    # dir_name = os.getcwd() + "\\"
    dir_name = os.getcwd() + "/"
    file_hyper_param = dir_name + "hyper_param.txt"
    file_attack_rate = dir_name + "attack_interval.txt"
    '''Write to file'''
    str_old_E = "E_cur_idxlist " + str(len(E_cur_idxlist)) + " " + " ".join( [str(e) for e in E_cur_idxlist] )
    str_pareto_shape = "pareto_shape " + str(aDict['pareto_shape'])
    str_probe_param_interval = "probe_param_interval " + str(probe_param_interval)
    str_probe_pkt_size = "probe_pkt_size " + str(aDict['probe_pkt_size'])
    str_tau = "tau " + str(tau)
    str_tb = "tb " + str(len(tb)) + " " + " ".join([str(u) for u in tb]) 
    write_setup(file_hyper_param, str_old_E, str_pareto_shape, str_tb, str_probe_param_interval, str_probe_pkt_size, str_tau)
    '''Write Attack Rate'''
    write_attack_rate(file_name=file_attack_rate, attack_rate=aDict['attack_rate'])
    str_cmd = "/export/home/Yudi_Huang/ns-3-dev/ns3 run AttackTest/AttackTest"
    os.system(str_cmd)
    
def py_run_attack_v2(aDict:dict):
    E_cur_idxlist = aDict['E_cur_idxlist']
    tb = aDict['tb']
    probe_param_interval = 0
    tau = -1
    # dir_name = os.getcwd() + "\\"
    dir_name = os.getcwd() + "/"
    file_hyper_param = dir_name + "hyper_param.txt"
    file_attack_rate = dir_name + "attack_interval.txt"
    '''Write to file'''
    str_old_E = "E_cur_idxlist " + str(len(E_cur_idxlist)) + " " + " ".join( [str(e) for e in E_cur_idxlist] )
    str_pareto_shape = "pareto_shape " + str(aDict['pareto_shape'])
    str_probe_param_interval = "probe_param_interval " + str(probe_param_interval)
    str_probe_pkt_size = "probe_pkt_size " + str(aDict['probe_pkt_size'])
    str_tau = "tau " + str(tau)
    str_tb = "tb " + str(len(tb)) + " " + " ".join([str(u) for u in tb]) 
    write_setup(file_hyper_param, str_old_E, str_pareto_shape, str_tb, str_probe_param_interval, str_probe_pkt_size, str_tau)
    '''Write Attack Rate'''
    write_attack_rate(file_name=file_attack_rate, attack_rate=aDict['attack_rate'])
    str_cmd = "/export/home/Yudi_Huang/ns-3-dev/ns3 run AttackTestV2"
    os.system(str_cmd)
    
    
def Py_process_delays(delays_raw, delays_vec):
    for line in delays_raw:
        tb = line[1]
        idx_zero = np.where(line[2:] == 0)[0]
        if(len(idx_zero) == 0):
            avg_delay = np.mean( line[2:] )
        else:
            avg_delay = np.mean( line[2:idx_zero[0]] )
        delays_vec[tb] = avg_delay
    
            
def run_simulation(para_from_matlab):
    if "task_name" in para_from_matlab:
        task_name = para_from_matlab['task_name']
    else:
        task_name = "Category_inference"
    # dir_name = "/export/home/Yudi_Huang/ns-3-dev/scratch/Category_inference/"
    dir_name = "/export/home/Yudi_Huang/ns-3-dev/scratch/"+ task_name + "/"
    # dir_name = ""
    file_hyper_param = dir_name + "hyper_param.txt"
    E_cur_idxlist = para_from_matlab['E_cur_idxlist']
    pareto_shape = para_from_matlab['shape']
    tb = para_from_matlab['tb']
    probe_param_interval = int(para_from_matlab['probe_param_interval'])
    probe_param_interval = floor(probe_param_interval)
    probe_pkt_size = int(para_from_matlab['probe_pkt_size'])
    tau = int(para_from_matlab['tau']) - 1
    '''Index Transformation'''
    E_cur_idxlist = [int(e-1) for e in E_cur_idxlist]
    # probe_pkt_size = int(probe_pkt_size)
    if type(tb) == list:
        tb = [int(e-1) for e in tb]
        str_tb = "tb " + str(len(tb)) + " " + " ".join([str(u) for u in tb]) 
    else: 
        tb = int(tb - 1)
        str_tb = "tb " + str(1) + " " + str(tb)
    # if type(tb) == int:
    #     tb = int(tb - 1)
    #     str_tb = "tb " + str(1) + " " + str(tb)
    # else: 
    #     tb = [int(e-1) for e in tb]
    #     str_tb = "tb " + str(len(tb)) + " " + " ".join([str(u) for u in tb]) 
        
    
    '''Write to file'''
    str_old_E = "E_cur_idxlist " + str(len(E_cur_idxlist)) + " " + " ".join( [str(e) for e in E_cur_idxlist] )
    str_pareto_shape = "pareto_shape " + str(pareto_shape)
    str_probe_param_interval = "probe_param_interval " + str(probe_param_interval)
    str_probe_pkt_size = "probe_pkt_size " + str(probe_pkt_size)
    str_tau = "tau " + str(tau)
    
    write_setup(file_hyper_param, str_old_E, str_pareto_shape, str_tb, str_probe_param_interval, str_probe_pkt_size, str_tau)
    str_cmd = "/export/home/Yudi_Huang/ns-3-dev/ns3 run " + task_name
    os.system(str_cmd)
    # os.system("/export/home/Yudi_Huang/ns-3-dev/ns3 run Category_inference")
    
            
def init_setup(para_from_matlab):
    # para_dict = sio.loadmat('para2py_hex3.mat', struct_as_record=False, squeeze_me=True)['para2py']
    # para_dict = sio.loadmat('para_hex_nb10_homo.mat', struct_as_record=False, squeeze_me=True)['para2py']
    para_dict = sio.loadmat('para_hex_nb10_het.mat', struct_as_record=False, squeeze_me=True)['para2py']
    # para_dict = sio.loadmat('para_hex_nb10_het.mat', struct_as_record=False, squeeze_me=True)['para2py']
    Adj, D, SPR, sa, sb, ta, tb = para_dict.Adj, para_dict.D, para_dict.SPR, para_dict.sa, para_dict.sb, para_dict.ta, para_dict.tb
    node_xval, node_yval = para_dict.node_xval, para_dict.node_yval
    '''Index Transformation'''
    SPR = np.array( [route-1 for route in SPR if len(route) > 0] )
    sa, sb = sa - 1, sb - 1
    ta = np.array( [u for u in ta] )
    tb = tb - 1
    if "task_name" in para_from_matlab:
        task_name = para_from_matlab['task_name']
    else:
        task_name = "Category_inference"
    
    bw = [2000000]*len(D[0]) # bw in kbps, delay in microsec (us).
    delay = [20]*len(D[0]) # bw in kbps, delay in microsec (us).

    tunnel_list = [ (route[0], route[-1]) for route in SPR if len(route) > 0 ]
    n_tunnels = len(tunnel_list)
    tunnel_demands = [10] * n_tunnels
    probe_intervals = [10] * n_tunnels # microsec (us)

    # dir_name = "/export/home/Yudi_Huang/ns-3-dev/scratch/Category_inference/"
    dir_name = "/export/home/Yudi_Huang/ns-3-dev/scratch/" + task_name + "/"
    graph_name = dir_name + "Hex3.graph"
    name_overlay_nodes = dir_name + "overlay_node_Hex3.txt"
    name_demands = dir_name + "tunnel_demands.txt"
    route_name = dir_name + "route_table_Hex3.txt"
    probe_interval_files = dir_name + "probe_intervals.txt"
    probe_setup_name = dir_name + "probe_setup.txt"
    gnb_coordinate_file_name = dir_name + "coordinate_gnb.txt"
    hyper_param_filename = dir_name + "hyper_param.txt"
    nUE_filename = dir_name + "nUE_file.txt"
    
    adj2underlay(Adj=Adj, D=D, bw=bw, delay=delay, filename=graph_name)
    SPR2routeTable(SPR=SPR, filename=route_name)
    E_cur_idxlist, e_new_idx, n_calibrate_pkt, n_uePerGnb, n_probes = para_from_matlab['E_cur_idxlist'], para_from_matlab['e_new_idx'], para_from_matlab['n_calibrate_pkt'], para_from_matlab['n_uePerGnb'], para_from_matlab['n_probes']
    n_UE, is_w_probing = para_from_matlab['n_UE'], para_from_matlab['is_w_probing']
    if 'bg_filename' in para_from_matlab:
        bg_filename = para_from_matlab['bg_filename']
    else:
        bg_filename = ""
    '''Index Transformation'''
    n_UE = [int(u) for u in n_UE]
    E_cur_idxlist = [int(e-1) for e in E_cur_idxlist]
    e_new_idx = int(e_new_idx - 1)
    n_calibrate_pkt = int(n_calibrate_pkt)
    n_uePerGnb, n_probes = int(n_uePerGnb), int(n_probes)
    
    wr_probe_setup(E_cur_idxlist=E_cur_idxlist, e_new_idx = e_new_idx, probe_type="naive", len_long_train=n_calibrate_pkt, n_uePerGnb=n_uePerGnb, n_probes=n_probes, is_w_probing= is_w_probing, bg_filename=bg_filename, file_name = probe_setup_name)
    wr_overlayNodes(overlay_node_list=[*range(len(Adj[0]))], filename=name_overlay_nodes)
    write_tunnel_demands(tunnel_demands=tunnel_demands, tunnel_list=tunnel_list, filename=name_demands)
    wr_probe_intervals(probe_intervals=probe_intervals, tunnel_list=tunnel_list, filename=probe_interval_files)
    wr_coordinate_gnb(node_xval=node_xval, node_yval=node_yval, filename=gnb_coordinate_file_name)
    wr_nUE(n_UE=n_UE, filename=nUE_filename)
    write_setup("setup.txt", "graph_name " + graph_name, "name_overlay_nodes " + name_overlay_nodes, "name_demands " + name_demands, "route_name " + route_name, "probe_setup_filename " + probe_setup_name, "probe_interval_filename " + probe_interval_files, "gnb_coordinate_file " + gnb_coordinate_file_name, "hyper_param_filename " + hyper_param_filename, "nUE_filename " + nUE_filename)