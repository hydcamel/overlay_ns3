# -*- coding: utf-8 -*-
"""
Created on Wed Sep 28 09:25:01 2022

@author: robbe
"""

import numpy as np
import scipy.io as sio
import os
import XPAttack_utils as utils

dir_name = "/export/home/Yudi_Huang/ns-3-dev/scratch/Category_inference/"
# dir_name = ""
graph_name = dir_name + "Hex3.graph"
name_overlay_nodes = dir_name + "overlay_node_Hex3.txt"
name_demands = dir_name + "tunnel_demands.txt"
route_name = dir_name + "route_table_Hex3.txt"
probe_interval_files = dir_name + "probe_intervals.txt"
probe_setup_name = dir_name + "probe_setup.txt"
gnb_coordinate_file_name = dir_name + "coordinate_gnb.txt"
hyper_param_filename = dir_name + "hyper_param.txt"

para_dict = sio.loadmat(dir_name+'para2py_hex3.mat', struct_as_record=False, squeeze_me=True)['para2py']
Adj, D, SPR, sa, sb, ta, tb = para_dict.Adj, para_dict.D, para_dict.SPR, para_dict.sa, para_dict.sb, para_dict.ta, para_dict.tb
node_xval, node_yval = para_dict.node_xval, para_dict.node_yval
'''Index Transformation'''
SPR = np.array( [route-1 for route in SPR if len(route) > 0] )

E_cur_idxlist = [e for e in range(len(SPR))]

'''Write to file'''
str_old_E = "E_cur_idxlist " + str(len(E_cur_idxlist)) + " " + " ".join( [str(e) for e in E_cur_idxlist] )
utils.write_setup(hyper_param_filename, str_old_E)
os.system("/export/home/Yudi_Huang/ns-3-dev/ns3 run Category_inference")