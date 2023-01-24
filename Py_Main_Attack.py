# -*- coding: utf-8 -*-
"""
Created on Mon Oct 31 16:44:41 2022

@author: yxh5389
"""
import numpy as np
import scipy.io as sio
import os
import XPAttack_utils as utils
import pickle

is_het_bg = False
is_w_est = True
is_nb_10 = True
# if is_het_bg:
#     if is_w_est:
#         para_dict = sio.loadmat('para_hex_matlab_vbg.mat', struct_as_record=False, squeeze_me=True)
#         attack_rate = sio.loadmat('Attack_Interval_est_het.mat', struct_as_record=False, squeeze_me=True)
#     else:
#         para_dict = sio.loadmat('para_hex_matlab_vbg.mat', struct_as_record=False, squeeze_me=True)
#         attack_rate = sio.loadmat('Attack_Interval_het.mat', struct_as_record=False, squeeze_me=True)
# else:
#     if is_w_est:
#         para_dict = sio.loadmat('para_hex_matlab.mat', struct_as_record=False, squeeze_me=True)
#         attack_rate = sio.loadmat('Attack_Interval_est.mat', struct_as_record=False, squeeze_me=True)
#     else:
#         para_dict = sio.loadmat('para_hex_matlab.mat', struct_as_record=False, squeeze_me=True)
#         attack_rate = sio.loadmat('Attack_Interval.mat', struct_as_record=False, squeeze_me=True)
        
# para_dict = sio.loadmat('para_hex_nb10_homo.mat', struct_as_record=False, squeeze_me=True)
# attack_rate = sio.loadmat('Attack_Interval_est_homo_nb10.mat', struct_as_record=False, squeeze_me=True)
para_dict = sio.loadmat('para_hex_nb10_het.mat', struct_as_record=False, squeeze_me=True)
attack_rate = sio.loadmat('Attack_Interval_est_het_nb10.mat', struct_as_record=False, squeeze_me=True)
cell_even_rank_iv = attack_rate['cell_even_rank_iv']
cell_optimal_iv = attack_rate['cell_optimal_iv']
cell_MM1_rank_iv = attack_rate['cell_MM1_rank_iv']
cell_MD1_rank_iv = attack_rate['cell_MD1_rank_iv']
cell_rd_rank_iv = attack_rate['cell_rd_rank_iv']
list_total_rate = attack_rate['list_total_rate']
is_no_attack = True
# is_no_attack = False

bw = 2000 # 2000Mbps = 2Gbps
n_MC = para_dict['n_MC']
nb = para_dict['nb']
root = 1;
base_bg_ratio = 4; # 4 = 25% = 500M/2G
base_bg_rate = 500; # 4 = 25% = 500M/2G
pareto_shape = 2.04
n_iab = para_dict['n_iab']
n_probes = 10000
probe_pkt_size = 1000
dir_name = os.getcwd() + "/"
# dir_name = os.getcwd() + "\\"
probe_setup_name = dir_name + "probe_setup.txt"
nUE_filename = dir_name + "nUE_file.txt"
attack_interval_file = dir_name + "attack_interval.txt"
if is_no_attack:
    delay_res_file = dir_name + "res_delays_all_wna_het_nb10_12.pkl"
else:
    delay_res_file = dir_name + "res_delays_all_ona_het_nb10_12.pkl"
is_attack_test = 10
n_rd = len(cell_rd_rank_iv[0][0])
n_rd = 3

if "cell_background_percent" in para_dict:
    is_het_bg = True
else:
    is_het_bg = False

aDict = {'Name': 'aDict'}


if is_no_attack:
    # delay_no_attack = np.array((n_iab, n_MC))
    dict_no_attack = {'title': 'na'}
    
dict_delays_opt = {'title': 'opt'}
dict_delays_mm1 = {'title': 'mm1'}
dict_delays_md1 = {'title': 'md1'}
dict_delays_rd = {'title': 'rd'}
dict_delays_even = {'title': 'even'}
    
for k in range(len(list_total_rate)):
    if k not in [12]:
        continue
    optimal_iv = cell_optimal_iv[k]
    MM1_rank_iv = cell_MM1_rank_iv[k]
    MD1_rank_iv = cell_MD1_rank_iv[k]
    rd_iv = cell_rd_rank_iv[k]
    even_iv = cell_even_rank_iv[k]
    delays_opt = np.zeros((n_iab, n_MC))
    delays_MM1 = np.zeros((n_iab, n_MC))
    delays_MD1 = np.zeros((n_iab, n_MC))
    delays_rd = [np.zeros((n_iab, n_MC))] * n_rd
    delays_even = np.zeros((n_iab, n_MC))
    if is_no_attack:
        delay_no_attack = np.zeros((n_iab, n_MC))
    
    for i in range(n_MC):
        if is_het_bg:
            if is_nb_10:
                bg_filename = dir_name + "bg_ratio_nb10_" + str(i+1) + ".csv"
            else:
                bg_filename = dir_name + "bg_ratio_" + str(i+1) + ".csv"
            # utils.wr_probe_setup(E_cur_idxlist=[-1,-1,-1], e_new_idx = 0, probe_type="naive", len_long_train=0, n_uePerGnb=1, n_probes=n_probes, is_w_probing= 0, bg_filename=bg_filename, file_name = probe_setup_name)
        else:
            bg_filename = dir_name + "bg_ratio_0" + ".csv"
            # utils.wr_probe_setup(E_cur_idxlist=[-1,-1,-1], e_new_idx = 0, probe_type="naive", len_long_train=0, n_uePerGnb=1, n_probes=n_probes, is_w_probing= 0, bg_filename=bg_filename, file_name = probe_setup_name)
        # else:
        #     utils.wr_probe_setup(E_cur_idxlist=[-1,-1,-1], e_new_idx = 0, probe_type="naive", len_long_train=0, n_uePerGnb=1, n_probes=n_probes, is_w_probing= 0, file_name = probe_setup_name)
        
        tbs = para_dict['cell_tb'][i]
        tbs_gnb = tbs - n_iab - 1
        E_cur_idxlist = [None] * len(tbs)
        for u in range(len(tbs)):
            if tbs_gnb[u] >= 2:
                E_cur_idxlist[u] = tbs_gnb[u] - 1
            else:
                E_cur_idxlist[u] = tbs_gnb[u]
        
        # print(tbs_gnb)
        aDict['tb'] = tbs_gnb
        aDict['E_cur_idxlist'] = E_cur_idxlist
        aDict['pareto_shape'] = pareto_shape
        aDict['probe_pkt_size'] = probe_pkt_size
        aDict['attack_rate'] = attack_rate
        utils.wr_probe_setup(E_cur_idxlist=[-1,-1,-1], e_new_idx = 0, probe_type="naive", len_long_train=0, n_uePerGnb=1, n_probes=n_probes, is_w_probing= 0, bg_filename=bg_filename, attack_interval_file = attack_interval_file, file_name = probe_setup_name)
        
        '''Delay track wihout attack'''
        utils.wr_nUE(n_UE=para_dict['cnt_nTarget'][i], filename=nUE_filename)
        if is_no_attack:
            aDict['attack_rate'] = [0] * n_iab
            utils.py_run_attack(aDict = aDict)
            delays_raw = np.loadtxt("delays_cnt.csv",delimiter=", ", dtype=int)
            utils.Py_process_delays(delays_raw=delays_raw, delays_vec=delay_no_attack[:, i])
            
        '''Optimal Attack'''
        aDict['attack_rate'] = optimal_iv[:, i]
        utils.py_run_attack(aDict = aDict)
        delays_raw = np.loadtxt("delays_cnt.csv",delimiter=", ", dtype=int)
        utils.Py_process_delays(delays_raw=delays_raw, delays_vec=delays_opt[:, i])
        '''MM1_Rank'''
        aDict['attack_rate'] = MM1_rank_iv[:, i]
        utils.py_run_attack(aDict = aDict)
        delays_raw = np.loadtxt("delays_cnt.csv",delimiter=", ", dtype=int)
        utils.Py_process_delays(delays_raw=delays_raw, delays_vec=delays_MM1[:, i])
        '''MD1_Rank'''
        aDict['attack_rate'] = MD1_rank_iv[:, i]
        utils.py_run_attack(aDict = aDict)
        delays_raw = np.loadtxt("delays_cnt.csv",delimiter=", ", dtype=int)
        utils.Py_process_delays(delays_raw=delays_raw, delays_vec=delays_MD1[:, i])
        '''Random_Rank'''
        for l in range(n_rd):
            aDict['attack_rate'] = rd_iv[:, l]
            utils.py_run_attack(aDict = aDict)
            delays_raw = np.loadtxt("delays_cnt.csv",delimiter=", ", dtype=int)
            utils.Py_process_delays(delays_raw=delays_raw, delays_vec=delays_rd[l][:, i])
        '''Even Attack'''
        aDict['attack_rate'] = even_iv[:, i]
        utils.py_run_attack(aDict = aDict)
        delays_raw = np.loadtxt("delays_cnt.csv",delimiter=", ", dtype=int)
        utils.Py_process_delays(delays_raw=delays_raw, delays_vec=delays_even[:, i])
        '''Save data during the process as checkpoint'''
        dict_delays_opt[k] = delays_opt
        dict_delays_mm1[k] = delays_MM1
        dict_delays_md1[k] = delays_MD1
        dict_delays_rd[k] = delays_rd
        dict_delays_even[k] = delays_even  
        if is_no_attack:
            dict_no_attack[k] = delay_no_attack
            with open(delay_res_file, "wb") as f:
                pickle.dump([dict_delays_opt, dict_delays_mm1, dict_delays_md1, dict_delays_rd, dict_delays_even, dict_no_attack], f)
        else:
            with open(delay_res_file, "wb") as f:
                pickle.dump([dict_delays_opt, dict_delays_mm1, dict_delays_md1, dict_delays_rd, dict_delays_even], f)
            