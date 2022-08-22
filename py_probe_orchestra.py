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