function [acc, list_miss, list_fa] = inference_acc(W_true,W)

% acc=zeros(1,length(W_true));
accumulate_diff = zeros(1,length(W_true));
accumulate_total = zeros(1,length(W_true));
list_miss = [];
list_fa = [];
for i=1:length(W_true)
    g1=W_true{i};
    g2=W{i};
    sum_diff=0;
    for j=1:length(g1)
        for k=1:length(g1)
            if g1(j,k)==0 && g2(j,k)>0
                list_fa = [list_fa; [i, j, k]];
                sum_diff=sum_diff+1;
            end
            if g1(j,k)>0 && g2(j,k)==0
                list_miss = [list_miss; [i, j, k]];
                sum_diff=sum_diff+1;
            end
        end
    end
    accumulate_diff(i) = sum_diff;
    accumulate_total(i) = (length(g1)-1);
%     acc(i)=1-sum_diff/(length(g1)-1);
end
acc = 1 - sum(accumulate_diff) / sum(accumulate_total);

