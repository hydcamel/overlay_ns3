# -*- coding: utf-8 -*-
"""
Created on Tue Sep 13 12:36:40 2022

@author: robbe
"""

import scipy.io as sio
import XPAttack_utils as utils


para_dict = sio.loadmat('para2py_hex3.mat', struct_as_record=False, squeeze_me=True)['para2py']
Adj, D, SPR, sa, sb, ta, tb = para_dict.Adj, para_dict.D, para_dict.SPR, para_dict.sa, para_dict.sb, para_dict.ta, para_dict.tb
node_xval, node_yval = para_dict.node_xval, para_dict.node_yval
utils.wr_coordinate_gnb(node_xval=node_xval, node_yval=node_yval, filename="coordinate_gnb.txt")
utils.init_setup()