function purifyInstIS_Cmp(Net, epsilon, flag)
%% Description
% This script purifies benchmarks problems based on the similarity among them.
% The similarity of two benchmark problems is defined as the Euclidian
% distnace between their features.
% This script regards the similarity of tow benchmarks based on the
% similarity of the algorithm performances (APs) on them too; the simiarity of
% APs can be calculated as th Euclidian distance among the APs or as the
% similarity of their goodness in % saolving the similar benchmarks.
% If two benchmarks are similar, then it checks the similarity among
% algorithm performances on these benchmarks.

% The arguments of this function are:
% Net: is the name of metadata file including the instances and their features
% and APs.
% epsilon: determines the treshhold of the similarity.
% flag: determines which similarity approach must be applied as follows; 
% =========================================================================
% 'Ftr': similarity just based on the features
% 'Ftr&AP': both features and APs with Euclidian distance
% 'Ftr&Good': features with Euclidian distnace and APs based on the goodness
% 'Ftr&AP&Good': features with Euclidian dustance and APs
% with both Euclidian distance and goodness criterion
% =========================================================================
% In the cases of goodness criterion, we must define a differen threshold
% for the goddness of APs. This is ture about the
% second case too, but since the number of APs are less than the number of
% features, they are purified with different thresholds simlicitly;

% =========================================================================
% Code by: Hossein Alipour
%          School of Mathematics and Statistics
%          The University of Melbourne
%          Australia
%          2021
%          Email: h.alipour@unimelb.edu.au
 
%  Copyright: Hossein Alipour
%

%% Main loop

for k = 1:length(epsilon)
    
    Xbar = readtable(sprintf('%s.csv',Net));
    
    % Swap the rows of Xbar randomly to have a purification based on the random
    % perturbation. Deactive this command if you have a preferable sort of purification.
    
    % Y = Xbar(randperm(size(Xbar, 1)), :);
    %
    % Xbar = Y;
    
    varlabels = Xbar.Properties.VariableNames;
    isfeat = strncmpi(varlabels,'feature_',8);
    isalgo = strncmpi(varlabels,'algo_',5);
    X = Xbar{:,isfeat};
    Y = Xbar{:,isalgo};
    
    %--------------------------------------------------------------------
    %     X = Xbar{:,isfeat};
    %     Y = Xbar{:,isalgo};
    
    opts.norm.flag = true;
    [X, Y, model.norm] = autoNormalize(X, Y, opts.norm);
    
    toPreclude = false((size(X,1)),1);
    Tprcl = 0; % counter of the precluded instances
    
    ViC_D = 0; % A counter of the violation from Euclidian distance for APs
    ViC_Good = 0; % A counter of the violation from the goodness criterion for APs
    %% Parameters for purifying APs based on the goodnes criterion
    
%    flag
        if (strcmp(flag, 'Ftr&Good') || strcmp(flag, 'Ftr&AP&Good'))
            
            opts.perf.MaxPerf = false;              % True if Y is a performance measure to maximize, False if it is a cost measure to minimise.
            opts.perf.AbsPerf = false;               % True if an absolute performance measure, False if a relative performance measure
            opts.perf.epsilon = 0.05;               % Threshold of good performance
            
            % -------------------------------------------------------------------------
            if opts.perf.MaxPerf
                Yaux = Y;
                Yaux(isnan(Yaux)) = -Inf;
                [rankPerf,rankAlgo] = sort(Yaux,2,'descend');
                bestPerformace = rankPerf(:,1);
                P = rankAlgo(:,1);
                if opts.perf.AbsPerf
                    Ybin = Yaux>=opts.perf.epsilon;
                else
                    Ybin = bsxfun(@ge,Yaux,(1-opts.perf.epsilon).*bestPerformace); % One is good, zero is bad
                end
            else
                Yaux = Y;
                Yaux(isnan(Yaux)) = Inf;
                [rankPerf,rankAlgo] = sort(Yaux,2,'ascend');
                bestPerformace = rankPerf(:,1);
                P = rankAlgo(:,1);
                if opts.perf.AbsPerf
                    Ybin = Yaux<=opts.perf.epsilon;
                    msg = [msg 'less than ' num2str(opts.perf.epsilon)];
                else
                    Ybin = bsxfun(@le,Yaux,(1+opts.perf.epsilon).*bestPerformace);
                end
            end
    end
    
    
    
    %% The main nested loop to preclude the similar instances according to a
    %  given tolearnce epsilon, preferably from the interval (0, 1).
    
    for i=1:size(X,1)
        if (~toPreclude(i))
            for j=i+1:size(X,1)
                if (~toPreclude(j))
                    dist = sqrt(sum((X(i,:) - X(j,:)) .^ 2)); % Euclidean distance
                    if (dist <= epsilon(k))
                        switch flag
                            case 'Ftr'
                                toPreclude(j) = true;
                                Tprcl = Tprcl +1;
                            case 'Ftr&AP'
                                dist_AP = sqrt(sum((Y(i,:) - Y(j,:)) .^ 2));
                                if (dist_AP <= sqrt(4/17)*epsilon(k)*(1+0.26)) % 0.26 is the value of CPUN
                                    toPreclude(j) = true;
                                    Tprcl = Tprcl +1;
                                else
                                    ViC_D = ViC_D + 1;
                                end
                            case 'Ftr&Good'
                                if (Ybin(i,:) == Ybin(j,:))
                                    toPreclude(j) = true;
                                    Tprcl = Tprcl +1;
                                else
                                    ViC_Good  = ViC_Good  + 1;
                                end
                            case 'Ftr&AP&Good'
                                if (Ybin(i,:) == Ybin(j,:))
                                    dist_AP = sqrt(sum((Y(i,:) - Y(j,:)) .^ 2));
                                    if (dist_AP <= sqrt(4/17)*epsilon(k)*(1+0.26)) % epsilon could be different from that given in the function.
                                        toPreclude(j) = true;
                                        Tprcl = Tprcl +1;
                                    else
                                        ViC_D = ViC_D + 1;
                                    end
                                else
                                        ViC_Good  = ViC_Good + 1;
                                end
                            end
                        end
                    end
                end
            end
    end
        
    Tprcl; 
    ViC_D ;
    ViC_Good; 
        
        
        %  Xbar = sortrows(Xbar); % This command is useful if you want to apply a random purification.
        
        PurifiedInst = Xbar;
        PurifiedInst(toPreclude,:) = [];
        PrecludedInst = Xbar;
        PrecludedInst(~toPreclude,:) = [];
        
        switch flag
            case 'Ftr'
                writetable(PurifiedInst,sprintf('Purified2_%s_%s_Dist_%.3f.csv',flag, Net, epsilon(k)));
                writetable(PrecludedInst,sprintf('Precluded2_%s_%s_Dist_%.3f.csv',flag, Net, epsilon(k)));
            case 'Ftr&AP'
                writetable(PurifiedInst,sprintf('Purified2_%s_%s_Dist_%.3f.csv',flag, Net, epsilon(k)));
                writetable(PrecludedInst,sprintf('Precluded2_%s_%s_Dist_%.3f.csv',flag, Net, epsilon(k)));
            case 'Ftr&Good'
                writetable(PurifiedInst,sprintf('Purified2_%s_%s_G_%.2f_Dist_%.3f.csv',flag, Net,  opts.perf.epsilon, epsilon(k)));
                writetable(PrecludedInst,sprintf('Precluded2_%s_%s_G_%.2f_Dist_%.3f.csv',flag, Net,  opts.perf.epsilon, epsilon(k)));
            case 'Ftr&AP&Good'
                writetable(PurifiedInst,sprintf('Purified2_%s_%s_G_%.2f_Dist_%.3f.csv',flag, Net,  opts.perf.epsilon, epsilon(k)));
                writetable(PrecludedInst,sprintf('Precluded2_%s_%s_G_%.2f_Dist_%.3f.csv',flag, Net,  opts.perf.epsilon, epsilon(k)));
        end
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
        clear Y;
        clear Ybin;
        clear toPreclude;
        fclose('all');
    end
    
