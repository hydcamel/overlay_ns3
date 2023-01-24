clear classes
is_recompute = 0;
is_init = 0;
do_w_probing = 1;
is_het_bg = 1;
global root n_iab SPR XPAttack_utils thr_burst tol pareto_shape enum_pareto_shape
global enum_traffic param_probe param_background param_target
n_MC = 10;
nb = 5;
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
    
    cnt_nTarget = zeros(n_MC, n_iab);
    cell_tb = cell(1, n_MC);
    W_true_cell = cell(1, n_MC);
    path_b_full_cell = cell(1, n_MC);
    cell_background_percent = cell(1, n_MC);
    for i = 1: n_MC
        list_tb = randi([1 n_iab],1,nb);
        list_tb( list_tb == 2 ) = 3;
        for j = 1:nb
            cnt_nTarget(i, list_tb(j)) = cnt_nTarget(i, list_tb(j)) + 1;
        end
        tmp_tbs = unique(list_tb);
        cell_tb{i} = tmp_tbs + n_iab;
        path_b_full = cell(1, length(tmp_tbs));
        for k = 1 : length(path_b_full)
            if tmp_tbs(k) == 1
                path_b_full{k} = [SPR{1}, 1+n_iab];
            else
                path_b_full{k} = [SPR{tmp_tbs(k)-1}, tmp_tbs(k)+n_iab];
            end
        end
        [~,W_true,~]=T_groundtruth(Adj_full ,path_a_full, path_b_full, link_parameter);
        W_true_cell{i} = W_true;
        path_b_full_cell{i} = path_b_full;
        cell_background_percent{i} = round( unifrnd(25/35*4, 25/15*4, n_iab, n_iab), 3 );
        filename = strcat('bg_ratio_',string(i),'.csv');
        writematrix(cell_background_percent{i},filename,'Delimiter',' ');
    end
    
    
    
    
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
%             save(strcat('para2py_hex3.mat'), 'para2py');
%             save('para_hex_matlab.mat');
            save('para_hex_matlab_vbg.mat');
        else
            save(strcat('para2py_hex4.mat'), 'para2py');
        end
    end
else
    if is_het_bg > 0
        load('para_hex_matlab_vbg.mat');
    else
        load('para_hex_matlab.mat');
    end
end

%% Init
root = 2;
base_bg_ratio = 4; %% 4 = 25% = 500M/2G
base_bg_rate = 500; %% 4 = 25% = 500M/2G
enum_traffic = 1:3; % 1: Poisson, 2: ON/OFF, 3: CRF
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
n_UE = ones(1, n_iab);
bw_base = 2000; % 2000Mbps = 2Gbps
bw = bw_base * ones(n_iab, n_iab); % 2000Mbps = 2Gbps
enum_pareto_shape = [1.14, 1.44, 1.74, 2.04];
is_w_probing = 10;
is_init = 0;
do_w_probing = 10;

% n_UE(tb - n_iab) = 2;


if do_w_probing
    para_probe = py.dict(pyargs('E_cur_idxlist',E_cur_idxlist,'e_new_idx',1,'n_calibrate_pkt',1000, 'n_uePerGnb', n_uePerGnb, 'n_probes', n_probes, 'n_UE', n_UE, 'is_w_probing', is_w_probing));
    %% Prepare Python Env
    if count(py.sys.path,'') == 0
        insert(py.sys.path,int32(0),'');
    end
    XPAttack_utils = py.importlib.import_module('XPAttack_utils');
    py.importlib.reload(XPAttack_utils);
    % py.XPAttack_utils.init_setup(para_probe);
    if is_init > 0
        py.XPAttack_utils.init_setup(para_probe);
    end

    %% Iterate over each branch of the root
    list_n_probes = [100, 500, 1000, 2000];
    list_n_probes = [1000];
    acc = zeros(size(list_n_probes, 2), n_MC);
    BT_set = find(T_true(root,:));
    BT_set = BT_set( BT_set <= n_iab );
    pareto_shape = 2.04;
    global list_false
    list_false = [];
% %     acc = zeros(1, length(enum_pareto_shape));
% %     res_W_inf_pareto = cell(length(enum_pareto_shape), n_MC);
% %     for iter_shape_idx = 1 : length(enum_pareto_shape)
% %         pareto_shape = enum_pareto_shape(iter_shape_idx);
    for k = 1 : size(list_n_probes, 2)
%         bg_filename = strcat('bg_ratio_',string(i),'.csv');
%         para_probe = py.dict(pyargs('E_cur_idxlist',E_cur_idxlist,'e_new_idx',1,'n_calibrate_pkt',1000, 'n_uePerGnb', n_uePerGnb, 'n_probes', list_n_probes(k), 'n_UE', n_UE, 'is_w_probing', is_w_probing, ''));
%         py.XPAttack_utils.init_setup(para_probe);
        for iter = 1 : n_MC
            if is_het_bg > 0
                bg_filename = strcat('/export/home/Yudi_Huang/ns-3-dev/scratch/Category_inference/bg_ratio_',string(iter),'.csv');
                para_probe = py.dict(pyargs('E_cur_idxlist',E_cur_idxlist,'e_new_idx',1,'n_calibrate_pkt',1000, 'n_uePerGnb', n_uePerGnb, 'n_probes', list_n_probes(k), 'n_UE', n_UE, 'is_w_probing', is_w_probing, 'bg_filename', bg_filename));
                py.XPAttack_utils.init_setup(para_probe);
            end
            tb = cell_tb{iter};
            W=cell(1,length(tb));  %if tree in slice A shares with path in slice B
            for idx_tb = 1 : length(tb)
%                 idx_tb = 2;
                tb_per = tb(idx_tb);
                W{idx_tb}=zeros(length(T_true),length(T_true)); %W(i,j) means the weight share with edge (i,j) in T
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
                    [ W{idx_tb} ] = W_Inference( Adj_full, W{idx_tb}, T_per, path_a_full, path_b_full, tb_per, new_tT(1:idx-1) );
                end
            end
            [acc(k, iter)] = inference_acc(W_true_cell{iter},W);
            save(strcat('W_acc_', string(k), '_' , string(iter) ,'.mat'), 'W', 'acc');
        end
    end
    
%         [acc(iter_shape_idx)] = inference_acc(W_true_cell{iter},W);
%         save(strcat('W_acc_', str(pareto_shape) ,'.mat'), 'W', 'acc');
        %% save W and tb for current (1) shape + (2) tb
    
end

% global vec_stem_coordinate idx_stem vec_tau
% for iter = 1 : n_MC
%     W_total = zeros(length(T_true),length(T_true));
%     for u = 1 : length(cell_tb{iter})
%         W_total = W_total + W_true_cell{1,iter}{1,u};
%     end
%     W_total = W_total > 0;
%     idx_stem = 1;
%     vec_stem_coordinate = zeros(2, sum(sum(W_total)));
%     vec_tau = zeros(1,length(vec_stem_coordinate));
% end
n_BW_slice = 20;
delays_log = -1 * ones( length(T_true), n_BW_slice + 3 ); % 1: s, 2: t, 3: tau
delays_log_burst = -1 * ones( length(T_true), n_BW_slice + 3 ); % 1: s, 2: t, 3: tau
res_MM1 = zeros(size(delays_log,1), 3); %% background + capacity
res_MD1 = zeros(size(delays_log,1), 3); %% background + capacity
res_MM1_burst = zeros(size(delays_log,1), 3); %% background + capacity
res_MD1_burst = zeros(size(delays_log,1), 3); %% background + capacity
idx_delay = 1;
probe_pkt_size = 1000; % bytes
base_interval = 40000; % 40000ns = 200 mb/s
base_rate = 200;
target_rate = 100;
pareto_shape = enum_pareto_shape(end);
fun1=@(x,xdata)fit_function(x,xdata,1);
fun2=@(x,xdata)fit_function(x,xdata,2);
back_ground_traffic = 500;
vec_max_probing_param_rate = -1*ones(1, size(delays_log,1));
vec_n_shared_links = zeros(1, n_MC);

for iter = 1 : n_MC
% for iter = 1 : n_MC
    W_seen = zeros(length(T_true), length(T_true));
    ABW_true = 0 * W_seen;
    for u = 1 : length(cell_tb{iter})
        ABW_true = ABW_true + target_rate * (cnt_nTarget(iter, cell_tb{iter}(u)-n_iab)) * (W_true_cell{1,iter}{1,u} > 0);
    end
    ABW_true = bw_base - ABW_true - back_ground_traffic;
    tbs = cell_tb{iter};
    vec_n_UE = py.list(cnt_nTarget(iter, :));
    py.XPAttack_utils.wr_nUE(vec_n_UE, 'nUE_file.txt');
    E_to_tbs = tbs - n_iab;
    E_to_tbs(E_to_tbs >= 3) = E_to_tbs(E_to_tbs >= 3) - 1;
    for u = 1 : length(cell_tb{iter})
%     for u = 1 : 1
        W = W_true_cell{1,iter}{1,u};
        tb = tbs(u);
        n_shared_links = sum( sum(W>0) );
        vec_tau = -1*ones(1, n_shared_links);
        idx = 1;
        for m = 1 : length(W)
            list_sharedL = find(W(m, :) > 0);
            if isempty(list_sharedL)
                continue;
            end
            for it = list_sharedL
                if it - m == n_iab || W_seen(m, it) > 0
                    continue;
                end
                vec_tau(idx) = find_tau(T_true, it, W);
                delays_log(idx_delay, 1) = m;
                delays_log(idx_delay, 2) = it;
                delays_log(idx_delay, 3) = vec_tau(idx);
                delays_log_burst(idx_delay, 1) = m;
                delays_log_burst(idx_delay, 2) = it;
                delays_log_burst(idx_delay, 3) = vec_tau(idx);
                W_seen(m, it) = 1;
                idx = idx + 1;
                idx_delay = idx_delay + 1;
            end
        end
        vec_tau = vec_tau(vec_tau >= 0);
        idx_start = idx_delay - length(vec_tau);
        vec_n_shared_links(iter) = vec_n_shared_links(iter) + length(vec_tau);
% %         for m = 1 : length(vec_tau)
% %             [id_route_tau, route_to_tau] = route_find(vec_tau(m)-n_iab);
% %             tau_path_bw = find_path_bw(route_to_tau, ABW_true); % Mbps
% %             vec_max_probing_param_rate(idx_start+m-1) = tau_path_bw; % Mbps
% % %             probe_param_interval = probe_pkt_size * 8 / (tau_path_bw); % microseconds
% %             for k = 0 : n_BW_slice-1
% %                 probe_param_interval = 1/(tau_path_bw * k / n_BW_slice / base_rate) * base_interval; % nanoseconds
% %                 if isinf(probe_param_interval)
% %                     probe_param_interval = 0;
% %                 end
% %                 %% Run simulation through python
% %                 para_probe = py.dict(pyargs('E_cur_idxlist', E_to_tbs, 'shape', pareto_shape, 'tb', [tb-n_iab], 'probe_param_interval', probe_param_interval, 'probe_pkt_size', probe_pkt_size, 'tau', vec_tau(m)-n_iab));
% %                 %% Prepare Python Env
% %                 if count(py.sys.path,'') == 0
% %                     insert(py.sys.path,int32(0),'');
% %                 end
% %                 py.XPAttack_utils.run_simulation(para_probe);
% %                 %% Binarization
% %                 delays_raw = readmatrix('delays_cnt.csv');
% %                 delays_raw = delays_raw(delays_raw(:,3)>0, :);
% %                 delays_raw = delays_raw(delays_raw>0);
% %                 delays_log(idx_start+m-1, 4+k) = mean(rmoutliers(delays_raw(3:end)));
% %                 delays_log_burst(idx_start+m-1, 4+k) = mean(delays_raw(3:end));
% %             end
% %             save('delays_log.mat','delays_log', 'delays_log_burst');
% %         end
        %% parameter estimation for the current target path: for each of the shared link
%         options.MaxFunctionEvaluations = 1000; 
%         options = optimoptions('lsqcurvefit','Algorithm','levenberg-marquardt');
%         
%         for m = 1 : length(vec_tau)
%             x0=[0,3,0.3];
%             res_MM1(idx_start+m-1,:)=lsqcurvefit(fun1,x0, tau_path_bw * (0:n_BW_slice-1)./ n_BW_slice/1e3 ,delays_log(idx_start+m-1,4:end)/1e6,[0 1.5 0],[1 5 0.4],options);
%             res_MD1(idx_start+m-1,:)=lsqcurvefit(fun2,x0, tau_path_bw * (0:n_BW_slice-1)./ n_BW_slice/1e3 ,delays_log(idx_start+m-1,4:end)/1e6,[0 1.5 0],[1 5 0.4],options);
%             res_MM1_burst(idx_start+m-1,:)=lsqcurvefit(fun1,x0, tau_path_bw * (0:n_BW_slice-1)./ n_BW_slice/1e3 ,delays_log_burst(idx_start+m-1,4:end)/1e6,[0 1.5 0],[1 5 0.4],options);
%             res_MD1_burst(idx_start+m-1,:)=lsqcurvefit(fun2,x0, tau_path_bw * (0:n_BW_slice-1)./ n_BW_slice/1e3 ,delays_log_burst(idx_start+m-1,4:end)/1e6,[0 1.5 0],[1 5 0.4],options);
%         end
    end
    
end

tick_bw = 0 : n_BW_slice-1;
[~,sp]=Dijkstra_source(T_true,root);
load('delays_log_1w.mat');
delays_log(:, 4:end) = delays_log(:, 4:end) / 1e6; %% ns to ms
delays_delta = delays_log;
delays_delta(:, 4:end) = delays_delta(:, 4:end) - delays_delta(:, 4);
cell_param_mm1_exp = cell(1, n_MC);
cell_param_md1_mu_exp = cell(1, n_MC);
cell_param_md1_lambda_exp = cell(1, n_MC);
for iter = 1 : n_MC
%     param_est = -1*ones(size(W,1), size(W,1));
    ABW_true = 0 * W_seen;
    for u = 1 : length(cell_tb{iter})
        ABW_true = ABW_true + target_rate * (cnt_nTarget(iter, cell_tb{iter}(u)-n_iab)) * (W_true_cell{1,iter}{1,u} > 0);
    end
    ABW_true = bw_base - ABW_true - back_ground_traffic;
    tbs = cell_tb{iter};
    idx_start = sum(vec_n_shared_links(1:iter-1))+1;
    idx_end = sum(vec_n_shared_links(1:iter));
%     param_est = param_MM1_empirical(delays_log(sum(vec_n_shared_links(1:iter-1))+1, sum(vec_n_shared_links(1:iter)), :), param_est, tbs, ABW_true, sp, T_true, tick_bw);
    param_est = param_MM1_empirical(delays_delta(idx_start: idx_end, :), tbs, ABW_true, sp, tick_bw); %% for one iter in MC
    [mu_est, lambda_est] = param_MD1_empirical(delays_delta(idx_start: idx_end, :), tbs, ABW_true, sp, tick_bw); %% for one iter in MC
    mask = param_est > 0;
    cell_param_mm1_exp{iter} = param_est(mask) - ABW_true(mask);
    cell_param_md1_mu_exp{iter} = mu_est(mask) - bw_base;
    cell_param_md1_lambda_exp{iter} = lambda_est(mask) - (bw_base - ABW_true(mask));
end
save('res_param_1w.mat', 'cell_param_mm1_exp', 'cell_param_md1_mu_exp', 'cell_param_md1_lambda_exp');




function [idx, route] = route_find(m)
global SPR
if m <= 1
    route = SPR{m};
    idx = m;
else
    route = SPR{m-1};
    idx = m-1;
end
end

function path_bw = find_path_bw(route, bw)

path_bw = bw(route(1), route(2));
for i = 2 : length(route)-1 
    path_bw = min( path_bw, bw(route(i), route(i+1)) );
end

end

