function assessDiversity(Net)

%% Descriptions
% This script assesses SpaceDistinctness and SpaceDiversity of purified 
% files obtained via purifyInstIS.m for differen similairty treshhold,
% epsilon. The results wii be saved in a relevant file; 

% The array of similairty treshholds
epsilon = ["0"; "0.05"; "0.10"; "0.15"; "0.20"; "0.25"; "0.30"; "0.35"; "0.40"]; 

TblHeader_Diversity = {'Epsilon' 'InstNumb' 'SpaceDistinctness' 'SpaceDiversity'};

textHeader_Diversity = strjoin(TblHeader_Diversity, ',');

fid = fopen(sprintf('Diversity_Purified_%s.csv',Net),'w'); 
fprintf(fid,'%s\n',textHeader_Diversity);
fclose(fid);
clear fid* 

for i=1:length(epsilon)

eps = str2double(epsilon(i));

if eps==0
    Xbar = readtable(sprintf('%s.csv',Net)); % Read data from the original file;
else
    Xbar = readtable(sprintf('Purified_%s_Dist_%.2f.csv',Net, eps)); % Read data from the purified file corresponding wiht epsilon;
end

varlabels = Xbar.Properties.VariableNames;
isfeat = strncmpi(varlabels,'feature_',8);


X = Xbar{:,isfeat};

opts.norm.flag = true;
[X, model.norm] = autoNormalize_X(X, opts.norm);

%% The main loop to calculate the average distance of instances  


AvEucDist = zeros(length(X),1);
for ii=1:length(X)
    allDiff = bsxfun(@minus,X,X(ii,:));
    EucDist = cellfun(@norm,num2cell(allDiff,2));
    AvEucDist(ii) = sum(EucDist)/(length(X)-1);
end

SpaceDistinctness = sum(AvEucDist)/length(AvEucDist)

SpaceDiversity = sum(AvEucDist)
%}
%% Wrtie data on the table

Current_data_Diversity = [eps, length(X), SpaceDistinctness, SpaceDiversity];
fid = fopen(sprintf('Diversity_Purified_%s.csv',Net),'a');
fprintf(fid,'%f,%f,%f,%f\n', Current_data_Diversity);
fclose(fid);
clear fid* 
%%
clear Xbar;
clear X;
clear avDist;
end

