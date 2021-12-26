clc; clear all; close all;

%% user input
param.m = 983893.134581486;
param.b= -4403.04101740844;
param.xm_zero = 1428;
param.mx_zero = 0;
param.fpass = 0.5; 
param.fs = 1500; 

pos_static_file_name = 'processed_pos_static.txt';
neg_static_file_name = 'processed_neg_static.txt';
pos_raw_data = ["experiment_121921/log_val_20211219182706_50", ...
    "experiment_121921/log_val_20211219183204_70", ...
    "experiment_121921/log_val_20211219183648_100"];


% the pos_vel and neg_vel should have the same order as raw_data.
pos_vel = [50, 70, 100];


%%
npos = length(pos_vel);


%% static load
StaticLoadPos = load(pos_static_file_name);


%% Load and process the data
pos_data = readTestData(pos_raw_data, param);



pos_filtered_data = getFit(pos_data); 

% 
theta = 0:0.01:2*pi;
%  
pos_fitted_data = zeros(length(theta), npos);
for data_idx = 1:npos
   pos_fitted_data(:, data_idx) = pos_filtered_data{data_idx}.fitresult(theta);
end


%%

%%
figure(30);
set(gcf,'Position',[600 600 800 500]);
subplot(2,1,1); hold on;
title('Positive Velocity')
for data_idx = 1:npos
    plot(theta,  pos_fitted_data(:, data_idx) - StaticLoadPos, 'LineWidth', 1, 'DisplayName',int2str(pos_vel(data_idx)));
end
xlabel('Position (rad)');
ylabel('Load (g)');
legend;




%% Gear friction coefficinet
pos_avg_data = zeros(npos, 1);
for data_idx = 1:npos
    pos_avg_data(data_idx) = mean( pos_fitted_data(:, data_idx) - StaticLoadPos) *4.5*0.0254; %/ (data * 0.114 * 1/60 * 2*pi);
end




% change it from gram to Newton
pos_avg_data = pos_avg_data* 1e-3*9.81; 

a = 0.114 * 1/60 * 2*pi; 
pos_vel_real = pos_vel*a;


X_pos = [ones(length(pos_vel_real),1) pos_vel_real'];


b1 = X_pos\pos_avg_data;


x1 = 0:0.01:1.5;


y1 = x1*b1(2) + b1(1);

figure(5);
set(gcf,'Position',[600 600 800 500]);
scatter(pos_vel_real, pos_avg_data', 'r'); hold on;

plot(x1,y1,'-k');

xlabel('Velocity (rad/s)');
ylabel('Torque (Nm)');
title('Gear train frictional torque'); 
ylim([-0.05 0.05]);
xlim([-1.5 1.5]);
legend('Data', 'Best fit line','Location','northwest'); 

%%
% %% Current plot
% figure(3) 
% plot(raw_data_gear{1}.xm_current);
%% 

function test_data = readTestData( test_data_results, param )

for i = 1:length( test_data_results )
   
    filename = append( test_data_results(i), '.txt' );
    data = importdata(filename);
    
    xm_abs_pos = data.data(:,10) - param.xm_zero;
    mx_abs_pos = ( data.data(:,13) - param.mx_zero ) ;
    
    % Relative angular position of each motor shaft
    REV2RAD = 2*pi/4096; % revolution to radian conversion 
    xm_rel_pos = mod(xm_abs_pos, 4095) * REV2RAD;
    mx_rel_pos = mod(mx_abs_pos, 4095) * REV2RAD;
    
    % Revolution index
    rev_count_idx = floor(xm_abs_pos/ 4095);
    min_rev_n = min(rev_count_idx); max_rev_n = max(rev_count_idx);
    full_idx = rev_count_idx ~= min_rev_n & rev_count_idx ~= max_rev_n;
    
    force = data.data(:,3);
    force_lp = lowpass(force, param.fpass, param.fs);
    

    test_data{i}.force = force(full_idx);
    test_data{i}.force_lp = force_lp(full_idx);
    test_data{i}.rev_idx = rev_count_idx(full_idx);

    test_data{i}.xm_abs_pos = xm_abs_pos(full_idx);
    test_data{i}.mx_abs_pos = mx_abs_pos(full_idx);
    test_data{i}.xm_rel_pos = xm_rel_pos(full_idx);
    test_data{i}.mx_rel_pos = mx_rel_pos(full_idx);

    
    
end

end

% fit data
function data_fit = getFit(data)

for i = 1:length(data)
    x = data{i}.mx_rel_pos;
    y = data{i}.force_lp;
    [fitresult, gof] = createFit(x, y);
    data_fit{i}.fitresult = fitresult;
    data_fit{i}.gof = gof;
end

end
function [fitresult, gof] = createFit(x, y)
%CREATEFIT(X1,Y1)
%  Create a fit.
%
%  Data for 'untitled fit 1' fit:
%      X Input : x1
%      Y Output: y1
%  Output:
%      fitresult : a fit object representing the fit.
%      gof : structure with goodness-of fit info.
%
%  See also FIT, CFIT, SFIT.

[xData, yData] = prepareCurveData( x, y );

% Set up fittype and options.
ft = fittype( 'fourier5' );
opts = fitoptions( 'Method', 'NonlinearLeastSquares' );
opts.Display = 'Off';
opts.StartPoint = [0 0 0 0 0 0 0 0 0 0 0 1.00048851978505];

% Fit model to data.
[fitresult, gof] = fit( xData, yData, ft, opts );

end

% function [coupler, non_coupler] = calculateCouplerEffect(data1, data2)
% 
% theta = 0:0.01:2*pi;
% NUM_OF_SPEED = length(data1)
% 
% coupler = zeros(NUM_OF_SPEED, length(theta));
% non_coupler = zeros(NUM_OF_SPEED * 2, length(theta));
% 
% for i = 1:NUM_OF_SPEED
%     coupler(:,i) = ( data{i}.fitresult(theta) + data{i + NUM_OF_SPEED}.fitresult(theta) ) * 0.5;
%     non_coupler(:,i) = data{i}.fitresult(theta) - coupler(:,i);
%     non_coupler(:,i + NUM_OF_SPEED) = data{i + NUM_OF_SPEED}.fitresult(theta) - coupler(:,i);
% end
% 
% end