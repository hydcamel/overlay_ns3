delays = readmatrix('delays_cnt.csv');
load('para2py_hex3.mat');
SPR = para2py.SPR;


thr_burst_delay = zeros(3, length(SPR));
for i = 1 : length(SPR)
    route = SPR{i};
    idx = find(delays(:, 2) == route(end) - 1);
    thr_burst_delay(3, i) = prctile(delays(idx,3:end), 95);
    thr_burst_delay(1, i) = route(1);
    thr_burst_delay(2, i) = route(end);
end

save(strcat('Thr_burst_pareto_hex3.mat'), 'thr_burst_delay');