import pandas as pd
import matplotlib.pyplot as plt
from scipy.signal import butter, filtfilt
import sys
sys.path.append('.')
from Scripts.LowPassFilter import LowPassFilter
import numpy as np

# Import the data
experimentCsvFile = r"Scripts/Experiment_Rpm.csv"
rpmTimeData = pd.read_csv(experimentCsvFile)
rpmTimeMatrix = rpmTimeData.values

# Filter the values using a band-pass filter
b, a = butter(2, 80, btype='low', fs= 1000, analog=False)
filteredRpm = filtfilt(b, a, rpmTimeMatrix[:, 1])

# Manual low pass filter
lowPassFilteredRpm = []
filterConstant = 0.2
prevValue = 0
for i in range(len(rpmTimeMatrix[:, 1])):
    filteredValue = filterConstant * (rpmTimeMatrix[i, 1]) + (1-filterConstant) * prevValue
    prevValue = filteredValue
    lowPassFilteredRpm.append(filteredValue)

# Lowpass filter
lowPassFilt = LowPassFilter(3., 0.05)
lastTime = 0.
expLowPassFiltRpm = []
for i in range(len(rpmTimeMatrix[:, 1])):
    deltaTime = rpmTimeMatrix[i, 0] - lastTime
    print(deltaTime)
    filteredVal = lowPassFilt.updateAndReconfigureFilter(rpmTimeMatrix[i, 1], deltaTime=deltaTime, cutOffFreq=3.)
    lastTime = rpmTimeMatrix[i, 0]
    print(lowPassFilt.getExponentialPower())
    expLowPassFiltRpm.append(filteredVal)

# Plot the data
plt.plot(rpmTimeMatrix[:, 0], rpmTimeMatrix[:, 1], label="Unfiltered data")
# plt.plot(rpmTimeMatrix[:, 0], filteredRpm, label="Filtered")
# plt.plot(rpmTimeMatrix[:, 0], lowPassFilteredRpm, label="LowPass Filtered")
plt.plot(rpmTimeMatrix[:, 0], expLowPassFiltRpm, label="ExpLowPass")
plt.grid()
plt.legend()
plt.show()