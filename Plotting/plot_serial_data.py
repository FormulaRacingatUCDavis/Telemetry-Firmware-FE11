# Usage info:
# Input port name as command line argument, can use get_serial_ports.py to find ports
# Can press any key to pause the plotting, pressing again will resume at present time
# If no data shows up, try running it again or unplugging and replugging the transceiver
# Time value is seconds that the board has been running

from threading import Thread
import serial
import time
import collections
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import struct
import sys
import copy


class serialPlot:
    def __init__(self, serialPort, serialBaud, plotLength, packetNumBytes, numDataTypes, typeNumVals, valNumBytes, typeValMultiplier):
        self.port = serialPort
        self.baud = serialBaud
        self.packetNumBytes = packetNumBytes
        self.numDataTypes = numDataTypes
        self.typeNumVals = typeNumVals
        self.valNumBytes = valNumBytes
        self.typeValMultiplier = typeValMultiplier
        self.rawData = bytearray(packetNumBytes)
        self.data = []
        for i in range(numDataTypes):
            self.data.append([])
            for j in range(typeNumVals[i] + 1):
                # each data type has an array for each value of that type + time
                # time is index 0
                self.data[i].append(collections.deque([], maxlen=plotLength))
        self.isRun = True
        self.isReceiving = False
        self.thread = None
        self.prevData = None
        self.plotTimer = 0
        self.previousTimer = 0
        self.firstTime = 0

        print('Trying to connect to: ' + str(serialPort) +
              ' at ' + str(serialBaud) + ' BAUD.')
        try:
            self.serialConnection = serial.Serial(
                serialPort, serialBaud, timeout=4)
            print('Connected to ' + str(serialPort) +
                  ' at ' + str(serialBaud) + ' BAUD.')
        except:
            sys.exit("Failed to connect with " + str(serialPort) +
                     ' at ' + str(serialBaud) + ' BAUD.')

    def readSerialStart(self):
        if self.thread == None:
            self.thread = Thread(target=self.backgroundThread)
            self.thread.start()
            # Block till we start receiving values
            while self.isReceiving != True:
                time.sleep(0.1)

    def getSerialData(self, frame, ax, lines, lineValueText, lineLabel, intervalText, timeRange):
        # make a copy of the current data in the buffer
        currData = copy.deepcopy(self.rawData[:])
        if currData == self.prevData:
            # data has not been updated, no need to update plot
            return

        # evaluate the time since previous update
        currentTimer = time.perf_counter()
        # the first timer reading will be erroneous
        self.plotTimer = int((currentTimer - self.previousTimer) * 1000)
        self.previousTimer = currentTimer
        intervalText.set_text('Plot Interval = ' + str(self.plotTimer) + 'ms')

        # packet structure:
        # byte 0-1: validation (2 uint8_t)
        # byte 2-3: data id (uint16_t)
        # byte 4-11: data (4 uint16_t)
        # byte 12-15: time (uint32_t)

        # check valid packet
        valid = currData[:2]
        # check data id
        dataId = struct.unpack('h', currData[2:4])[0]
        if (dataId not in range(self.numDataTypes) or valid[0] != 0x00 or valid[1] != 0xff):
            # data is bad, reset buffer
            self.serialConnection.reset_input_buffer()
            self.prevData = None
            return

        # read the rest of the packet
        # convert milliseconds to seconds
        newTime = struct.unpack('L', currData[12:])[0] / 1000
        self.data[dataId][0].append(newTime)
        for j in range(self.typeNumVals[dataId]):
            # put each data value in the corresponding array
            value = struct.unpack(
                'h', currData[4+j*self.valNumBytes:4+(j+1)*self.valNumBytes])[0]
            value *= self.typeValMultiplier[dataId]
            self.data[dataId][j+1].append(value)
            lines[dataId][j].set_data(
                self.data[dataId][0], self.data[dataId][j+1])
            lineValueText[dataId][j].set_text(f"[{lineLabel[dataId][j]}] = {value:.1f}")


        # update x bounds to most recent data
        if self.prevData == None:
            self.firstTime = newTime
            ax[0].set_xbound(self.firstTime, self.firstTime + timeRange)
        elif (newTime - self.firstTime > timeRange):
            ax[0].set_xbound(newTime - timeRange, newTime)
        self.prevData = currData

    def backgroundThread(self):  # retrieve data continuously
        time.sleep(1.0)  # give some buffer time for retrieving data
        self.serialConnection.reset_input_buffer()
        while (self.isRun):
            self.serialConnection.readinto(self.rawData)
            self.isReceiving = True

    def close(self):
        self.isRun = False
        self.thread.join()
        self.serialConnection.close()
        print('Disconnected')


class Anim:
    def __init__(self, animationInit):
        self.anim = animationInit
        self.paused = False

    def togglePause(self, event):
        if self.paused:
            self.anim.resume()
        else:
            self.anim.pause()
        self.paused = not self.paused


def main():
    portName = sys.argv[1]  # input port used as command line argument
    baudRate = 38400
    maxPlotLength = 1000  # max length of the data arrays
    timeRange = 20  # in seconds
    packetNumBytes = 16
    numDataTypes = 2
    typeNumVals = [4, 4]  # number of data values for each data type
    valNumBytes = 2  # short
    # multiplier to apply to each value to convert units
    typeValMultiplier = [1, 0.1]
    s = serialPlot(portName, baudRate, maxPlotLength, packetNumBytes,
                   numDataTypes, typeNumVals, valNumBytes, typeValMultiplier)  # initialize variables
    s.readSerialStart()  # starts background thread
    time.sleep(1.5)

    # plotting starts below
    pltInterval = 10  # period at which the plot animation updates [ms]

    # bounds
    xmin, xmax = (0, timeRange)
    ybounds = [[0, 100], [0, 200]]  # value bounds for each data type

    fig, ax = plt.subplots(numDataTypes, sharex=True)
    fig.set_figheight(5.4)  # can change for larger figure

    # labels and colors
    plt.suptitle("Live Telemetry Data")
    plotLabel = ['Wheel Speed (CPS)',
                 'Cooling Loop Temperature/Pressure (C/PSI)']
    legLabel = [['Front Right', 'Front Left', 'Rear Right', 'Rear Left'],
                ['Inlet Temp', 'Outlet Temp', 'Inlet Pres', 'Outlet Pres']]
    lineLabel = [['FR', 'FL', 'RR', 'RL'], ['IT', 'OT', 'IP', 'OP']]
    style = ['r-', 'g-', 'b-', 'y-']  # colors for the different lines

    # time axis
    ax[0].set_xbound(xmin, xmax)
    ax[numDataTypes - 1].set_xlabel("Time (s)")

    lines = []
    lineValueText = []
    for i in range(numDataTypes):
        # data axis
        ax[i].set_ybound(ybounds[i])
        ax[i].set_ylabel(plotLabel[i])

        lines.append([])
        lineValueText.append([])
        for j in range(typeNumVals[i]):
            # create line objects to be updated in loop
            lines[i].append(ax[i].plot([], [], style[j],)[0])
            lineValueText[i].append(ax[i].text(
                0.80, 0.90-j*0.08, '', transform=ax[i].transAxes))  # text positioning
        ax[i].legend(legLabel[i], loc="upper left")
    intervalText = ax[0].text(0.35, 1.1, '', transform=ax[0].transAxes)

    # initialize animation that will continuously update until closed
    anim = Anim(animation.FuncAnimation(fig, s.getSerialData, fargs=(
        ax, lines, lineValueText, lineLabel, intervalText, timeRange), interval=pltInterval, cache_frame_data=False))
    fig.canvas.mpl_connect('key_press_event', anim.togglePause)
    plt.show()
    s.close()


if __name__ == '__main__':
    main()
