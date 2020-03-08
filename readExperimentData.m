%% Read csv file
csvMatrix = readmatrix('Experiment_Rpm.csv');
exTimestamp = csvMatrix(:, 1);
exAngle = csvMatrix(:, 2);
% Plot the simulation data along with real experiment data
plot(exTimestamp, exAngle);
simRpm = Sim_Out.Sim_RPM.Data;
simTime = Sim_Out.Sim_RPM.Time;
hold on;
plot(simTime, simRpm);
hold off;
