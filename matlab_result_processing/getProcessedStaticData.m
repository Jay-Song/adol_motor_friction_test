clear all; clc; close all; 

%% user input
pos_filename = 'pos.txt';
neg_filename = 'neg.txt';
processed_pos_file_name = 'processed_pos_static.txt';
processed_neg_file_name = 'processed_neg_static.txt';
m = 980409.3205;
b = -1734.899824;

%%
load_pos = readStatic(pos_filename, m, b);
load_neg = readStatic(neg_filename, m, b);

load_pos_fit = getFit(load_pos);
load_neg_fit = getFit(load_neg);

theta = 0:0.01:2*pi;

staticload_pos = load_pos_fit.fitresult(theta);
staticload_neg = load_neg_fit.fitresult(theta);

figure(10); hold on;
set(gcf,'Position',[600 600 800 500]);
title('Averaged static load cell values')
plot(load_pos(:,1), load_pos(:,2));
plot(load_neg(:,1), load_neg(:,2));
xlabel('Position [rad]');
ylabel('load [g]');
legend('Positive Velocity', 'Negative Velocity')
hold off;

figure(11); hold on;
set(gcf,'Position',[600 600 800 500]);
title('Fitted static load cell values')
plot(theta, staticload_pos);
plot(theta, staticload_neg);
xlabel('Position [rad]');
ylabel('load [g]');
legend('Positive Velocity', 'Negative Velocity')
hold off;

%% save the result
writematrix(staticload_pos, processed_pos_file_name);
writematrix(staticload_neg, processed_neg_file_name);

%%
function out = readStatic(filename, m, b)

fileID = fopen(filename);
data = textscan(fileID, '%f %f %f %f %f %s %f %f %f %s %f %f', 'Delimiter', '\t');
fclose(fileID);

force = data{2} * m + b;
% figure(7); hold on;
% set(gcf,'Position',[600 600 800 500]);
% title('Raw static load cell data')
% xlabel('Time [s]');
% ylabel('load [g]');
% plot(force(10:end-10));

force = lowpass(force, 0.001, 1500);

% figure(8); hold on;
% set(gcf,'Position',[600 600 800 500]);
% title('Filtered static load cell data')
% xlabel('Time [s]');
% ylabel('load [g]');
% plot(force(10:end-10));

REV2RAD = 2*pi/4096;
mx_pos = mod(data{12}, 4095) * REV2RAD;


load = [force(10:end-10), mx_pos(10:end-10)];




[theta,jj,kk]=unique(load(:,2)); 
mean_load = accumarray(kk,(1:numel(kk))',[],@(x) mean(load(x,1)));


out = [theta, mean_load];
end

% fit data
function data_fit = getFit(data)

    x = data(:,1);
    y = data(:,2);
    [fitresult, gof] = createFit(x, y);
    data_fit.fitresult = fitresult;
    data_fit.gof = gof;

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