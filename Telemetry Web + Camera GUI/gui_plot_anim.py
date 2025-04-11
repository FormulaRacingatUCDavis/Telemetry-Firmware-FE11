from nicegui import app, ui
import collections
import time
# -----------------------------------------------------

from threading import Thread
import serial
import struct
import copy
import sys
import serial.tools.list_ports


class serialPlot:
    def __init__(self, serialPort, serialBaud, plotLength, packetNumBytes, numDataTypes, typeNumVals, valNumBytes):
        self.port = serialPort
        self.baud = serialBaud
        self.packetNumBytes = packetNumBytes
        self.numDataTypes = numDataTypes
        self.typeNumVals = typeNumVals
        self.valNumBytes = valNumBytes
        self.rawData = bytearray(packetNumBytes)
        self.data = []
        for i in range(numDataTypes):
            self.data.append([])
            for _ in range(typeNumVals[i] + 1):
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
        self.chart = None
        self.pause = False
        self.intervalLabel = None

        print('Trying to connect to ' + str(serialPort) +
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

    def initCharts(self):
        # Create the ECharts component
        self.chart = ui.echart({
            # Initial x-axis data
            'xAxis': {
                'type': 'category',
                'data': list(self.data[0][0]),
                'name': 'Time (s)',
                'nameLocation': 'center',
                'nameGap': '30'
            },
            'yAxis': {
                'type': 'value',
                'name': 'Fake Data',
                'nameLocation': 'center',
                'nameGap': '30'
            },
            'series': [{
                'type': 'line',
                'data': list(self.data[0][1]),  # Initial y-axis data
                'smooth': False,
                # 'lineStyle': {'color': FRUCD_DARK_BLUE},  # Set line color
                'name': "FR",
                'symbol': 'none',
            }, {
                'type': 'line',
                'data': list(self.data[0][2]),  # Initial y-axis data
                'smooth': False,
                'name': "FL",
                'symbol': 'none',
            }, {
                'type': 'line',
                'data': list(self.data[0][3]),  # Initial y-axis data
                'smooth': False,
                'name': "RR",
                'symbol': 'none',
            }, {
                'type': 'line',
                'data': list(self.data[0][4]),  # Initial y-axis data
                'smooth': False,
                'name': "RL",
                'symbol': 'none',
            }],
            'title': {'text': 'Live Telemetry Data'},
            'tooltip': {'trigger': 'axis'},
            'legend': {},
            'grid': {
                'left': '3%',
                'right': '3%',
                'bottom': '10%',
                'containLabel': True
            },
        }).style('width: 100%; height: 400px;')  # Adjust chart size

        # self.chart.on('click', s.togglePause)

    def togglePause(self):
        self.pause = not self.pause

    def getSerialData(self):
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

        # packet structure:
        # byte 0-1: validation (2 uint8_t)
        # byte 2-3: data id (uint16_t)
        # byte 4-11: data (4 uint16_t)
        # byte 12-15: time (uint32_t)

        # check valid packet
        valid = currData[:2]
        # check data id
        dataId = struct.unpack('h', currData[2:4])[0]
        if (dataId not in range(self.numDataTypes) or valid[0]):
            # data is bad, reset buffer
            self.serialConnection.reset_input_buffer()
            self.prevData = None
            return

        # read the rest of the packet
        # convert milliseconds to seconds
        newTime = struct.unpack('L', currData[12:])[0] / 1000000
        self.data[dataId][0].append(float(f"{newTime:.2f}"))
        for j in range(self.typeNumVals[dataId]):
            # put each data value in the corresponding array
            value = struct.unpack(
                'h', currData[4+j*self.valNumBytes:4+(j+1)*self.valNumBytes])[0]
            self.data[dataId][j+1].append(value)

        self.chart.options['xAxis']['data'] = list(self.data[0][0])
        self.chart.options['series'][0]['data'] = list(self.data[0][1])
        self.chart.options['series'][1]['data'] = list(self.data[0][2])
        self.chart.options['series'][2]['data'] = list(self.data[0][3])
        self.chart.options['series'][3]['data'] = list(self.data[0][4])
        if (not self.pause):
            self.chart.update()
            self.intervalLabel.set_text(f'Plot Interval: {self.plotTimer} ms')

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


# -----------------------------------------------------
# FRUCD Brand 2.0 Colors
FRUCD_DARK_BLUE = '#003a70'

app.add_static_files('/static', 'static')


def frucd_repeat_background():
    ui.add_head_html(
        "<style>body {background-image: url('/static/FRUCD_logo.png'); background-size: 5%;}</style>")


def main_navigation_menu():
    with ui.button(icon='menu') as button:
        button.classes(f'!bg-[{FRUCD_DARK_BLUE}]')
        with ui.menu():
            ui.menu_item('Home', on_click=lambda: ui.navigate.to(
                '/', new_tab=False))
            with ui.menu_item('Live Data', auto_close=False):
                with ui.item_section().props('side'):
                    ui.icon('keyboard_arrow_right')
                with ui.menu().props("anchor='top end' self='top start' auto-close"):
                    ui.menu_item('All Data', on_click=lambda: ui.navigate.to(
                        '/all_data', new_tab=False))
                    ui.menu_item('Electrical')
                    ui.menu_item('Cooling')
                    ui.menu_item('Custom')
            with ui.menu_item('Camera Feeds', auto_close=False):
                with ui.item_section().props('side'):
                    ui.icon('keyboard_arrow_right')
                with ui.menu().props("anchor='top end' self='top start' auto-close"):
                    ui.menu_item('GUI', on_click=lambda: ui.navigate.to(
                        '/camera_feed_gui', new_tab=False))
                    ui.menu_item('No GUI', on_click=lambda: ui.navigate.to(
                        '/camera_feed_no_gui', new_tab=False))
            ui.menu_item('Data Explorer')


@ui.page('/camera_feed_gui')
def camera_feed_gui():
    frucd_repeat_background()
    main_navigation_menu()

    ui.image('http://localhost:8080/gui').style('margin:auto; height:85%; width:85%')


@ui.page('/camera_feed_no_gui')
def camera_feed_no_gui():
    frucd_repeat_background()
    main_navigation_menu()

    ui.image(
        'http://localhost:8080/no_gui').style('margin:auto; height:85%; width:85%')


@ui.page('/all_data')
def all_data():
    # frucd_repeat_background()
    main_navigation_menu()

    # Define constants
    # max_length = 100  # Rolling window size
    update_interval = 18  # Update interval in milliseconds

    # Set up a periodic timer to call the update function
    if s:
        if (s.chart == None):
            s.initCharts()
            s.intervalLabel = ui.label('Plot Interval: -- ms')
        ui.timer(update_interval / 1000, s.getSerialData)
    else:
        print("failed to initialize serialplot")

    ui.button('Pause/Resume', on_click=lambda: s.togglePause())

# TODO: ADD MAP, PLOTLY, LEFT/RIGHT SHELF, LOG VIEW


frucd_repeat_background()

with ui.card(align_items='center').classes('fixed-center'):
    ui.image('/static/FRUCD_GD_White(1).png')
    with ui.row():
        with ui.dropdown_button('Live Data', auto_close=True).classes(f'!bg-[{FRUCD_DARK_BLUE}]'):
            ui.item('All Data', on_click=lambda: ui.navigate.to(
                '/all_data', new_tab=False))
            ui.item('Electrical')
            ui.item('Cooling')
            ui.item('Custom')
        with ui.dropdown_button('Camera Feeds', auto_close=True).classes(f'!bg-[{FRUCD_DARK_BLUE}]'):
            ui.item('GUI', on_click=lambda: ui.navigate.to(
                '/camera_feed_gui', new_tab=False))
            ui.item('No GUI', on_click=lambda: ui.navigate.to(
                '/camera_feed_no_gui', new_tab=False))
        ui.button('Data Explorer').classes(f'!bg-[{FRUCD_DARK_BLUE}]')

ui.run(port=8000, show=False)

if (len(sys.argv) == 2):
    # input port given as command line argument
    portName = sys.argv[1]
    print("Port " + portName + " supplied.")
else:
    # find port in use
    ports = serial.tools.list_ports.comports()
    if (len(ports)) < 1:
        sys.exit("No port found")
    portName = ports.pop().name
    print("No port explicitly supplied, found port " + portName + ".")
baudRate = 38400
maxPlotLength = 200  # max length of the data arrays
# timeRange = 20  # in seconds
packetNumBytes = 16
numDataTypes = 2
typeNumVals = [4, 1]  # number of data values for each data type
valNumBytes = 2  # short
s = serialPlot(portName, baudRate, maxPlotLength, packetNumBytes,
               numDataTypes, typeNumVals, valNumBytes)  # initialize variables
s.readSerialStart()  # starts background thread
# time.sleep(1.5)
