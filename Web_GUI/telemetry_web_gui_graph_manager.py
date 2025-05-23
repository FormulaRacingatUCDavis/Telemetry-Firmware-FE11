import asyncio
from queue import Queue
from nicegui import app, ui
import plotly.graph_objects as go
import time

# CAN info taken from FE12 CAN Index: https://docs.google.com/spreadsheets/d/1r-51IrmEZ4_uZa8dfQ3R4SaJ1A9HKPQQGiPVquRnGhc/edit?gid=0#gid=0

class DashboardGraphs:
    def __init__(self):
        self.MAX_DATA_POINTS = 50
        self.MAX_QUEUE_SIZE = 5
        self.UPDATE_INTERVAL = 0.1

        # Torque Request
        self.torque_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.speed_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)

        # Random Shit for Testing
        self.front_strain_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.front_wheel_speed_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.tc_torque_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)


    def GraphsInit(self):
        self.TorqueGraph()
        self.SpeedGraph()
        self.FrontStrainGraph()
        self.FrontWheelSpeedGraph()
        self.TCTorqueGraph()

    def UpdateGraphs(self, can_data):
        if can_data.get("Torque") != None:
            self.torque_queue.put(can_data["Torque"])
        if can_data.get("Speed") != None:
            self.speed_queue.put(can_data["Speed"])
        if can_data.get("Front_Strain_Gauge") != None:
            self.front_strain_queue.put(can_data["Front_Strain_Gauge"])
        if can_data.get("Front_Wheel_Speed") != None:
            self.front_wheel_speed_queue.put(can_data["Front_Wheel_Speed"])
        if can_data.get("TC_Torque_Request") != None:
            self.tc_torque_queue.put(can_data["TC_Torque_Request"])

    def TorqueGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Torque (Nm * 10)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.torque_queue.empty():
                    data.append(self.torque_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def SpeedGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Speed (RPM)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.speed_queue.empty():
                    data.append(self.speed_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def FrontStrainGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Front Strain Gauge (Raw ADC)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.front_strain_queue.empty():
                    data.append(self.front_strain_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def FrontWheelSpeedGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Front Wheel Speed (RPM)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.front_wheel_speed_queue.empty():
                    data.append(self.front_wheel_speed_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def TCTorqueGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="TC Torque Request (Nm * 10)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.tc_torque_queue.empty():
                    data.append(self.tc_torque_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

class PEIGraphs:
    def __init__(self):
        self.MAX_DATA_POINTS = 50
        self.MAX_QUEUE_SIZE = 5
        self.UPDATE_INTERVAL = 0.01

        # PEI Status
        self.current_adc_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.current_reference_adc_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)

        #Diagnostic BMS Data
        self.bms_hi_temp_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.bms_soc_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.bms_pack_voltage_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)

    def GraphsInit(self):
        self.CurrentADCGraph()
        self.CurrentReferenceADCGraph()
        self.BMSHITempGraph()
        self.BMSSOCGraph()
        self.BMSPackVoltageGraph()

    def UpdateGraphs(self, can_data):
        if can_data.get("Current_ADC") != None:
            self.current_adc_queue.put(can_data["Current_ADC"])
        if can_data.get("Current_Reference") != None:
            self.current_reference_adc_queue.put(can_data["Current_Reference"])
        if can_data.get("HI_Temp") != None:
            self.bms_hi_temp_queue.put(can_data["HI_Temp"])
        if can_data.get("SOC") != None:
            self.bms_soc_queue.put(can_data["SOC"])
        if can_data.get("Pack_Voltage") != None:
            self.bms_pack_voltage_queue.put(can_data["Pack_Voltage"])

    def CurrentADCGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Current ADC Reading",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.current_adc_queue.empty():
                    data.append(self.current_adc_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def CurrentReferenceADCGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Current Reference ADC Reading",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.current_reference_adc_queue.empty():
                    data.append(self.current_reference_adc_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def BMSHITempGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="BMS HI Temp (C)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.bms_hi_temp_queue.empty():
                    data.append(self.bms_hi_temp_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def BMSSOCGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="BMS SOC (%)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.bms_soc_queue.empty():
                    data.append(self.bms_soc_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def BMSPackVoltageGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="BMS Pack Voltage",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.bms_pack_voltage_queue.empty():
                    data.append(self.bms_pack_voltage_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

class TNodeGraphs:
    def __init__(self):
        self.MAX_DATA_POINTS = 50
        self.MAX_QUEUE_SIZE = 5
        self.UPDATE_INTERVAL = 0.01

        # Cooling Loop Temps
        self.inlet_water_temp_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.outlet_water_temp_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.air_in_rad_temp_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.air_out_rad_temp_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)

        # Wheel Speed Rear
        self.wheel_speed_rr_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.wheel_speed_rl_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)

        # Cooling Loop Pressures
        self.inlet_water_press_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.outlet_water_press_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)

        # Strain Gauge Rear
        self.rl_toe_strain_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.rluf_aarm_strain_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.rlub_aarm_strain_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.rllf_aarm_strain_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)

    def GraphsInit(self):
        self.InletWaterTempGraph()
        self.OutletWaterTempGraph()
        self.AirInRadTempGraph()
        self.AirOutRadTempGraph()
        self.WheelSpeedRRGraph()
        self.WheelSpeedRLGraph()
        self.InletWaterPressGraph()
        self.OutletWaterPressGraph()
        self.RLToeStrainGraph()
        self.RLUBAArmStrainGraph()
        self.RLUBAArmStrainGraph()
        self.RLLFAArmStrainGraph()

    def UpdateGraphs(self, can_data):
        if can_data.get("Inlet_Water_Temp") != None:
            self.inlet_water_temp_queue.put(can_data["Inlet_Water_Temp"])
        if can_data.get("Outlet_Water_Temp") != None:
            self.outlet_water_temp_queue.put(can_data["Outlet_Water_Temp"])
        if can_data.get("Air_Into_Radiator_Temp") != None:
            self.air_in_rad_temp_queue.put(can_data["Air_Into_Radiator_Temp"])
        if can_data.get("Air_Out_Of_Radiator_Temp") != None:
            self.air_out_rad_temp_queue.put(can_data["Air_Out_Of_Radiator_Temp"])
        if can_data.get("Wheel_Speed_Rear_Right") != None:
            self.wheel_speed_rr_queue.put(can_data["Wheel_Speed_Rear_Right"])
        if can_data.get("Wheel_Speed_Rear_Left") != None:
            self.wheel_speed_rl_queue.put(can_data["Wheel_Speed_Rear_Left"])
        if can_data.get("Inlet_Water_Pressure") != None:
            self.inlet_water_press_queue.put(can_data["Inlet_Water_Pressure"])
        if can_data.get("Outlet_Water_Pressure") != None:
            self.outlet_water_press_queue.put(can_data["Outlet_Water_Pressure"])
        if can_data.get("RL_Toe_Strain_Gauge") != None:
            self.rl_toe_strain_queue.put(can_data["RL_Toe_Strain_Gauge"])
        if can_data.get("RLUF_A_Arm_Strain_Gauge") != None:
            self.rluf_aarm_strain_queue.put(can_data["RLUF_A_Arm_Strain_Gauge"])
        if can_data.get("RLUB_A_Arm_Strain_Gauge") != None:
            self.rlub_aarm_strain_queue.put(can_data["RLUB_A_Arm_Strain_Gauge"])
        if can_data.get("RLLF_A_Arm_Strain_Gauge") != None:
            self.rllf_aarm_strain_queue.put(can_data["RLLF_A_Arm_Strain_Gauge"])

    def InletWaterTempGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Inlet Water Temp (C * 10)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.inlet_water_temp_queue.empty():
                    data.append(self.inlet_water_temp_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def OutletWaterTempGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Outlet Water Temp (C * 10)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.outlet_water_temp_queue.empty():
                    data.append(self.outlet_water_temp_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def AirInRadTempGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Air In Radiator Temp (C * 10)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.air_in_rad_temp_queue.empty():
                    data.append(self.air_in_rad_temp_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def AirOutRadTempGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Air Out Radiator Temp (C * 10)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.air_out_rad_temp_queue.empty():
                    data.append(self.air_out_rad_temp_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def WheelSpeedRRGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Wheel Speed RR (CPS)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.wheel_speed_rr_queue.empty():
                    data.append(self.wheel_speed_rr_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def WheelSpeedRLGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Wheel Speed RL (CPS)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.wheel_speed_rl_queue.empty():
                    data.append(self.wheel_speed_rl_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def InletWaterPressGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Inlet Water Pressure (PSI * 100)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.inlet_water_press_queue.empty():
                    data.append(self.inlet_water_press_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def OutletWaterPressGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="Outlet Water Press (PSI * 100)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.outlet_water_press_queue.empty():
                    data.append(self.outlet_water_press_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def RLToeStrainGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="RL Toe Strain Gauge (raw ADC)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.rl_toe_strain_queue.empty():
                    data.append(self.rl_toe_strain_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def RLUFAArmStrainGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="RLUF A-Arm Strain Gauge (raw ADC)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.rluf_aarm_strain_queue.empty():
                    data.append(self.rluf_aarm_strain_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def RLUBAArmStrainGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="RLUB A-Arm Strain Gauge (raw ADC)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.rlub_aarm_strain_queue.empty():
                    data.append(self.rlub_aarm_strain_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())

    def RLLFAArmStrainGraph(self):
        data = []
        fig = go.Figure()
        fig.add_scatter()
        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text="RLLF A-Arm Strain Gauge (raw ADC)",
                          title_automargin=True
                          )
        plot = ui.plotly(fig)

        async def update_graph():
            while True:
                if not self.rllf_aarm_strain_queue.empty():
                    data.append(self.rllf_aarm_strain_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]
                    plot.update()
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_graph())