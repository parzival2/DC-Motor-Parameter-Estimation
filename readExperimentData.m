%% Read csv file
csvMatrix = readmatrix('Experiment_Rpm.csv');
exTimestamp = csvMatrix(:, 1);
exAngle = csvMatrix(:, 2);
% Plot the simulation data along with real experiment data
plot(exTimestamp, exAngle);
simRpm = out.rpm.data;
simTime = out.rpm.time;
hold on;
plot(simTime, simRpm);
hold off;
