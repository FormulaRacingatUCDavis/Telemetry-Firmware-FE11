import asyncio
import threading
from queue import Queue
from nicegui import app, ui
import plotly.graph_objects as go
import time

# CAN info taken from FE12 CAN Index: https://docs.google.com/spreadsheets/d/1r-51IrmEZ4_uZa8dfQ3R4SaJ1A9HKPQQGiPVquRnGhc/edit?gid=0#gid=0

class DashboardData:
    def __init__(self):
        self.MAX_DATA_POINTS = 50
        self.MAX_QUEUE_SIZE = 5
        self.UPDATE_INTERVAL = 1

        # Vehicle State
        self.hv_requested_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.throttle1_level_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.throttle2_level_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.brake_level_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.vehicle_state_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.throttle1_level_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.vcu_ticks_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)

        # Torque Request
        self.torque_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.speed_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.direction_command_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.inverter_enable_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.discharge_enable_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.speed_mode_enable_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.torque_limit_queue = Queue(maxsize=self.MAX_QUEUE_SIZE) # TODO: Finish label functions for green

        # Random Shit for Testing
        self.front_strain_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.front_wheel_speed_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)
        self.tc_torque_queue = Queue(maxsize=self.MAX_QUEUE_SIZE)

    def LabelDataInit(self):
        with ui.row().classes('items-center'):
            with ui.grid(columns=1):
                self.HVRequestedLabel()
                self.Throttle1LevelLabel()
                self.Throttle2LevelLabel()
                self.BrakeLevelLabel()
                self.DirectionCommandLabel()
                self.InverterEnableLabel()
                self.DischargeEnableLabel()
                self.SpeedModeEnableLabel()
                self.TorqueLimitLabel()
            with ui.grid(columns=1):
                self.VehicleStateLabel()

    def GraphsInit(self):
        self.TorqueGraph()
        self.SpeedGraph()
        self.FrontStrainGraph()
        self.FrontWheelSpeedGraph()
        self.TCTorqueGraph()

    def UpdateGraphs(self, can_data):
        self.TorqueGraph().update_graph()
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

    def HVRequestedLabel(self):
        label = ui.label("HV Requested: No Data")
        async def update_label():
            while True:
                if not self.hv_requested_queue.empty():
                    can_data = self.hv_requested_queue.get()
                    if can_data:
                        label.set_text("HV Requested: Requested")
                    else:
                        label.set_text("HV Requested: Not Requested")
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_label())

    def Throttle1LevelLabel(self):
        label = ui.label("Throttle 1 Level: No Data")
        async def update_label():
            while True:
                if not self.throttle1_level_queue.empty():
                    can_data = self.throttle1_level_queue.get()
                    label.set_text(f"Throttle 1 Level: {can_data}%")
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_label())

    def Throttle2LevelLabel(self):
        label = ui.label("Throttle 2 Level: No Data")
        async def update_label():
            while True:
                if not self.throttle2_level_queue.empty():
                    can_data = self.throttle2_level_queue.get()
                    label.set_text(f"Throttle 2 Level: {can_data}%")
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_label())

    def BrakeLevelLabel(self):
        label = ui.label("Brake Level: No Data")
        async def update_label():
            while True:
                if not self.brake_level_queue.empty():
                    can_data = self.brake_level_queue.get()
                    label.set_text(f"Brake Level: {can_data}%")
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_label())

    def VehicleStateLabel(self):
        ui.label("Vehicle State Log (Lowest Msg is Latest)")
        log = ui.log()
        async def update_label():
            while True:
                if not self.vehicle_state_queue.empty():
                    can_data = self.vehicle_state_queue.get()

                    match can_data:
                        case int('00', 16):
                            log.push(f"LV")
                        case int('01', 16):
                            log.push(f"Precharge")
                        case int('02', 16):
                            log.push(f"HV")
                        case int('03', 16):
                            log.push(f"Drive")
                        case int('05', 16):
                            log.push(f"Startup")
                        case int('81', 16):
                            log.push(f"FAULT: Drive Request from LV", classes='text-red')
                        case int('82', 16):
                            log.push(f"FAULT: Precharge Timeout", classes='text-red')
                        case int('83', 16):
                            log.push(f"FAULT: Brake Not Pressed", classes='text-red')
                        case int('84', 16):
                            log.push(f"FAULT: HV Disabled While Driving", classes='text-red')
                        case int('85', 16):
                            log.push(f"FAULT: Sensor Discrepancy", classes='text-red')
                        case int('86', 16):
                            log.push(f"FAULT: BSPD Tripped", classes='text-red')
                        case int('87', 16):
                            log.push(f"FAULT: Shutdown Circuit Open", classes='text-red')
                        case int('88', 16):
                            log.push(f"FAULT: Uncalibrated", classes='text-red')
                        case int('89', 16):
                            log.push(f"FAULT: Hard BSPD", classes='text-red')
                        case int('8A', 16):
                            log.push(f"FAULT: MC Fault", classes='text-red')
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_label())

    def VCUTicksLabel(self):
        label = ui.label("VCU Ticks: No Data")
        async def update_label():
            while True:
                if not self.vcu_ticks_queue.empty():
                    can_data = self.vcu_ticks_queue.get()
                    label.set_text(f"VCU Ticks: {can_data} ms")
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_label())

    def DirectionCommandLabel(self):
        label = ui.label("Direction Command: No Data")
        async def update_label():
            while True:
                if not self.direction_command_queue.empty():
                    can_data = self.direction_command_queue.get()
                    if can_data:
                        label.set_text("Direction Command: Forward")
                    else:
                        label.set_text("Direction Command: Reverse")
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_label())

    def InverterEnableLabel(self):
        label = ui.label("Inverter Enable: No Data")
        async def update_label():
            while True:
                if not self.inverter_enable_queue.empty():
                    can_data = self.inverter_enable_queue.get()
                    if can_data:
                        label.set_text("Inverter Enable: Enable")
                    else:
                        label.set_text("Inverter Enable: Disable")
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_label())

    def DischargeEnableLabel(self):
        label = ui.label("Discharge Enable: No Data")
        async def update_label():
            while True:
                if not self.discharge_enable_queue.empty():
                    can_data = self.discharge_enable_queue.get()
                    if can_data:
                        label.set_text("Discharge Enable: Enable")
                    else:
                        label.set_text("Discharge Enable: Disable")
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_label())

    def SpeedModeEnableLabel(self):
        label = ui.label("Speed Mode Enable: No Data")
        async def update_label():
            while True:
                if not self.speed_mode_enable_queue.empty():
                    can_data = self.speed_mode_enable_queue.get()
                    if can_data:
                        label.set_text("Speed Mode Enable: Enable")
                    else:
                        label.set_text("Speed Mode Enable: Disable")
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_label())

    def TorqueLimitLabel(self):
        label = ui.label("Torque Limit: No Data")
        async def update_label():
            while True:
                if not self.torque_limit_queue.empty():
                    can_data = self.torque_limit_queue.get()
                    label.set_text(f"Torque Limit: {can_data}")
                await asyncio.sleep(self.UPDATE_INTERVAL)

        asyncio.create_task(update_label())

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.torque_queue.empty():
                    data.append(self.torque_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.speed_queue.empty():
                    data.append(self.speed_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.front_strain_queue.empty():
                    data.append(self.front_strain_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.front_wheel_speed_queue.empty():
                    data.append(self.front_wheel_speed_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.tc_torque_queue.empty():
                    data.append(self.tc_torque_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

class PEIData:
    def __init__(self):
        self.MAX_DATA_POINTS = 50
        self.MAX_QUEUE_SIZE = 0
        self.UPDATE_INTERVAL = 1
        self.thread_stop_flag = False

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

        def update_graph():
            prev_time = time.time()
            while not self.thread_stop_flag:
                if not self.current_adc_queue.empty():
                    data.append(self.current_adc_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                if time.time() - prev_time >= self.UPDATE_INTERVAL:
                    plot.update()
                    prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()
        # thread.join()

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

        def update_graph():
            prev_time = time.time()
            while self.thread_stop_flag:
                if not self.current_reference_adc_queue.empty():
                    data.append(self.current_reference_adc_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                if time.time() - prev_time >= self.UPDATE_INTERVAL:
                    plot.update()
                    prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()
        # thread.join()

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

        def update_graph():
            prev_time = time.time()
            while not self.thread_stop_flag:
                if not self.bms_hi_temp_queue.empty():
                    data.append(self.bms_hi_temp_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                if time.time() - prev_time >= self.UPDATE_INTERVAL:
                    plot.update()
                    prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()
        # thread.join()

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

        def update_graph():
            prev_time = time.time()
            while not self.thread_stop_flag:
                if not self.bms_soc_queue.empty():
                    data.append(self.bms_soc_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                if time.time() - prev_time >= self.UPDATE_INTERVAL:
                    plot.update()
                    prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()
        # thread.join()

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

        def update_graph():
            prev_time = time.time()
            while not self.thread_stop_flag:
                if not self.bms_pack_voltage_queue.empty():
                    data.append(self.bms_pack_voltage_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                if time.time() - prev_time >= self.UPDATE_INTERVAL:
                    plot.update()
                    prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()
        # thread.join()

class TNodeData:
    def __init__(self):
        self.MAX_DATA_POINTS = 50
        self.MAX_QUEUE_SIZE = 5
        self.UPDATE_INTERVAL = 1

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.inlet_water_temp_queue.empty():
                    data.append(self.inlet_water_temp_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.outlet_water_temp_queue.empty():
                    data.append(self.outlet_water_temp_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.air_in_rad_temp_queue.empty():
                    data.append(self.air_in_rad_temp_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.air_out_rad_temp_queue.empty():
                    data.append(self.air_out_rad_temp_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.wheel_speed_rr_queue.empty():
                    data.append(self.wheel_speed_rr_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.wheel_speed_rl_queue.empty():
                    data.append(self.wheel_speed_rl_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.inlet_water_press_queue.empty():
                    data.append(self.inlet_water_press_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.outlet_water_press_queue.empty():
                    data.append(self.outlet_water_press_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.rl_toe_strain_queue.empty():
                    data.append(self.rl_toe_strain_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.rluf_aarm_strain_queue.empty():
                    data.append(self.rluf_aarm_strain_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.rlub_aarm_strain_queue.empty():
                    data.append(self.rlub_aarm_strain_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()

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

        def update_graph():
            prev_time = time.time()
            while True:
                if not self.rllf_aarm_strain_queue.empty():
                    data.append(self.rllf_aarm_strain_queue.get())
                    if len(data) > self.MAX_DATA_POINTS:
                        data.pop(0)

                    fig.data[0].y = data[:]

                    if time.time() - prev_time >= self.UPDATE_INTERVAL:
                        plot.update()
                        prev_time = time.time()

        thread = threading.Thread(target=update_graph)
        thread.start()