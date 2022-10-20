function w2vec_stem(T, W, st, tbs)

global vec_stem_coordinate n_iab idx_stem vec_tau

BT = find(T(st, :) > 0);

list_tmp = find(vec_stem_coordinate(1,:) == st);

if BT > n_iab && BT-st == n_iab
    return;
elseif ismember(BT, tbs)
    return;
elseif BT > n_iab && BT-st ~= n_iab && W(st, BT) > 0
    if ~ismember( BT, vec_stem_coordinate(2,list_tmp) )
        vec_stem_coordinate(1, idx_stem) = st;
        vec_stem_coordinate(2, idx_stem) = BT;
        vec_tau(idx_stem) = BT;
        idx_stem = idx_stem + 1;
    end
    return;
elseif BT > n_iab && BT-st ~= n_iab && W(st, BT) == 0
    return;
end


if W(st, BT) > 0 && ~ismember( BT, vec_stem_coordinate(2,list_tmp) )
    [~,sp]=Dijkstra_source(T,BT);
    for i = 1 : length(sp)
        if ~isempty(sp{i})
            route = sp{i};
            for u = 1:length(route)-1
                if W(route(u), route(u+1)) > 0
                    break;
                end
                if u == length(route)-1
                    vec_tau(idx_stem) = route(end);
                    vec_stem_coordinate(1, idx_stem) = st;
                    vec_stem_coordinate(2, idx_stem) = BT;
                    idx_stem = idx_stem + 1;
                end
            end
        end
    end
end

end