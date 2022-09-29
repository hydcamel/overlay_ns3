function [weight] = weight_calc(ta, tb)
global SPR thr_burst XPAttack_utils

%% run one round of simulation
n_old_E = length(ta)+1;
E_cur_idxlist = zeros(1, n_old_E);
idx = 1;
for i = 1 : length(SPR)
    route = SPR{i};
    if ismember(route(end), ta) || route(end) == tb
        E_cur_idxlist(idx) = i;
        idx = idx + 1;
    end
end

%% Run simulation through python
para_probe = py.dict(pyargs('E_cur_idxlist',E_cur_idxlist));
%% Prepare Python Env
if count(py.sys.path,'') == 0
    insert(py.sys.path,int32(0),'');
end
py.XPAttack_utils.run_simulation(para_probe);

%% Binarization
delays_raw = readmatrix('delays_cnt.csv');
tmp = sum(delays_raw > 0, 1);
tmp(1:2) = size(delays_raw,1) + 5;
n_effective = sum( tmp >= size(delays_raw,1) ) - 2;
delays_raw = delays_raw(:, 1:n_effective+2);
index_permute = zeros(1, n_old_E);

for i = 1 : size(delays_raw,1)
%     s = delays_raw(i,1) + 1; % pay attention to index alignment
    t = delays_raw(i,2) + 1;
    for j = 1 : size(delays_raw,1)
        route = SPR{E_cur_idxlist(j)};
        if route(end) == t
            index_permute(j) = i;
            delays_raw(i, 3:end) = delays_raw(i, 3:end) > thr_burst(3, E_cur_idxlist(j));
            break;
        end
    end
end
delays_raw = delays_raw(index_permute, 3:end);
% delays_raw = delays_raw > thr_burst;

n_samples = size(delays_raw, 2);
if n_old_E == 3
    %% Calc rho
    rho_hat = zeros(7, 1);
    %% 1, 2, 3
    rho_hat(1:3) = sum(delays_raw(1:3, :), 2) ./ n_samples;
    %% 12, 13, 23
    idx = 4;
    for i = 1 : 2
        for j = i+1 : 3
            rho_hat(idx) = sum(delays_raw(i, :) & delays_raw(j, :), 2) ./ n_samples;
            idx = idx + 1;
        end
    end
    %% 123
    rho_hat(7) = sum(delays_raw(1, :) & delays_raw(2, :) & delays_raw(3, :), 2) ./ n_samples;

    %% Calc weights
    A=[1,0,0,1,1,0,1;0,1,0,1,0,1,1;0,0,1,0,1,1,1;1,1,0,1,1,1,1;1,0,1,1,1,1,1;0,1,1,1,1,1,1;1,1,1,1,1,1,1];
    weight = lsqr(A, rho_hat);
    % weight = (A' * A) \ (A' * rho_hat);
elseif n_old_E == 2
    %% Calc rho
    rho_hat = zeros(3, 1);
    %% 1, 2
    rho_hat(1:2) = sum(delays_raw(1:2, :), 2) ./ n_samples;
    %% 12
    rho_hat(3) = sum(delays_raw(1, :) & delays_raw(2, :), 2) ./ n_samples;
    A=[1,0,1;0,1,1;1,1,1];
    weight = lsqr(A, rho_hat);
end