function bw_query(G, W, T, sT, tT, path_a, path_b, sb, tb, RNJ, ratio_BW)
global n_iab root

BT=find(T(sT,:)~=0); %BT: successor of sT
is_no_share = 1;

%% find the tau sharing only sT-BT with the target path
if W(sT, BT) > 0
    if BT > n_iab 
        if sT - BT == n_iab % last wireless hop
            return;
        end
        tau = BT;
    else
        [~,sp]=Dijkstra_source(T,BT);
        for i = 1 : n_iab
            ue = i + n_iab;
            if ue == tb
                continue;
            end
            route = sp{ue};
            if ~isempty(route)
                for j = 1:length(route) - 1
                    if W(route(j), route(j+1)) > 0
                        is_no_share = 0;
                        break;
                    end
                end
                if is_no_share > 0
                    tau = ue;
                    break;
                else
                    is_no_share = 1;
                end
            end
        end
    end
end

%% Read delay measurement under each ratio of BW
delay_hat = zeros(1, length(ratio_BW));
for i = 1 : length(ratio_BW)
    %% Write to config and run NS3
    %% Read delays on the target path
end

%% Estimate parameter based on lsqcurvefit

end