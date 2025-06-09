import os
import time
import csv
import socket
import asyncio
import itertools
import threading
from nicegui import app, ui, events, run
import plotly.graph_objects as go
from camera_feed import CameraFeed
from telemetry_dbc_decoder import DBCDecoder
from telemetry_web_gui_graph_manager import DashboardData, PEIData, TNodeData
import biscuit_trimmed
import savvy_can_parser
from plotly_resampler import FigureResampler

# FRUCD Brand 2.0 Colors
FRUCD_DARK_BLUE = '#003a70'

CAMERA_STREAM_IP = "localhost"

app.add_static_files('/static', 'static')

# camera steam initialization
camera_stream = None
if __name__ == "__main__":
    camera_stream = CameraFeed(stream_ip = CAMERA_STREAM_IP)

# DBC CAN Decoder initialization
dbc_can_decoder = None
if __name__ == "__main__":
    dbc_can_decoder = DBCDecoder()

# Graphs initialization
dashboard_stats = DashboardData()
pei_stats = PEIData()
t_node_stats = TNodeData()

def distribute_can():
    def distribute_data():
        # TODO: CHANGE FROM POLLING TO EVENT-DRIVEN
        # TODO: CONSIDER CHANGING TEXT DATA TO MARKDOWN
        # TODO: TRY SENDING 10+ CAN MESSAGES AT ONCE INSTEAD OF EVENT-DRIVEN
        # TODO: MULTITHREADING COULD ALSO SOLVE PROBLEM
        # TODO: MAKE CENTRAL UPDATE FUNCTION, SPLIT EACH GRAPH FUNCTION INTO INIT AND UPDATE WITH CONDITIONAL PARAMETER, KEEP IN MIND LOCAL VARS
        while True:
            if not dbc_can_decoder.can_queue.empty():
                can_data = dbc_can_decoder.can_queue.get()
                match can_data["CAN_MSG_Name"]:
                    case "Dashboard_Vehicle_State":
                        dashboard_stats.UpdateGraphs(can_data)
                        camera_stream.data_queue.put(can_data)
                    case "PEI_BMS_Status":
                        pei_stats.UpdateGraphs(can_data)
                    case "PEI_Status":
                        pei_stats.UpdateGraphs(can_data)
                    case "TelemNode_Cooling_Loop_Temps":
                        t_node_stats.UpdateGraphs(can_data)
                    case "TelemNode_Cool_Loop_Pressures":
                        t_node_stats.UpdateGraphs(can_data)
                    case "TelemNode_Wheel_Speed_Rear":
                        t_node_stats.UpdateGraphs(can_data)
                    case "Dashboard_Torque_Request":
                        dashboard_stats.UpdateGraphs(can_data)
                        camera_stream.data_queue.put(can_data)
                    case "Dashboard_Random_Shit":
                        dashboard_stats.UpdateGraphs(can_data)
                    case "PEI_Diagnostic_BMS_Data":
                        pei_stats.UpdateGraphs(can_data)
                    case "TelemNode_Strain_Gauges_Rear":
                        t_node_stats.UpdateGraphs(can_data)

    thread = threading.Thread(target=distribute_data)
    thread.start()

def frucd_repeat_background():
    ui.add_head_html("<style>body {background-image: url('/static/FRUCD_logo.png'); background-size: 5%;}</style>")

def main_navigation_menu():
    with ui.button(icon='menu') as button:
        button.classes(f'!bg-[{FRUCD_DARK_BLUE}]')
        with ui.menu():
            ui.menu_item('Home', on_click=lambda: ui.navigate.to('/', new_tab=False))
            with ui.menu_item('Live PCAN Data', auto_close=False):
                with ui.item_section().props('side'):
                    ui.icon('keyboard_arrow_right')
                with ui.menu().props("anchor='top end' self='top start' auto-close"):
                    ui.menu_item('Dashboard Data', on_click=lambda: ui.navigate.to('/dashboard_data', new_tab=False))
                    ui.menu_item('PEI Data', on_click=lambda: ui.navigate.to('/pei_data', new_tab=False))
                    ui.menu_item('T-Node Data', on_click=lambda: ui.navigate.to('/t_node_data', new_tab=False)) # TODO: ORDER DATA BY CAN SHEET, NOT SUB-TEAM
                    ui.menu_item('Motor Controller Data', on_click=lambda: ui.navigate.to('/motor_controller_data', new_tab=False))
                    ui.menu_item('Custom')
            with ui.menu_item('Camera Feeds', auto_close=False):
                with ui.item_section().props('side'):
                    ui.icon('keyboard_arrow_right')
                with ui.menu().props("anchor='top end' self='top start' auto-close"):
                    ui.menu_item('GUI', on_click=lambda: ui.navigate.to('/camera_feed_gui', new_tab=False))
                    ui.menu_item('No GUI', on_click=lambda: ui.navigate.to('/camera_feed_no_gui', new_tab=False))
            with ui.menu_item('Data Tools', auto_close=False):
                with ui.item_section().props('side'):
                    ui.icon('keyboard_arrow_right')
                with ui.menu().props("anchor='top end' self='top start' auto-close"):
                    ui.menu_item('Data Explorer', on_click=lambda: ui.navigate.to('/data_explorer', new_tab=False))
                    ui.menu_item('Data Parser', on_click=lambda: ui.navigate.to('/data_parser', new_tab=False))

@ui.page('/camera_feed_gui')
def camera_feed_gui():
    frucd_repeat_background()
    main_navigation_menu()

    ui.interactive_image(f'http://{CAMERA_STREAM_IP}:8080/gui').style('margin:auto; height:60%; width:60%')

    with ui.row().classes('w-full justify-center'):
        ui.button("Toggle Recording", on_click=lambda: camera_stream.ToggleRecording() if __name__ == "__main__" else None).classes(f'!bg-[{FRUCD_DARK_BLUE}]')

@ui.page('/camera_feed_no_gui')
def camera_feed_no_gui():
    frucd_repeat_background()
    main_navigation_menu()

    ui.interactive_image(f'http://{CAMERA_STREAM_IP}:8080/no_gui').style('margin:auto; height:60%; width:60%')

@ui.page('/dashboard_data')
def dashboard_data():
    frucd_repeat_background()
    main_navigation_menu()

    with ui.row().classes('flex justify-center items-start w-screen'):
        with ui.card(align_items='center'):
            dashboard_stats.LabelDataInit()

    with ui.grid(columns=1).classes('w-full'):
        dashboard_stats.GraphsInit()


@ui.page('/pei_data')
def pei_data():
    frucd_repeat_background()
    main_navigation_menu()

    with ui.row().classes('flex justify-center items-start w-screen'):
        with ui.card(align_items='center'):
            ui.button()

    with ui.grid(columns=1).classes('w-full'):
        pei_stats.GraphsInit()

@ui.page('/t_node_data')
def t_node_data():
    frucd_repeat_background()
    main_navigation_menu()

    with ui.row().classes('flex justify-center items-start w-screen'):
        with ui.card(align_items='center'):
            ui.button()

    with ui.grid(columns=1).classes('w-full'):
        t_node_stats.GraphsInit()

@ui.page('/motor_controller_data')
def motor_controller_data():
    frucd_repeat_background()
    main_navigation_menu()

    with ui.row().classes('flex justify-center items-start w-screen'):
        with ui.card(align_items='center'):
            ui.button()

    #with ui.grid(columns=1).classes('w-full'):


@ui.page('/data_explorer')
async def data_explorer():
    await ui.context.client.connected()

    frucd_repeat_background()
    main_navigation_menu()

    # TODO: ADD DATA REPLAY

    curr_directory = app.storage.tab.get('directory', 'static')
    def set_curr_directory(directory):
        app.storage.tab['directory'] = directory
        ui.navigate.reload()

    with ui.card(align_items='center') as card:
        card.classes('fixed-center')
        card.style('width:35%')

        explorer_columns = [{'headerName':'File Name', 'field':'file_name', 'checkboxSelection':True, 'align':'left'}]
        file_names = [f for f in os.listdir(curr_directory) if os.path.isfile(os.path.join(curr_directory, f))]
        explorer_rows = []
        for i in range(len(file_names)):
            explorer_rows.append({'file_name':file_names[i]})
        explorer = ui.aggrid({'columnDefs':explorer_columns, 'rowData': explorer_rows, 'rowSelection':'multiple'})

        async def download_selected_files():
            selected_files = await explorer.get_selected_rows()
            if selected_files:
                ui.notify(f"Downloading {len(selected_files)} File(s)", close_button="Dismiss")
                for files in selected_files:
                    ui.download(f"{curr_directory}/{files['file_name']}")
            else:
                ui.notify("No Files Selected", type='negative', close_button='Dismiss')

        async def preview_selected_file():
            selected_files = await explorer.get_selected_rows()
            if selected_files:
                if len(selected_files) != 1:
                    ui.notify("Multiple File Preview Unsupported", type='warning', close_button='Dismiss')
                else:
                    # TODO: ADD DIFFERENT SCATTERS FOR EACH COLUMN
                    with open(f"{curr_directory}/{selected_files[0]['file_name']}", newline='') as csv_file:
                        reader = list(csv.reader(csv_file))
                        data = list(map(list, itertools.zip_longest(*reader, fillvalue=None))) # transposes csv
                        fig = go.Figure(go.Scatter(y=data[4][:]))
                        fig.update_layout(margin=dict(l=20, r=20, t=20, b=20))
                        plot = ui.plotly(fig)

            else:
                ui.notify("No File Selected", type='negative', close_button='Dismiss')

        with ui.row():
            with ui.dropdown_button('Directories', auto_close=True) as ddb:
                ddb.classes(f'!bg-[{FRUCD_DARK_BLUE}]')
                ui.item('Static', on_click=lambda: set_curr_directory('static'))
                ui.item('CAN Data', on_click=lambda: set_curr_directory('/home/frucd/projects/Raspi-TelemHost-Firmware-FE12/logs'))
                ui.item('Camera Stream Recordings', on_click=lambda: set_curr_directory('stream_recordings'))

            ui.button("Download Selected Files", on_click=download_selected_files).classes(f'!bg-[{FRUCD_DARK_BLUE}]')
            # ui.button("Preview Selected File", on_click=preview_selected_file).classes(f'!bg-[{FRUCD_DARK_BLUE}]') # TODO: FIX

@ui.page('/data_parser')
def data_parser():
    frucd_repeat_background()
    main_navigation_menu()

    fig = FigureResampler(go.Figure())
    ran_already = False
    plot = None
    format_selection = None

    async def graph_parsed_binary(e: events.UploadEventArguments):
        nonlocal ran_already, plot, format_selection
        if not ran_already:
            plot = ui.plotly(fig)
        ran_already = True
        fig.data = []

        if format_selection.value == 1:
            data = await run.cpu_bound(biscuit_trimmed.parse_biscuit, e.content.read())
        elif format_selection.value == 2:
            data = await run.cpu_bound(savvy_can_parser.parse_savvy, e.content.read().decode('utf-8'))

        fig.update_layout(margin=dict(l=10, r=10, t=10, b=10),
                          autosize=True,
                          title_text=e.name.title(),
                          title_automargin=True
                          )

        print(data)

        for i in range(len(data[0])):
            col_data = [row[i] for row in data]
            fig.add_trace(go.Scattergl(y=col_data))

        plot.update()


    with ui.card(align_items='center') as card:
        card.classes('fixed-center')

        ui.markdown('###**Data Parser**')
        ui.markdown('Parses Car Data and Graphs Data<br />'
                    'To use, upload file, choose format, then click the cloud icon to parse.').style('text-align: center')
        ui.separator()

        format_selection = ui.toggle({1: 'Biscuit', 2: 'Savvy'}, value=1)

        ui.upload(on_upload=graph_parsed_binary).classes('w-full')

# TODO: ADD MAP, PLOTLY, LEFT/RIGHT SHELF, LOG VIEW

@ui.page('/')
def home_page():
    frucd_repeat_background()

    with ui.card(align_items='center').classes('fixed-center'):
        ui.image('/static/FRUCD_GD_White(1).png')
        with ui.row():
            with ui.dropdown_button('Live PCAN Data', auto_close=True).classes(f'!bg-[{FRUCD_DARK_BLUE}]'):
                ui.item('Dashboard Data', on_click=lambda: ui.navigate.to('/dashboard_data', new_tab=False))
                ui.item('PEI Data', on_click=lambda: ui.navigate.to('/pei_data', new_tab=False))
                ui.item('T-Node Data', on_click=lambda: ui.navigate.to('/t_node_data', new_tab=False))
                ui.item('Motor Controller Data', on_click=lambda: ui.navigate.to('/motor_controller_data', new_tab=False))
                ui.item('Custom')
            with ui.dropdown_button('Camera Feeds', auto_close=True).classes(f'!bg-[{FRUCD_DARK_BLUE}]'):
                ui.item('GUI', on_click=lambda: ui.navigate.to('/camera_feed_gui', new_tab=False))
                ui.item('No GUI', on_click=lambda: ui.navigate.to('/camera_feed_no_gui', new_tab=False))
            with ui.dropdown_button('Data Tools', auto_close=True).classes(f'!bg-[{FRUCD_DARK_BLUE}]'):
                ui.item('Data Explorer', on_click=lambda: ui.navigate.to('/data_explorer', new_tab=False))
                ui.item('Data Parser', on_click=lambda: ui.navigate.to('/data_parser', new_tab=False))

ui.timer(0.1, distribute_can, once=True)
ui.run(port=8000, show=False, reload=False)