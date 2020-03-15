import numpy as np

class LowPassFilter(object):
    def __init__(self, cutOffFreq, deltaTime):
        self.mCutOffFreq = cutOffFreq
        self.mDeltaTime = deltaTime
        self.mExpPower = 1 - np.exp(-self.mDeltaTime * 2 * np.pi * self.mCutOffFreq)
        self.mOutput = 0.

    def updateFilter(self, input):
        self.mOutput += (input - self.mOutput) * self.mExpPower
        return self.mOutput

    def updateAndReconfigureFilter(self, input, deltaTime, cutOffFreq):
        self.reconfigureFilter(deltaTime, cutOffFreq)
        return self.updateFilter(input)

    def reconfigureFilter(self, deltaTime, cutOffFreq):
        self.mDeltaTime = deltaTime
        self.mCutOffFreq = cutOffFreq
        self.mExpPower = 1 - np.exp(-self.mDeltaTime * 2 * np.pi * self.mCutOffFreq)

    def getExponentialPower(self):
        return self.mExpPower