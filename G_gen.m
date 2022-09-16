function [ Adj, D, Adj_full, sa, sb, ta, tb, path_a, path_b, DG, SPR, node_xval, node_yval ] = G_gen(G_type)

%% G_type = 0:Hex
if G_type == 0 %Hex: nodes on axis and 4 symmetric areas
    n_edge = 3; % maximal number of nodes in each row, i.e, number of nodes in each edge
    d = 200; % interval = 200m
    d_1 = d*sqrt(3);
    node_xAxis_xval = -floor((n_edge-1)/2)*d_1: d_1: floor((n_edge-1)/2)*d_1;
    node_xAxis_yval = zeros(1,length(node_xAxis_xval));
    node_yAxis_yval = -(n_edge-1)*d: d:(n_edge-1)*d;
    node_yAxis_xval = zeros(1,2*(n_edge-1));
    node_yAxis_yval(n_edge) = []; % tick out the repeated original node
    
    n_area_nodes = n_edge^2;
    tmp_area_xval = zeros(1, n_area_nodes);
    tmp_area_yval = zeros(1, n_area_nodes);
    idx = 1;
    n_interval = 2*(n_edge-1)-1;
    for i = 1 : n_edge-1
        for j = 1 : 2 : n_interval
            tmp_area_xval(1, idx) = d_1 / 2 * i;
            tmp_area_yval(1, idx) = d/2 * (j+1) - mod(i,2)* d/2;
            idx = idx + 1;
        end
        n_interval = n_interval - 1;
    end
    n_area_nodes = sum(tmp_area_xval ~= 0);
    
    node_areas_xval = zeros(4, n_area_nodes); % quadrant 1 2 3 4
    node_areas_yval = zeros(4, n_area_nodes); % quadrant 1 2 3 4
    % quadrant 1
    node_areas_yval(1, :) = tmp_area_yval(1, 1:n_area_nodes);
    node_areas_xval(1, :) = tmp_area_xval(1, 1:n_area_nodes);
    % quadrant 2
    node_areas_yval(2, :) = node_areas_yval(1, :);
    node_areas_xval(2, :) = -node_areas_xval(1, :);
    % quadrant 3
    node_areas_yval(3, :) = -node_areas_yval(1, :);
    node_areas_xval(3, :) = -node_areas_xval(1, :);
    % quadrant 4
    node_areas_yval(4, :) = -node_areas_yval(1, :);
    node_areas_xval(4, :) = node_areas_xval(1, :);
    node_xval = [node_xAxis_xval, node_yAxis_xval, node_areas_xval(1, :), node_areas_xval(2, :), node_areas_xval(3, :), node_areas_xval(4, :)];
    node_yval = [node_xAxis_yval, node_yAxis_yval, node_areas_yval(1, :), node_areas_yval(2, :), node_areas_yval(3, :), node_areas_yval(4, :)];
    
    Adj = zeros(length(node_xval), length(node_xval));
    D = zeros(length(node_xval), length(node_xval)^2);
    idx = 1;
    for i = 1 : length(node_xval)
        %% upper child
        idx_y_tmp = find(node_yval == node_yval(i) + d);
        for j = 1 : length(idx_y_tmp)
            if node_xval(idx_y_tmp(j)) == node_xval(i)
                Adj(i, idx_y_tmp(j)) = 1;
                D(i, idx) = 1; D(idx_y_tmp(j), idx) = -1; idx = idx + 1;
                break;
            end
        end
        %% lower child (left and right)
        idx_y_tmp = find(node_yval == node_yval(i) - d/2);
        for j = 1 : length(idx_y_tmp)
            if node_xval(idx_y_tmp(j)) == node_xval(i)-d_1/2 || node_xval(idx_y_tmp(j)) == node_xval(i)+d_1/2
                Adj(i, idx_y_tmp(j)) = 1;
                D(i, idx) = 1; D(idx_y_tmp(j), idx) = -1; idx = idx + 1;
            end
        end
    end
    D = D(:, 1:idx-1);
    
    n_iab = length(Adj);
    Adj_full = [Adj, eye(n_iab); zeros(n_iab, 2*n_iab)];
%     for i = 1 : n_iab
%         Adj_full(i, i+n_iab) = 1;
%     end
    
    DG = digraph(Adj);
    SPR = shortestpathtree(DG,2,'OutputForm','cell');
    sa = 2; sb = 2;
    ta = 1:length(Adj);
    tb = 3:10;
    path_a = SPR; 
    path_b = SPR(tb,1);
    ta = ta + n_iab; tb = tb + n_iab; 
end

end