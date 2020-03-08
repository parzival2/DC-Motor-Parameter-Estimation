%% Read csv file
csvMatrix = readmatrix('ValidationData.csv');
exTimestamp = csvMatrix(:, 1);
exAngle = csvMatrix(:, 2);
% Plot the simulation data along with real experiment data
plot(exTimestamp, exAngle);
hold on;
plot(sim_Angleindeg);
hold off;
