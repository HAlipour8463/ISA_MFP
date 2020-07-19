%% Descriptions
% This script calls different functions to provide a metadata file, which
% then used by the ISA toolkit provided by the MATILDA team: 
% https://matilda.unimelb.edu.au/matilda/showMobileHomePage
% Some commands must be activated/deactivated based on our purpose. The
% functions provide some tools to do an exploratory data analysis as the
% pre-processing data analysis of ISA. 
%% Arrays of networks' names and algorithms' names
% The argument must be select according to the following string arrays:
clear
clc

% NetName=[ "AC_F_Rand"; "AC_F_Spec"; "AC_S_Spec";  "Netgen"; "Transit_Grid_1way"; "Transit_Grid_2way"];
% NetName=[  "Random"; "Wash_GldBad"; "Wash_DinicBad"; "Wash_Cher";  "Vision_Pool"; "Genrmf_Moderate"; "Wash_SqrMesh" ]
%"Wash_DinicBad"; "Vision_Pool"; "Genrmf_Moderate"; "Wash_ExpoLine"
%NetName=["AC_F_Rand"; "AC_F_Spec"; "AC_S_Spec";  "Netgen"; "Transit_Grid_1way"; "Transit_Grid_2way"; ...
  %  "Wash_Mesh"; "Random"; "Wash_GldBad";  "Wash_Cher"; "Wash_SqrMesh"; "Wash_ExpoLine"; "Wash_DExpoLine"; ...
% ["Wash_Line_Sparse_Pool"; "Wash_RLG_Pool"; "Wash_RLG2_Pool"]
%   NetName = ["AC_results_100k"; "AC_results_1m"; "AC_results_10m"]
% AlgName=["push"; "Pseudo"];
%{
 NetName=["Features_5idx_AC"; "Features_5idx_ACFRand"; "Features_5idx_ACFSpec"; ...
     "Features_5idx_ACSSpec"; "Features_5idx_Genrmf_Long"; "Features_5idx_Genrmf_Wide"; ...
     "Features_5idx_Netgen"; "Features_5idx_TG"; "Features_5idx_TG1way"; ...
     "Features_5idx_TG2way"; "Features_5idx_Wash_Cher";  "Features_5idx_Wash_ExpoLine"; ...
     "Features_5idx_Wash_GldBad"; "Features_5idx_WashLineSparse"; "Features_5idx_WashLineModerate";  ...
     "Features_5idx_WashRLG2HiCap"; "Features_5idx_WashRLG2LoCap"; "Features_5idx_WashRLGLong";  ...
     "Features_5idx_WashRLGWide"; "Features_5idx_WashSquareModerate"; "Features_5idx_WashSquareSparse"; 
     "Features_AK";  "Features_Vision_Pool"; "Features_Genrmf_Moderate" ];
%}
%  NetName=[ "WashGldBad"; "WashLine"; "AC"; "AK"; "WashSquare"; "TG"; "WashExpoLine"; ...
%      "Vision"; "Netgen"; "RMF"; "WashRLG"; "WashCher"];

% StatName = ["mean"; "std"; "var"]; % The array of statistical quantities applied to the original results



%% Load different parameters including different sizes and capacities of each network
%{
Benchmark_nodes;
Benchmark_Cap;
Benchmark_Cap_ak
Benchmark_nodes_ak
%}


%% Define the headers to create the table and collect data from different sources

%---------------------Basic Factors and Features----------------------------------------
%{
TblHeader_BasicFactors = {'Instances' 'Source' 'Nodes' 'Arcs' 'NetCap' 'maxcap'	'mincap' ...
    'SrcAAC' 'SnkAAC' 'Src_degree'	'Snk_degree' 'StDAC' 'NmBdCap' 'NmGdCap' 'PotNetExcess'	'PotNetDeficit'};

textHeader_BasicFactors = strjoin(TblHeader_BasicFactors, ',');
%}
%{
TblHeader = {'Instances' 'Source' 'Push0_Max' 'Push_max' 'PAR_Max' 'PAR0_Max' 'P2R_Max' 'P2R0_Max' 'Pseudo_Max' ...
    'algo_Push' 'algo_PAR' 'algo_P2R' 'algo_Pseudo' 'x_PushROC' 'x_PARROC' 'x_P2RROC' 'x_PseudoROC' ...
    'feature_Nodes' 'feature_Arcs' 'feature_Density' 'feature_CapDens' 'feature_AvNdDgr' ...
    'feature_TrivialEtraFlw' 'feature_PrcNetPotPth' 'feature_EstMinPrcDepFlw' ...
    'feature_PerBdCap' 'feature_PerGdCap' 'feature_StDAC' 'feature_OutlMxCap' ...
    'feature_OutlMnCap' 'feature_CapAvNddg' 'feature_RlCapAvNddg'  ...
    'feature_RlPotNetExcess' 'feature_AVRlPotNetExcess'};

textHeader = strjoin(TblHeader, ',');

%}


%---------------------------Standard deviations----------------------------
%{
StdTblHeader_BasicFactors = {'Instances' 'Source' 'Nodes' 'RSDArcs' 'MeanArcs' 'RSDNetCap' 'MeanNetCap' 'RSDmaxcap'	'Meanmaxcap' 'RSDmincap' 'Meanmincap' ...
    'RSDSrcAAC' 'MeanSrcAAC' 'RSDSnkAAC' 'MeanSnkAAC' 'RSDSrc_degree' 'MeanSrc_degree'	'RSDSnk_degree' 'MeanSnk_degree' ...
    'RSDStDAC'  'MeanStDAC' 'RSDNmBdCap' 'MeanNmBdCap' 'RSDNmGdCap' 'MeanNmGdCap' 'RSDPotNetExcess' 'MeanPotNetExcess'	'RSDPotNetDeficit' 'MeanPotNetDeficit' 	};

StdtextHeader_BasicFactors = strjoin(StdTblHeader_BasicFactors, ',');

StdTblHeader = {'Instances' 'Source' 'RSDPush0_Max' 'MeanPush0_Max' 'RSDPush_max' 'MeanPush_max' 'RSDPAR_Max' 'MeanPAR_Max' 'RSDPAR0_Max' 'MeanPAR0_Max' ...
    'RSDP2R_Max' 'MeanP2R_Max' 'RSDP2R0_Max' 'MeanP2R0_Max' 'RSDPseudo_Max' 'MeanPseudo_Max'  ...
    'algo_RSDPush' 'algo_MeanPush' 'algo_RSDPAR' 'algo_MeanPAR' 'algo_RSDP2R' 'algo_MeanP2R' ...
    'algo_RSDPseudo' 'algo_MeanPseudo' 'x_RSDPushROC'  'x_MeanPushROC' 'x_RSDPARROC' 'x_MeanPARROC' 'x_RSDP2RROC' 'x_MeanP2RROC' 'x_RSDPseudoROC' 'x_MeanPseudoROC' ...
    'feature_Nodes' 'feature_RSDArcs' 'feature_MeanArcs'  'feature_RSDDensity' 'feature_MeanDensity' 'feature_RSDCapDens'  'feature_MeanCapDens' ...
    'feature_RSDAvNdDgr' 'feature_MeanAvNdDgr' 'feature_RSDTrivialEtraFlw' 'feature_MeanTrivialEtraFlw' 'feature_RSDPrcNetPotPth'  'feature_MeanPrcNetPotPth' ...
    'feature_RSDEstMinPrcDepFlw' 'feature_MeanEstMinPrcDepFlw' 'feature_RSDPerBdCap' 'feature_MeanPerBdCap' 'feature_RSDPerGdCap' 'feature_MeanPerGdCap' ...
    'feature_RSDStDAC'   'feature_MeanStDAC' 'feature_RSDOutlMxCap' 'feature_MeanOutlMxCap' 'feature_RSDOutlMnCap'  'feature_MeanOutlMnCap' ...
    'feature_RSDCapAvNddg'  'feature_MeanCapAvNddg' 'feature_RSDRlCapAvNddg' 'feature_MeanRlCapAvNddg' ...
     'feature_RSDRlPotNetExcess' 'feature_MeanRlPotNetExcess' 'feature_RSDAVRlPotNetExcess' 'feature_MeanAVRlPotNetExcess'};

StdtextHeader = strjoin(StdTblHeader, ',');
%}



%% write header to file

%------------- Create relevant files to write data on them----------------

%{
fid = fopen('Basic_factors_metadata_AC.csv','w'); 
fprintf(fid,'%s\n',textHeader_BasicFactors);
fclose(fid);
clear fid* 
%}

%{
fid = fopen('maxflow_metadata.csv','w'); 
fprintf(fid,'%s\n',textHeader);
fclose(fid);
clear fid* 
% write data to end of file
% N = randn(4,12);
% dlmwrite('maxflow_metadata.csv',N,'delimiter',',','-append');

%}

%---------------------Purified by SimularityAssessment.m ------------------
%{
epsilon=0.40

fid = fopen(sprintf('Purified_individuals_metadata_Dist_%.2f.csv', epsilon),'w'); 
fprintf(fid,'%s\n',textHeader);
fclose(fid);
clear fid* 

fid = fopen(sprintf('Precluded_individuals_metadata_Dist_%.2f.csv', epsilon),'w'); 
fprintf(fid,'%s\n',textHeader);
fclose(fid);
clear fid* 
%}

%---------------------------Standard deviations----------------------------

%{
fid = fopen('RSD_Mean_basic_factors_metadata_glob_5id.csv','w'); 
fprintf(fid,'%s\n',StdtextHeader_BasicFactors);
fclose(fid);
clear fid* 


% fid = fopen('maxflow_metadata.csv','w'); 
fid = fopen('RSD_Mean_metadata_glob_5idx.csv','w'); 
fprintf(fid,'%s\n',StdtextHeader);
fclose(fid);
clear fid* 

%}



% -------------------------------------------------------------------------

%% Call different functions  for different purposes


% Name=cell(2, 13); % Define a cell according to the size of NetName


for NN=1:size(NetName)
    
%     ISAFtr(NetName(NN));
%     ISAFtrExtractorPool(NetName(NN));
    purifyInstIS(NetName(NN), epsilon);
%     assessDiversity(NetName(NN));

%% This is to collect all networks' features in a metadata file.
%{
Xbar = readtable(sprintf('%s.csv',NetName(NN)));
MainTbl = readtable('maxflow_metadata.csv');
MainTbl=[MainTbl; Xbar];
writetable(MainTbl, 'maxflow_metadata.csv');
%}
    %%
  %{
    BnchCaps = sprintf('%s_Caps', NetName(NN));
%     BnchCaps = sprintf('%s_Caps', 'AK');
    BnchCaps = eval(BnchCaps);% This command converts the string BnchNds into variable BnchNds.
    % Using eval() do not advised in general however; see the below link:
    % https://au.mathworks.com/matlabcentral/answers/?term=eval
    
    BnchNds = sprintf('%s_Nds', NetName(NN));
%     BnchNds = sprintf('%s_Nds', 'AK');
    BnchNds = eval(BnchNds);
    
    for CP=1:size(BnchCaps)
        for ND=1:size(BnchNds)
            
%             ISAStdExtractor(NetName(NN), BnchCaps(CP), BnchNds(ND));
%             ISA_Mean_RSD_Extractor(NetName(NN), BnchCaps(CP), BnchNds(ND));
            
%             for idx = 1:5
%                 idx=1;
%             ClctdiffSizes(NetName(NN), BnchCaps(CP), BnchNds(ND), idx);
                %             ISABsCFCtrClctr(NetName(NN), BnchCaps(CP), BnchNds(ND));
%             ISABsCFCtrClctrGlob(NetName(NN), BnchCaps(CP), BnchNds(ND));
%             ISAFtrExtractorAv(NetName(NN), BnchCaps(CP), BnchNds(ND));
%              ISAFtrExtractor(NetName(NN), BnchCaps(CP), BnchNds(ND), idx);
%              ISAFtrExtractorPrAlg(NetName(NN), BnchCaps(CP), BnchNds(ND), idx);
%             ISAFtrClctr(NetName(NN), BnchCaps(CP), BnchNds(ND));
%             ISADIMACSFtrClctr(NetName(NN), BnchCaps(CP), BnchNds(ND));
%             dimacs_features(NetName(NN), BnchCaps(CP), BnchNds(ND))
%             ISAFtrClctrMat(NetName(NN), BnchCaps(CP), BnchNds(ND));
%             end
        end
    end
    %}
end
