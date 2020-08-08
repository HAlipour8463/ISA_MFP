function CVNND_Cmp(Net, epsilon, flag)

%% Descriptions
% CVNND stands for cofficient varioation of the nearest neighbor distances (NNDs).
% For each point in the instance space, this function calculates its distance
% form its nearest neighbor point. Then, claculates the coefficinet variation
% of all such distances. CVNND is used as a measure of distribution
% uniformuity (evenness) for non-classified samples. In fact, uniformity
% can be calculated as 1-CVNND.

% Depending on the flag, we might regard NND for both features and
% algorithm prefromnces (APs). In such cases, NNDs of features and APs are  
% calculated with different scales, which concide with the scalses that
% data sets are purified with the corresponding flags in
% purifyInstIS_Cmp(Net, epsilon, flag) function. 

% The arguments of this function are:
% Net: is the name of metadata file including the instances and their features
% and APs.
% epsilon: determines the treshhold of the similariy for which metadata are
% purified thorugh purifyInstIS_Cmp(Net, epsilon, flag) function.
% flag: determines which scale must be applied based on the similarity approach
% used in purifyInstIS_Cmp(Net, epsilon, flag) function as follows; 
% =========================================================================
% 'Ftr': similarity just based on the features;
% 'Ftr&AP': both features and APs with Euclidian distance;
% 'Ftr&Good': features with Euclidian distnace and APs based on the
% goodness;
% 'Ftr&AP&Good': features with Euclidian dustance and APs with both 
% Euclidian distance and goodness criterion;
% =========================================================================
% Code by: Hossein Alipour
%          School of Mathematics and Statistics
%          The University of Melbourne
%          Australia
%          2021
%          Email: h.alipour@unimelb.edu.au
 
%  Copyright: Hossein Alipour
%

%% Parameters 
% Samples of the array of similairty treshholds
% epsilon = [0.025, 0.05, 0.075, 0.10, 0.125, 0.15, 0.175, 0.20, 0.225, 0.25, ...
%     0.275, 0.30, 0.325, 0.35, 0.375, 0.40]; 
% epsilon = [0 0.050, 0.100, 0.150, 0.200, 0.250, 0.300, 0.350, 0.400]; 
% epsilon = [0 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40]; 

 if (strcmp(flag, 'Ftr&Good') || strcmp(flag, 'Ftr&AP&Good'))
     opts.perf.epsilon = 0.05; % this must coincide with that used in purifyInstIS_Cmp(Net, epsilon, flag);
 end
 
 TblHeader_CVNND = {'Epsilon' 'InstNumb' 'std' 'CV' 'Uniformity' };

textHeader_CVNND = strjoin(TblHeader_CVNND, ',');

fid = fopen(sprintf('CVNND_%s_Purified_%s.csv',flag, Net),'w'); 
fprintf(fid,'%s\n',textHeader_CVNND);
fclose(fid);
clear fid* 
 
 %%
for i=1:length(epsilon)

    switch flag
        case 'Ftr'
            Xbar = readtable(sprintf('Purified_%s.csv',flag, Net)); 
        case 'Ftr&AP'
            Xbar = readtable(sprintf('Purified_%s_%s_Dist_%.3f.csv',flag, Net, epsilon(i)));
        case 'Ftr&Good'
            Xbar = readtable(sprintf('Purified_%s_%s_G_%.2f_Dist_%.3f.csv',flag, Net,  opts.perf.epsilon, epsilon(i)));
        case 'Ftr&AP&Good'
            Xbar = readtable(sprintf('Purified_%s_%s_G_%.2f_Dist_%.3f.csv',flag, Net,  opts.perf.epsilon, epsilon(i)));
    end
% Xbar = readtable(sprintf('Purified_%s_%s_Dist_%.3f.csv',flag, Net, eps)); % Read data from the purified file corresponding wiht epsilon;


varlabels = Xbar.Properties.VariableNames;
isfeat = strncmpi(varlabels,'feature_',8);
isalgo = strncmpi(varlabels,'algo_',5);
X = Xbar{:,isfeat};
Y = Xbar{:,isalgo};

opts.norm.flag = true;
[X, Y, model.norm] = autoNormalize(X, Y, opts.norm);

switch flag
    case 'Ftr'
        Z = X;
    case 'Ftr&AP'
        Z = [X sqrt(4/17)*Y]; % factor sqrt(4/17) applies the different epsilons
%          regarded in purifying features and APs imlicitly. This equals to
%          epsilon1/spsilon2, where features are purified with epsilon1 and
%          APs are purified with epsilon2; here, 17 is the nubmer of features and
%          4 is the number of algorithms. 
    case 'Ftr&Good'
        Z = [X Y/(1 + opts.perf.epsilon)]; 
    case 'Ftr&AP&Good'
        Z = [X sqrt(4/17)*Y/(1 + opts.perf.epsilon)]; 
end

%% The loop to calculate CVNND and Uniformity (Evenness);

nearestDist = zeros(length(Z),1);
for ii=1:length(Z)
    Ztmp = Z;
    Ztmp(ii,:)=[];
    allDiff = bsxfun(@minus,Ztmp,Z(ii,:));
    EucDist = cellfun(@norm,num2cell(allDiff,2));
    nearestDist(ii) = min(EucDist);
end

CV = std(nearestDist)/mean(nearestDist)

Uniformity = 1- CV

%}
%% Wrtie data on the table

Current_data_CVNND = [eps, length(X), std(nearestDist), CV, Uniformity];
fid = fopen(sprintf('CVNND_%s_Purified_%s.csv',flag, Net),'a');
fprintf(fid,'%f,%f,%f,%f,%f\n', Current_data_CVNND);
fclose(fid);
clear fid* 
%%
clear Xbar;
clear X;
end
%{
%% Normlizing CVNND
Xbar = readtable(sprintf('CVNND_%s_Purified_%s.csv',flag, Net)); 
Nrm = [Xbar.InstNumb, Xbar.CV];
opts.norm.flag = true;
[Nrm, model.norm] = autoNormalize_X(Nrm, opts.norm);
CVNND_Nrm = Nrm(:,1).*Nrm(:,2);
Xbar.NrmCVNND = CVNND_Nrm;
writetable(Xbar,sprintf('CVNND_%s_Nrm_Purified_%s.csv',flag, Net));
%}


