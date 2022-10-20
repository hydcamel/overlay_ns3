function tau = find_tau(T, BT, W) 
%% return UE ID

global n_iab

[~,sp]=Dijkstra_source(T,BT);
for i = 1+n_iab : 2*n_iab
    if ~isempty(sp{i})
        route = sp{i};
        is_break = 1;
        for u = 1:length(route)-1
            if W(route(u), route(u+1)) > 0
                is_break = 0;
                break;
            end
        end
        if is_break == 1
            tau = i;
            return
        end
    end
end