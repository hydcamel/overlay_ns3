clear classes
is_recompute = 0;
is_init = 0;
global root n_iab SPR XPAttack_utils thr_burst tol
if is_recompute == 1
    G_type = 0;
    [ Adj, D, Adj_full, sa, sb, ta, tb, path_a, path_b, DG, SPR, node_xval, node_yval ] = G_gen(G_type); % G_type = 0:Hex
    SPR(sa) = [];
    n_iab = length(Adj);
    path_a_full = path_a; path_b_full = path_b; 
    for i = 1 : n_iab
        path_a_full{i} = [path_a_full{i}, i + n_iab];
    end
    for i = 1 : length(path_b)
        tmp = path_b_full{i};
        path_b_full{i} = [path_b_full{i}, tmp(end) + n_iab];
    end
    
    link_parameter = struct('lambda', zeros(length(Adj_full), length(Adj_full)), 'mu', zeros(length(Adj_full), length(Adj_full)));
    [T_true,W_true,para_true]=T_groundtruth(Adj_full ,path_a_full, path_b_full, link_parameter);
    para2py.Adj = Adj;
    para2py.SPR = SPR;
    para2py.D = D;
    para2py.sa = sa;
    para2py.sb = sb;
    para2py.ta = ta;
    para2py.tb = 3;
    para2py.node_xval = round(node_xval, 2);
    para2py.node_yval = round(node_yval, 2);
    if G_type == 0
        if n_iab <= 20
            save(strcat('para2py_hex3.mat'), 'para2py');
        else
            save(strcat('para2py_hex4.mat'), 'para2py');
        end
    end
else
    load('para_hex_matlab.mat');
end

%% Init
root = 2;
% thr_burst = 338483;
thr_burst = load('Thr_burst_pareto_hex3.mat');
thr_burst = thr_burst.thr_burst_delay;
tol = 0.05;
ta_init = [20, 36] - n_iab;
tb_init = tb(1) - n_iab;
E_cur_idxlist = zeros(1,3);
idx = 1;
n_uePerGnb = 1;
n_probes = 1000;

para_probe = py.dict(pyargs('E_cur_idxlist',E_cur_idxlist,'e_new_idx',1,'n_calibrate_pkt',1000, 'n_uePerGnb', n_uePerGnb, 'n_probes', n_probes));
%% Prepare Python Env
if count(py.sys.path,'') == 0
    insert(py.sys.path,int32(0),'');
end
XPAttack_utils = py.importlib.import_module('XPAttack_utils');
py.importlib.reload(XPAttack_utils);
% py.XPAttack_utils.init_setup(para_probe);
if is_init > 0
    py.XPAttack_utils.run_simulation(para_probe);
end

%% Iterate over each branch of the root
BT_set = find(T_true(root,:));
BT_set = BT_set( BT_set <= n_iab );
tb_per = tb(1);
W=zeros(length(T_true),length(T_true)); %W(i,j) means the weight share with edge (i,j) in T
for i = 1 : length(BT_set)
    T_per = T_true;
    T_per(root, BT_set) = 0;
    T_per(root, n_iab+1:end) = 0;
    T_per(root, BT_set(i)) = 1;
    new_tT = zeros(1, n_iab);
    idx = 1;
    for j = 1 : n_iab
        route = path_a_full{j};
        if( route(2) == BT_set(i) ) 
            new_tT(idx) = route(end);
            idx = idx + 1;
        end
    end
    [ W ] = W_Inference( Adj_full, W, T_per, path_a_full, path_b_full, tb_per, new_tT(1:idx-1) );
end



for i = 1 : length(SPR)
    if ismember( SPR{i}(end), [ta_init, tb_init] )
        E_cur_idxlist(idx) = i;
        idx = idx + 1;
    end
end
