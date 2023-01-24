clear classes
global root n_iab SPR XPAttack_utils thr_burst tol pareto_shape enum_pareto_shape limit_ar
global enum_traffic param_probe param_background param_target
n_MC = 10;
nb = 5;
is_het_bg = 0;
if is_het_bg > 0
    load('para_hex_matlab_vbg.mat');
    
else
    load('para_hex_matlab.mat');
    load('res_param_1w.mat')
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

probe_pkt_size = 1000; % bytes
base_interval = 40000; % 40000ns = 200 mb/s
base_rate = 200;
target_rate = 100;
pareto_shape = enum_pareto_shape(end);

% cell_delay_no_attack = cell(1, n_MC);
% cell_delay_optimal = cell(1, n_MC);
% cell_delay_mm1_abs = cell(1, n_MC);
% cell_delay_mm1_rank = cell(1, n_MC);
% cell_delay_md1_abs = cell(1, n_MC);
% cell_delay_md1_rank = cell(1, n_MC);
% n_random = 5;
% cell_delay_random = cell(n_random, n_MC);
% cell_delay_even = cell(1, n_MC);
% list_total_rate = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7] * bw_base;

list_total_rate = [0.1:0.05:0.8] * bw_base;
cell_optimal_ar = cell(1, length(list_total_rate));
cell_MM1_rank_ar = cell(1, length(list_total_rate));
cell_MD1_rank_ar = cell(1, length(list_total_rate));
n_rd = 10;
cell_rd_rank_ar = cell(1, length(list_total_rate));
% cell_random2_rank_ar = cell(1, length(list_total_rate));
% cell_random3_rank_ar = cell(1, length(list_total_rate));
cell_even_rank_ar = cell(1, length(list_total_rate));

cell_optimal_iv = cell(1, length(list_total_rate));
cell_MM1_rank_iv = cell(1, length(list_total_rate));
cell_MD1_rank_iv = cell(1, length(list_total_rate));
n_rd = 10;
cell_rd_rank_iv = cell(1, length(list_total_rate));
% cell_random2_rank_ar = cell(1, length(list_total_rate));
% cell_random3_rank_ar = cell(1, length(list_total_rate));
cell_even_rank_iv = cell(1, length(list_total_rate));

limit_ar = 0.5 * bw_base;
[~,sp]=Dijkstra_source(T_true,root);
cell_W_total = cell(1, n_MC);
for i = 1 : n_MC
    W_all = zeros(length(W_true_cell{i}{1}), length(W_true_cell{i}{1}));
    for j = 1 : length(W_true_cell{i})
        W_all = W_all + W_true_cell{i}{j};
    end
    cell_W_total{i} = W_all;
end


for i = 1 : length(list_total_rate)
%     if i < length(list_total_rate)
%         continue;
%     end
    opt_rank_ar = zeros(n_iab, n_MC);
    MM1_rank_ar = zeros(n_iab, n_MC);
    MD1_rank_ar = zeros(n_iab, n_MC);
    rd_rank_ar = zeros(n_iab, n_rd);
%     rd2_rank_ar = zeros(n_iab, n_MC);
%     rd3_rank_ar = zeros(n_iab, n_MC);

%     opt_rank_iv = zeros(n_iab, n_MC);
%     MM1_rank_iv = zeros(n_iab, n_MC);
%     MD1_rank_iv = zeros(n_iab, n_MC);
%     rd_rank_iv = zeros(n_iab, n_rd);
    
    tr = list_total_rate(i);
    
    for iter = 1 : n_MC
        vec_n_shared = zeros(1, n_iab);
%         for u = 1 : 2*n_iab
%             for v = 1 : 2*n_iab
%                 
%             end
%         end
        if is_het_bg > 0
            back_ground_traffic = ceil(base_bg_ratio ./ cell_background_percent{iter} * base_bg_rate);
        else
            back_ground_traffic = 500 * ones(n_iab, n_iab);
        end
        ABW_true = zeros(n_iab*2, n_iab * 2);
        for u = 1 : length(cell_tb{iter})
            ABW_true = ABW_true + target_rate * (cnt_nTarget(iter, cell_tb{iter}(u)-n_iab)) * (W_true_cell{1,iter}{1,u} > 0);
        end
        ABW_true = bw_base - ABW_true - [back_ground_traffic, zeros(n_iab, n_iab); zeros(n_iab, 2*n_iab)];
        ABW_true(cell_W_total{iter} == 0) = 1e7;
%         opt_rank_ar(:, iter) = optimal_ar(ABW_true, tr, cell_W_total{iter}, T_true);
        opt_rank_ar(:, iter) = excess_opt(ABW_true, tr, cell_W_total{iter}, T_true, sp);
        ABW_est = cell_param_mm1_exp{iter};
        ABW_est(ABW_est <= 0) = 1e6;
%         MM1_rank_ar(:, iter) = est_rank_ar(ABW_est, ABW_true, tr, cell_W_total{iter}, T_true);
        MM1_rank_ar(:, iter) = excess_est_rank(ABW_est, ABW_true, tr, cell_W_total{iter}, T_true, sp);
        ABW_est = cell_param_md1_mu_exp{iter} - cell_param_md1_lambda_exp{iter};
        ABW_est(ABW_est <= 0) = 1e6;
        MD1_rank_ar(:, iter) = excess_est_rank(ABW_est, ABW_true, tr, cell_W_total{iter}, T_true, sp);
%         rd1_rank_ar(:, iter) = random_ar(tr, ABW_true, T_true);
%         rd2_rank_ar(:, iter) = random_ar(tr, ABW_true, T_true);
%         rd3_rank_ar(:, iter) = random_ar(tr, ABW_true, T_true);
    end
    for k = 1 : n_rd
        rd_rank_ar(:, k) = random_ar(tr, ABW_true, T_true);
    end
    opt_rank_iv = base_interval ./ (opt_rank_ar/base_rate);
    opt_rank_iv(isinf(opt_rank_iv)) = 0;
    cell_optimal_ar{i} = opt_rank_ar;
    cell_optimal_iv{i} = opt_rank_iv;
    
    MM1_rank_iv = base_interval ./ (MM1_rank_ar/base_rate);
    MM1_rank_iv(isinf(MM1_rank_iv)) = 0;
    cell_MM1_rank_ar{i} = MM1_rank_ar;
    cell_MM1_rank_iv{i} = MM1_rank_iv;
    
    MD1_rank_iv = base_interval ./ (MD1_rank_ar/base_rate);
    MD1_rank_iv(isinf(MD1_rank_iv)) = 0;
    cell_MD1_rank_ar{i} = MD1_rank_ar;
    cell_MD1_rank_iv{i} = MD1_rank_iv;
    
    rd_rank_iv = base_interval ./ (rd_rank_ar/base_rate);
    rd_rank_iv(isinf(rd_rank_iv)) = 0;
    cell_rd_rank_ar{i} = rd_rank_ar;
    cell_rd_rank_iv{i} = rd_rank_iv;
%     cell_random2_rank_ar{i} = rd2_rank_ar;
%     cell_random3_rank_ar{i} = rd3_rank_ar;
   
    cell_even_rank_ar{i} = tr / n_iab * ones(n_iab, n_MC);
    even_rank_iv = base_interval ./ (cell_even_rank_ar{i}/base_rate);
    even_rank_iv(isinf(even_rank_iv)) = 0;
    cell_even_rank_iv{i} = even_rank_iv;
end
if is_het_bg > 0
    save('Attack_Rate_het.mat', 'cell_optimal_ar', 'cell_MM1_rank_ar', 'cell_MD1_rank_ar', 'cell_rd_rank_ar', 'cell_even_rank_ar', 'list_total_rate');
    save('Attack_Interval_het.mat', 'cell_optimal_iv', 'cell_MM1_rank_iv', 'cell_MD1_rank_iv', 'cell_rd_rank_iv', 'cell_even_rank_iv', 'list_total_rate');
else
    save('Attack_Rate.mat', 'cell_optimal_ar', 'cell_MM1_rank_ar', 'cell_MD1_rank_ar', 'cell_rd_rank_ar', 'cell_even_rank_ar', 'list_total_rate');
    save('Attack_Interval.mat', 'cell_optimal_iv', 'cell_MM1_rank_iv', 'cell_MD1_rank_iv', 'cell_rd_rank_iv', 'cell_even_rank_iv', 'list_total_rate');
end
