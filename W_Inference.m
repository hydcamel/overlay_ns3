function [ W ] = W_Inference( G, W, T, path_a, path_b, tb, lfs )

global root n_iab

% W=zeros(length(T),length(T)); %W(i,j) means the weight share with edge (i,j) in T
%Query
W=query(G, W, T, root, lfs, path_a, path_b, length(path_a)+2, tb, T);

flag=1;
while flag
    flag=0;
%     for i=length(path_a)+1:length(T)-1 %path s->i, i->left, i->right cannot have positive weights at the same time
    for i = 1 : size(T, 1) %path s->i, i->left, i->right cannot have positive weights at the same time
        if i > n_iab || i == root
            continue;
        end
        [~,sp]=Dijkstra_source(T, root);
        sp=sp{i};
        sum_w=0;
        for j=1:length(sp)-1
            sum_w=sum_w+W(sp(j),sp(j+1));
        end      
        if sum_w>0 %if path s->i have positive weight
            [~,sp]=Dijkstra_source(T,i);
            candidate_next=[];
            max_sum=[];
%             for j=1:length(path_a)
            for j=1:n_iab
                sum_w=0;
%                 tmp=sp{j};
                tmp=sp{j + n_iab};
                if ~isempty(tmp)
                    for k=1:length(tmp)-1
                        sum_w=sum_w+W(tmp(k),tmp(k+1));
                    end
                end
                if sum_w>0
                    if ~ismember(tmp(2),candidate_next)
                        candidate_next=[candidate_next,tmp(2)];
                        max_sum=[max_sum,sum_w];
                    else
                        index=find(candidate_next==tmp(2));
                        if sum_w>max_sum(index)
                            max_sum(index)=sum_w;
                        end
                    end
                end
            end
            if length(candidate_next)>1 %only leave the branch with biggest sharing parts
                [~,index]=max(max_sum);
                for j=1:length(candidate_next)
                    if j~=index
                        W(i,candidate_next(j))=0;
                    end
%                     if ~ismember(candidate_next(j),1:length(path_a))
                    if candidate_next(j) < n_iab
                        [~,sp]=Dijkstra_source(T,candidate_next(j));
                        for k=1:length(sp)
                            tmp=sp{k};
                            if ~isempty(tmp)
                                for l=1:length(tmp)-1
                                    W(tmp(l),tmp(l+1))=0;
                                end
                            end
                        end
                    end
                end
                flag=1;
            end
        end
    end
end
end