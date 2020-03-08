from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QObject, QIODevice
from PyQt5.QtWidgets import QApplication, QWidget
import pyqtgraph as pg
from ExperimentDataCollector_Des import Ui_Form
import re
import csv

class ExperimentDataCollector(QWidget, Ui_Form):
    def __init__(self):
        super(ExperimentDataCollector, self).__init__()
        # Setupui
        self.setupUi(self)
        # Create a serial port
        self.__serialPort = QSerialPort()
        # Setup Serialport
        # We are using USB serial driver, so the baud rate doesn't matter
        self.__serialPort.setBaudRate(QSerialPort.Baud1200, QSerialPort.Input)
        # Get the list of all available serial ports
        portsList = QSerialPortInfo.availablePorts()
        # Only connect to Chibios port
        chibiOsPort = None
        self.setWindowTitle("Experiment Data Collector")
        for port in portsList:
            if ("ChibiOS" in port.description()):
                chibiOsPort = port
                print(chibiOsPort.description())
        # Check of the device is connected.
        if (chibiOsPort is None):
            # We didn't find anything
            statusString = "Cannot find Chibios based device."
            print(statusString)
        else:
            # Set the serial port
            self.__serialPort.setPort(chibiOsPort)
            self.__serialPort.setDataBits(QSerialPort.Data8)
            self.__serialPort.setFlowControl(QSerialPort.NoFlowControl)
            self.__serialPort.setParity(QSerialPort.NoParity)
            self.__serialPort.setStopBits(QSerialPort.OneStop)
            # Connect signals and slots
            self.__serialPort.readyRead.connect(self.__onSerialPortReadyRead)
            self.startButton.clicked.connect(self.__onStartButtonClicked)
            # Open the device
            self.__serialPort.open(QIODevice.ReadWrite)
            # Initialize variables
            # Timestamp in seconds
            self.__timeStampArray = []
            # Angle in degrees
            self.__angleArray = []
            # Add the curves as well for plotting
            self.__angleDataCurve = pg.PlotCurveItem()
            self.graphicsView.addItem(self.__angleDataCurve)
            # Open csv file to write
            csvFileObj = open("ValidationData.csv", mode='w', newline='')
            self.__csvWriterObj = csv.writer(csvFileObj)
            self.__csvWriterObj.writerow(['Timestamp', 'Angle in deg'])

    def __onStartButtonClicked(self):
        message = "valid\r\n"
        self.__serialPort.write(message.encode())
        self.__angleDataCurve.clear()
        self.__timeStampArray.clear()
        self.__angleArray.clear()

    def __onSerialPortReadyRead(self):
        numbersRegex = re.compile(r"[-+]?\d*\.?\d+")
        while (self.__serialPort.canReadLine()):
            bytesRead = self.__serialPort.readLine()
            strBytesRead = str(bytesRead)
            floatsfound = numbersRegex.findall(strBytesRead)
            if(floatsfound):
                timestamp = float(floatsfound[0])
                angle = float(floatsfound[1]) * 57.2958
                self.__timeStampArray.append(timestamp)
                self.__angleArray.append(angle)
                self.__angleDataCurve.setData(self.__timeStampArray, self.__angleArray)
                self.__csvWriterObj.writerow([timestamp, angle])

    def close(self):
        print("Closing")

if __name__ == '__main__':
    app = QApplication([])
    experiment = ExperimentDataCollector()
    experiment.show()
    app.exec_()