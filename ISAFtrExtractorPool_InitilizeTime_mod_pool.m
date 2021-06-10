function ISAFtrExtractorPool_InitilizeTime_mod_pool(Net)

%% Description
% This code collects Basic factors of maximum flow problem and saves them in "maxflow_basic_factors_metadata.csv" file.
% The basic factors extracted by "dimacs_basic_factors.c" software from DIMACS benchmarks
% and collected by "ISABsCFCtrClctrGlob.m" software for dfirrenet indexes of fixed sizes in thier correlated files.
% Based on This factors, the code extracts different features of MFP and
% saves the results in "maxflow_metadata.csv" file to be used by ISA.

%% Determine main directories
Main_dir = 'F:\codes\ISA\Feature_collector\No-OCs\1and10_runRep_InitilaizeTime-f'; % pwd
Network_Basic_factors_dir = 'F:\codes\ISA\Feature_Results\Basic_factors';
Pseudo_dir = 'F:\codes\maxflow_implementations\Static\Impl\Spartan_Results\No-OCs\Pseudo';
Push_dir = 'F:\codes\maxflow_implementations\Static\Impl\Spartan_Results\No-OCs\Push-relable';
PAR_dir = 'F:\codes\maxflow_implementations\Static\Impl\Spartan_Results\No-OCs\PAR';
P2R_dir = 'F:\codes\maxflow_implementations\Static\Impl\Spartan_Results\No-OCs\P2R';
Dinic_dir = 'F:\codes\maxflow_implementations\Static\Impl\Spartan_Results\No-OCs\Dinic';
Begin_dir = Network_Basic_factors_dir ;
Output_dir = Main_dir;

%% Define the headers and cratignthe relevant files

TblHeader_BasicFactors = {'Instances' 'Source' 'Nodes' 'Arcs' 'NetCap' 'maxcap'	'mincap' ...
    'SrcAAC' 'SnkAAC' 'Src_degree'	'Snk_degree' 'StDAC' 'NmBdCap' 'NmGdCap' 'PotNetExcess'	'PotNetDeficit'};

textHeader_BasicFactors = strjoin(TblHeader_BasicFactors, ',');

fid = fopen(sprintf('BasicFactors_%s.csv',Net),'w');
fprintf(fid,'%s\n',textHeader_BasicFactors);
fclose(fid);
clear fid*

%==========================================================================
TblHeader = {'Instances' 'Source' 'Dinic_max' 'Push_max' 'PAR_Max'  'P2R_Max' 'Pseudo_Max' 'rep_Dinic' 'rep_Push' 'rep_PAR' 'rep_P2R' 'rep_Pseudo' ...
    'algo_Dinic' 'algo_Push' 'algo_PAR' 'algo_P2R' 'algo_Pseudo' 'x_DinicROC' 'x_PushROC' 'x_PARROC' 'x_P2RROC' 'x_PseudoROC' ...
    'feature_Nodes' 'feature_Arcs' 'feature_Density' 'x_CapDens' 'feature_AvNdDgr' ...
    'x_TrEtraFlw' 'feature_PercNetPotPth' 'feature_EstPercDepFlw' ...
    'feature_PercLoCap' 'feature_PercHiCap' 'x_StDAC' 'x_RlMxCap' ...
    'x_RlMnCap' 'x_AvNdCap' 'feature_ScAvNdCap'  ...
    'feature_ScPotNetExcess' 'feature_AVScPotNetExcess' ...
    'feature_CVAC' 'feature_CVAvNdDg' 'feature_ScRngCap' 'feature_ScAvCap' 'feature_ScCapDens' ...
    };

textHeader = strjoin(TblHeader, ',');

%write header to file
% fid = fopen('maxflow_metadata.csv','w');
fid = fopen(sprintf('Features_%s.csv',Net),'w');
fprintf(fid,'%s\n',textHeader);
fclose(fid);
clear fid*
%==========================================================================

Empty_TblHeader = {'Empty_Inst'};

EmptytextHeader = strjoin(Empty_TblHeader, ',');

%write header to file
% fid = fopen('maxflow_metadata.csv','w');
fid = fopen(sprintf('Empty_Inst_%s.csv',Net),'w');
fprintf(fid,'%s\n',EmptytextHeader);
fclose(fid);
clear fid*

%==========================================================================

%% Load the list of files that must be neglected

if isfile(sprintf('empty_list_%s.m', Net))
    eval(sprintf('empty_list_%s', Net));
else
    Empty_List = [];
end

%%    Determine the name of sub-directories and files holding data
Net
Sub_dir = sprintf('%s', Net);

cd(Begin_dir);
cd(Sub_dir);
MyFolderInfo = dir(fullfile('./', '*.csv'));


for i=1:size(Empty_List)
    MyFolderInfo = MyFolderInfo(~endsWith({MyFolderInfo.name}, Empty_List(i)));
end

for Nm=1:length(MyFolderInfo)
    
    FlNm = MyFolderInfo(Nm).name
    [filepath,name,ext] = fileparts(FlNm);
    AlgFileName = name(1:end-7);
    AlgFileName = sprintf('%s.csv', AlgFileName);
    
    Pseudo_dir = 'F:\codes\maxflow_implementations\Static\Impl\Spartan_Results\No-OCs\Pseudo';
    Push_dir = 'F:\codes\maxflow_implementations\Static\Impl\Spartan_Results\No-OCs\Push_WithoutEliminating';
    PAR_dir = 'F:\codes\maxflow_implementations\Static\Impl\Spartan_Results\No-OCs\PAR_WithoutEliminating';
    P2R_dir = 'F:\codes\maxflow_implementations\Static\Impl\Spartan_Results\No-OCs\P2R_WithoutEliminating';
    Dinic_dir = 'F:\codes\maxflow_implementations\Static\Impl\Spartan_Results\No-OCs\Dinic';
    
    for idx = 1:1;
        
        %% Go to the subdirectories to read relevent data and extract favorite information
                PushPerf = 0;
                PushPerf2 = 0;
                Push_Max = 0;
                rep_Push = 0;
                %----------------
                PARPerf = 0;
                PARPerf2 = 0;
                PAR_Max = 0;
                rep_PAR = 0;
                %----------------
                P2RPerf = 0;
                P2RPerf2 = 0;
                P2R_Max = 0;
                rep_P2R = 0;
                %----------------
                PseudoPerf = 0;
                PseudoPerf2 = 0;
                Pseudo_Max = 0;
                rep_Pseudo = 0;
                %----------------
                DinicPerf = 0;
                DinicPerf2 = 0;
                Dinic_Max = 0;
                rep_Dinic = 0;
        
            %--------------------------------------------------------------------------
            cd(Network_Basic_factors_dir);
            cd(Sub_dir);
            
            BscFctrFileName = sprintf('%s.csv', name);
            %Read data from tables
            % FileName
            % NetBsCFCtr_Tbl = readtable(FileName);
            
            opts = detectImportOptions(BscFctrFileName,'NumHeaderLines',0);
            NetBsCFCtr_Tbl = readtable(BscFctrFileName,opts);
            %--------------------------------------------------------------------------
            % Basic Factors
            
            Nodes = NetBsCFCtr_Tbl.Nodes;
            Arcs = NetBsCFCtr_Tbl.Arcs;
            NetCap = NetBsCFCtr_Tbl.NetCap;
            MaxCap= NetBsCFCtr_Tbl.maxcap;
            MinCap= NetBsCFCtr_Tbl.mincap;
            SrcAAC = NetBsCFCtr_Tbl.SrcAAC;
            SnkAAC = NetBsCFCtr_Tbl.SnkAAC;
            Src_degree = NetBsCFCtr_Tbl.Src_degree;
            Snk_degree = NetBsCFCtr_Tbl.Snk_degree;
            AvCap = NetCap / Arcs;
            StDAC = NetBsCFCtr_Tbl.StDAC;
            NmBdCap = NetBsCFCtr_Tbl.NmBdCap;
            NmGdCap = NetBsCFCtr_Tbl. NmGdCap;
            PotNetExcess = NetBsCFCtr_Tbl.PotNetExcess;
            PotNetDeficit = NetBsCFCtr_Tbl.PotNetDeficit;
            StDAvNdDg =  NetBsCFCtr_Tbl.StDNddegree;
            
            %--------------------------------------------------------------------------
            % Features
            Density = Arcs / (Nodes*(Nodes-1));
            AvNdDg = Arcs / Nodes; % Average node degree
            CVAC = StDAC / AvCap;
            CVAvNdDg = StDAvNdDg / AvNdDg;
            EstPercDepFlw = ((NetCap - SrcAAC)/(Arcs-Src_degree))*(Arcs/(Nodes-1))/(SrcAAC/Src_degree);
            PercNetPotPth = SrcAAC / (NetCap - SrcAAC);
            PercLoCap = NmBdCap / Arcs;
            PercHiCap = NmGdCap / Arcs;
            ScRngCap = (MaxCap - MinCap) / AvCap;
            MedCap = (MaxCap + MinCap) / 2;
            ScAvCap = AvCap / MedCap;
            RlMxCap = MaxCap / AvCap;
            RlMnCap = MinCap / AvCap;
            AvNdCap = NetCap / Nodes;
            %ScAvNdCap = AvNdCap / AvCap;            
            CapDens = AvCap*Density;
            ScCapDens = ScAvCap*Density;
            ScAvNdCap = AvNdCap / MedCap;
            ScPotNetExcess = PotNetExcess / AvCap;
            AVScPotNetExcess = ScPotNetExcess / (Nodes-2);
            
            if (SrcAAC - SnkAAC >0)
                TrEtraFlw = SrcAAC - SnkAAC;
            else
                TrEtraFlw = 0;
            end
            %--------------------------------------------------------------------------
            Instances = strings; % Create an empty string array
            
            parNm = sprintf('%s', Net);
            numel(parNm);
            Instances(1)= sprintf('%s(%s)', name(1:end-7), parNm(end-3:end));
            %             Source = strings;
            Source = sprintf('%s', Net);
            
            
             Current_data_basic_factors = [Instances, Source, Nodes, Arcs, NetCap, MaxCap, MinCap, SrcAAC,	SnkAAC, ...
                Src_degree, Snk_degree, StDAC, NmBdCap, NmGdCap, PotNetExcess, PotNetDeficit];
            %--------------------------------------------------------------------------
            Current_data_fatures = [Instances, Source, Dinic_Max, Push_Max, PAR_Max,P2R_Max, Pseudo_Max, rep_Dinic, rep_Push, rep_PAR, rep_P2R, rep_Pseudo ...
                DinicPerf, PushPerf, PARPerf, P2RPerf, PseudoPerf, DinicPerf2, PushPerf2, PARPerf2, P2RPerf2, PseudoPerf2, Nodes, Arcs, Density, CapDens, AvNdDg, TrEtraFlw, ...
                PercNetPotPth, EstPercDepFlw, PercLoCap, PercHiCap, StDAC, RlMxCap,...
                RlMnCap, AvNdCap, ScAvNdCap, ScPotNetExcess, AVScPotNetExcess, CVAC, CVAvNdDg, ScRngCap, ScAvCap, ScCapDens];
            
            %% Go to the output directory to save features and performance measures in maxflow_metadata.csv file.
            
            cd(Output_dir);
            
            %-------------------------Collect basic factors----------------------------
            fid = fopen(sprintf('BasicFactors_%s.csv',Net),'a');
            % fprintf(fid,'%s,%s,%s,%s,%s\n', Instances, Dens1', Dens2', PushPerf', PseudoPerf');
            fprintf(fid,'%s,%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n', Current_data_basic_factors);
            fclose(fid);
            clear fid*
            % dlmwrite('maxflow_metadata.csv',Current_data,'delimiter',',','-append');
            
            %--------------------------collect features--------------------------------
            
            fid2 = fopen(sprintf('Features_%s.csv',Net),'a');
            fprintf(fid2,'%s,%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n', Current_data_fatures);
            fclose(fid2);
            clear fid2*
            

    end
end

%% Go back to the main directory

cd(Main_dir)





