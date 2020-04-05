clear all;
clc;
%% Add dc motor parameters
% The parameters are from https://electronics.stackexchange.com/questions/420313/dc-motor-model-issue
% DC_Motor_B = 0.0001; % Damping factor N/rad/sec
% DC_Motor_J = 0.993 * 10E-3 * (0.01)^2; % kg-m2
% DC_Motor_K = 1/(2*pi*849/60); % Back emf constant V/(rad/sec)
% DC_Motor_L = 0.343*10E-3; % H
% DC_Motor_R = 11.4; %ohm 

%% The new estimated parameters are
DC_Motor_B = 1.2011e-05;
DC_Motor_J = 1.7848e-06;
DC_Motor_K = 0.0040474;
DC_Motor_L = 0.000265;
DC_Motor_R = 0.0031974;
