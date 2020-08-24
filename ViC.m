function ViC(Net)

%% Descriptions
% This code calculates the viloation criterion of a purifed metadata;
% =========================================================================
% Code by: Hossein Alipour
%          School of Mathematics and Statistics
%          The University of Melbourne
%          Australia
%          2020
%          Email: h.alipour@unimelb.edu.au
 
%  Copyright: Hossein Alipour
%

%% Parameters 
% Samples of the array of similairty treshholds
% epsilon = [0, 0.025, 0.05, 0.075, 0.10, 0.125, 0.15, 0.175, 0.20, 0.225, 0.25, ...
%     0.275, 0.30, 0.325, 0.35, 0.375, 0.40]; 
% epsilon = [0, 0.050, 0.100, 0.150, 0.200, 0.250, 0.300, 0.350, 0.400]; 
% epsilon = [0, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40]; 

flag = ["Ftr"; "Ftr&AP"; "Ftr&Good"; "Ftr&AP&Good"];

TblHeader_ViC = {'Epsilon' 'Ftr_instNum' 'Ftr_AP_instNum' 'Ftr_Good_instNum' ...
    'Ftr_AP_Good_instNum' 'Ftr_AP' 'Ftr_Good' 'Ftr_AP_Good' 'Rl_Ftr_AP' 'Rl_Ftr_Good' 'Rl_Ftr_AP_Good' ...
    'Ftr_Sim_inst' 'Rl_Sim_Ftr' 'Rl_Sim_Ftr_AP' 'Rl_Sim_Ftr_Good' 'Rl_Sim_Ftr_AP_Good'};

textHeader_ViC = strjoin(TblHeader_ViC, ',');
fid = fopen(sprintf('ViC_Purified2_%s.csv',Net),'w'); 
fprintf(fid,'%s\n',textHeader_ViC);
fclose(fid);
clear fid* 

Xbar1 = readtable(sprintf('CVNND_%s_Purified2_%s.csv',flag(1), Net));
Xbar2 = readtable(sprintf('CVNND_%s_Purified2_%s.csv',flag(2), Net));
Xbar3 = readtable(sprintf('CVNND_%s_Purified2_%s.csv',flag(3), Net));
Xbar4 = readtable(sprintf('CVNND_%s_Purified2_%s.csv',flag(4), Net));

Current_data_ViC = zeros(length(Xbar1.Epsilon), 16);
fid = fopen(sprintf('ViC_Purified2_%s.csv',Net),'a');
fprintf(fid,'%f,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f\n', Current_data_ViC);
fclose(fid);
clear fid* 

Tbl = readtable(sprintf('ViC_Purified2_%s.csv',Net));
%%

 
Tbl.Epsilon = Xbar1.Epsilon; 

Tbl.Ftr_instNum = Xbar1.InstNumb;
Tbl.Ftr_AP_instNum = Xbar2.InstNumb;
Tbl.Ftr_Good_instNum = Xbar3.InstNumb;
Tbl.Ftr_AP_Good_instNum = Xbar4.InstNumb;

%% Raw ViC

Tbl.Ftr_AP = Xbar2.InstNumb - Xbar1.InstNumb;
Tbl.Ftr_Good = Xbar3.InstNumb - Xbar1.InstNumb;
Tbl.Ftr_AP_Good = Xbar4.InstNumb - Xbar1.InstNumb;

%% Relative ViC: raw ViC over the left instances

Tbl.Rl_Ftr_AP = Tbl.Ftr_AP ./ Xbar2.InstNumb;
Tbl.Rl_Ftr_Good = Tbl.Ftr_Good ./ Xbar3.InstNumb;
Tbl.Rl_Ftr_AP_Good = Tbl.Ftr_AP_Good ./ Xbar4.InstNumb;

%% Raw ViC over the total similar benchmarks


Tbl.Ftr_Sim_inst = Xbar1.InstNumb(1) - Xbar1.InstNumb; 

% Rl_Sim_Ftr indicates the proportion of similar benchmarks wit Ftr flag
% over all benchmarks;
Tbl.Rl_Sim_Ftr = Tbl.Ftr_Sim_inst ./ Xbar1.InstNumb(1); 


Tbl.Rl_Sim_Ftr_AP = Tbl.Ftr_AP ./ Tbl.Ftr_Sim_inst;
Tbl.Rl_Sim_Ftr_Good = Tbl.Ftr_Good ./ Tbl.Ftr_Sim_inst;
Tbl.Rl_Sim_Ftr_AP_Good = Tbl.Ftr_AP_Good ./ Tbl.Ftr_Sim_inst;



%% Wrtie data on the table
writetable(Tbl, sprintf('ViC_Purified2_%s.csv',Net));

end



