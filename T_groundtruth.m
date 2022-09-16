function [ T,W,para ] = T_groundtruth( G,path_a,path_b,parameter)
%return the groud truth's tree topology T, sharing matrix W
%1:length(path_a): destinations, length(T): source

%pick nodes in slice A and generate a new graph H
H=zeros(length(G),length(G));
for i=1:length(path_a)
    tmp=path_a{i};
    for j=1:length(tmp)-1
        H(tmp(j),tmp(j+1))=G(tmp(j),tmp(j+1));
    end
end
%% Transform H into logical graph such that there is no inmeasurable nodes (1-indegree and 1-outdegree)
P=H;
J=digraph(H);
to_delete=find(indegree(J)==1 & outdegree(J)==1);
for i=length(to_delete):-1:1
    tmp=to_delete(i);
    a=find(H(:,tmp)>0); % nodes to tmp (a, tmp)
    b=find(H(tmp,:)>0); % nodes from tmp (tmp, b)
    H(a,b)=H(a,tmp)+H(tmp,b);
    H(a,tmp)=0;
    H(tmp,b)=0;
end

%generate a matrix X which records (a,b) belongs to which link
X=cell(length(G),length(G));
for i=1:length(path_a)
    tmp=path_a{i};
    for j=1:length(tmp)-1
        for m=1:length(G)
            for n=1:length(G)
                if H(m,n)>0
                    [spcost1, ~] = Dijkstra_source(P, m);
                    [spcost2, ~] = Dijkstra_source(P, tmp(j+1));
                    if (m==tmp(j)||(spcost1(tmp(j))<Inf&&spcost1(tmp(j))>0)) && ((n==tmp(j+1)||(spcost2(n)>0&&spcost2(n)<Inf)))
                        X{tmp(j),tmp(j+1)}=[m,n];
                    end
                end
            end
        end
    end
end

%generate V, which is the middle step for W
V=cell(1,length(path_b));
for i=1:length(path_b)
    tmp=path_b{i};
    F=zeros(length(G),length(G));
    for j=1:length(tmp)-1
        if X{tmp(j),tmp(j+1)}
            temp=X{tmp(j),tmp(j+1)};
            F(temp(1),temp(2))=F(temp(1),temp(2))+G(tmp(j),tmp(j+1));
        end
    end
    V{i}=F;
end

%generate Q, which is the middle step for para
Q=cell(1,length(path_b));
for i=1:length(path_b)
    tmp=path_b{i};
    lambda=zeros(length(G),length(G));
    mu=zeros(length(G),length(G));
    for j=1:length(tmp)-1
        if X{tmp(j),tmp(j+1)}
            temp=X{tmp(j),tmp(j+1)};
            mu(temp(1),temp(2))=parameter.mu(tmp(j),tmp(j+1)); 
            lambda(temp(1),temp(2))=parameter.lambda(tmp(j),tmp(j+1));   
        end
    end
    Q{i}.mu=mu; 
    Q{i}.lambda=lambda;     
end


number_index=zeros(1,length(G));
index=1;
for i=2:length(G)
    if sum(H(i,:))+sum(H(:,i))
        number_index(i)=index;
        index=index+1;
    end
end
number_index(1)=index;
number_index = 1:length(G);
index = length(G);

T=zeros(index,index);
for i=1:length(H)
    for j=1:length(H)
        if H(i,j)>0
            T(number_index(i),number_index(j))=H(i,j);
        end
    end
end

W=cell(1,length(path_b));
for i=1:length(path_b)
    tmp=V{i};
    F=zeros(index,index);
    for j=1:length(tmp)
        for k=1:length(tmp)
            if tmp(j,k)>0
                F(number_index(j),number_index(k))=tmp(j,k);
            end
        end
    end
    W{i}=F;
end

para=cell(1,length(path_b));
for i=1:length(path_b)
    tmp=Q{i};
    lambda=zeros(index,index);
    mu=zeros(index,index);
    for j=1:length(tmp.mu)
        for k=1:length(tmp.mu)
            if tmp.mu(j,k)>0
                lambda(number_index(j),number_index(k))=tmp.lambda(j,k);
                mu(number_index(j),number_index(k))=tmp.mu(j,k);
            end
        end
    end
    para{i}.lambda=lambda;
    para{i}.mu=mu;     
end

end

