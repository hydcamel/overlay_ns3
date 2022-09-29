function [ W ] = query( G, W, T, sT, tT, path_a, path_b, sb, tb, RNJ)
%W: same dimension with tree topology by RNJ
%T, sT, tT: substructure
%parameter,flag_model,ta,sb,tb: from the ground truth
% if flag_model==1
%     tol=0.17;
% end
% if flag_model==2
%     tol=0.024;
% end
%tol=0.05;
global tol n_iab root
BT=find(T(sT,:)~=0); %BT: successor of sT
is_three_targets = 0;
% BT = BT(BT <= n_iab);
% child_BT = find(T(BT,:)~=0);
% if ismember(BT,1:length(path_a)) %if BT is a destination
% if ~any(child_BT <= n_iab) %if BT is a destination BS
if BT > n_iab %if BT is a destination UE
    t1=BT;
    t2=BT;
else
    [~,sp]=Dijkstra_source(T,BT);
    %first choose a random destination
    for i=tT %traverse all destinations in substructure
        if ~isempty(sp{i}) && i~=tb %have a path from BT to i
            t1=i;
            temp=sp{i};
            sec_node=temp(2);
            break;
        end
    end
    for i=tT
        if ~isempty(sp{i}) && i~=tb %have a path from BT to i
            temp=sp{i};
            if temp(2)~=sec_node
                t2=i;
                is_three_targets = 1;
                break;
            end
        end
    end
end

if t1 == tb
    W(sT, tb) = 1;
    return;
end

if is_three_targets > 0
    [weight] = weight_calc([t1 - n_iab, t2 - n_iab], tb - n_iab);
else
    [weight] = weight_calc(t1 - n_iab, tb - n_iab);
end

if ~any(isnan(weight)) && ~ismember(-Inf,weight) && ~ismember(Inf,weight)
    rhos=weight(end);
else
    rhos=0;
end

if rhos>tol
    [~,sp]=Dijkstra_source(RNJ,root);
    temp_path=sp{sT};
    sum_w=0;
    for i=1:length(temp_path)-1
        sum_w=sum_w+W(temp_path(i),temp_path(i+1));
    end
    if rhos-sum_w>tol
        W(sT,BT)=rhos-sum_w;
    elseif sum_w>0
        return
    end
end
if t1==t2
    return
end

new_sT=BT;
[~,spd]=Dijkstra_source(T,BT);
for i=1:length(RNJ)-1
    if length(spd{i})==2
        % i is a successor of new_sT
%         if ismember(i,1:length(path_a)) %if i is a destination
        if i > n_iab %if i is a destination
            new_tT=i;
            new_T=zeros(length(RNJ),length(RNJ));
            new_T(new_sT,new_tT)=T(new_sT,new_tT);
            [W]=query( G,W,new_T,new_sT,new_tT, path_a,path_b,sb,tb,RNJ);
        else
            [spcost,sp]=Dijkstra_source(RNJ,i);
            new_tT=[];
            new_T=zeros(length(RNJ),length(RNJ));
            new_T(new_sT,i)=T(new_sT,i);
            for j=tT
                if spcost(j)>0 && spcost(j)<10^6 %if j is a child of i
                    new_tT=[new_tT,j];
                    temp=sp{j}; %path i to j
                    for k=1:length(temp)-1
                        new_T(temp(k),temp(k+1))=T(temp(k),temp(k+1));
                    end
                end
            end
            [W]=query( G,W,new_T,new_sT,new_tT, path_a,path_b,sb,tb,RNJ);     
        end
    end
end            

end

