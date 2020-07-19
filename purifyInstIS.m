function purifyInstIS(Net, epsilon)

%% Description
% This script purifies benchmarks problems based on the similarity among them. 
% The similarity of two benchmark problems is defined as the Euclidian distnace between their features. 
% The arguments of this function are:
% Net: is the name of metadata file including the instances and their features
% epsilon: determines the treshhold of the similarity. 


Xbar = readtable(sprintf('%s.csv',Net));

% Swap the rows of Xbar randomly to have a purification based on the random
% perturbation. Deactive this command if you have a preferable sort of purification. 

% Xbar = Xbar(randperm(size(Xbar, 1)), :);


varlabels = Xbar.Properties.VariableNames;
isfeat = strncmpi(varlabels,'feature_',8);

X = Xbar{:,isfeat};

opts.norm.flag = true;
[X, model.norm] = autoNormalize_X(X, opts.norm);

toPreclude = false((size(X,1)),1);

%% The main nested loop to preclude the similar instances according to a 
%  given tolearnce epsilon, preferably from the interval (0, 1).

% epsilon = 0.05;

for i=1:size(X,1)
    if (~toPreclude(i))
        for j=i+1:size(X,1)
            if (~toPreclude(j))
                dist = sqrt(sum((X(i,:) - X(j,:)) .^ 2)); % Euclidean distance
                if (dist <= epsilon)
                    toPreclude(j) = true;
                end
            end
        end
    end
    
end

 Xbar = sortrows(Xbar); % This command is useful if you want to apply a random purification.

PurifiedInst = Xbar;
PurifiedInst(toPreclude,:) = [];
PrecludedInst = Xbar; 
PrecludedInst(~toPreclude,:) = [];

writetable(PurifiedInst,sprintf('Purified_%s_Dist_%.2f.csv',Net, epsilon));
writetable(PrecludedInst,sprintf('Precluded_%s_Dist_%.2f.csv',Net, epsilon));


%% Activate the following lines if there is more than one network and you want to 
% collect all of them in a Purified_metadata file, which created by the main script, ISA.m;
%  Activvate the relevant commands in ISA.m is you want to create these metada files. 

%{
PurifiedMainTbl = readtable(sprintf('Purified_metadata_Dist_%.2f.csv', epsilon));
PurifiedMainTbl=[PurifiedMainTbl; PurifiedInst];
writetable(PurifiedMainTbl, sprintf('Purified_metadata_Dist_%.2f.csv', epsilon));

PrecludedMainTbl = readtable(sprintf('Precluded_metadata_Dist_%.2f.csv', epsilon));
PrecludedMainTbl=[PrecludedMainTbl; PrecludedInst];
writetable(PrecludedMainTbl, sprintf('Precluded_metadata_Dist_%.2f.csv', epsilon));
%}
%% 
clear Xbar;
clear X;
clear toPreclude;
fclose('all');


