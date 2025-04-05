import os
import time
import csv
import asyncio
import itertools
from nicegui import app, ui
import plotly.graph_objects as go
from camera_feed import CameraFeed

# FRUCD Brand 2.0 Colors
FRUCD_DARK_BLUE = '#003a70'

CAMERA_STREAM_IP = 'localhost'

app.add_static_files('/static', 'static')

# camera steam initialization
camera_stream = None
if __name__ == "__main__":
    camera_stream = CameraFeed()

def frucd_repeat_background():
    ui.add_head_html("<style>body {background-image: url('/static/FRUCD_logo.png'); background-size: 5%;}</style>")

def main_navigation_menu():
    with ui.button(icon='menu') as button:
        button.classes(f'!bg-[{FRUCD_DARK_BLUE}]')
        with ui.menu():
            ui.menu_item('Home', on_click=lambda: ui.navigate.to('/', new_tab=False))
            with ui.menu_item('Live Data', auto_close=False):
                with ui.item_section().props('side'):
                    ui.icon('keyboard_arrow_right')
                with ui.menu().props("anchor='top end' self='top start' auto-close"):
                    ui.menu_item('All Data', on_click=lambda: ui.navigate.to('/all_data', new_tab=False))
                    ui.menu_item('Electrical')
                    ui.menu_item('Cooling')
                    ui.menu_item('Custom')
            with ui.menu_item('Camera Feeds', auto_close=False):
                with ui.item_section().props('side'):
                    ui.icon('keyboard_arrow_right')
                with ui.menu().props("anchor='top end' self='top start' auto-close"):
                    ui.menu_item('GUI', on_click=lambda: ui.navigate.to('/camera_feed_gui', new_tab=False))
                    ui.menu_item('No GUI', on_click=lambda: ui.navigate.to('/camera_feed_no_gui', new_tab=False))
            ui.menu_item('Data Explorer', on_click=lambda: ui.navigate.to('/data_explorer', new_tab=False))

@ui.page('/camera_feed_gui')
def camera_feed_gui():
    frucd_repeat_background()
    main_navigation_menu()

    ui.interactive_image(f'http://{CAMERA_STREAM_IP}:8080/gui').style('margin:auto; height:60%; width:60%')

@ui.page('/camera_feed_no_gui')
def camera_feed_no_gui():
    frucd_repeat_background()
    main_navigation_menu()

    ui.interactive_image(f'http://{CAMERA_STREAM_IP}:8080/no_gui').style('margin:auto; height:60%; width:60%')

@ui.page('/all_data')
def all_data():
    frucd_repeat_background()
    main_navigation_menu()

    #ui.slider(min=1, max=5, value=3)
    #cols = 5
    #with ui.grid(columns=cols):
    data = []
    fig = go.Figure()
    fig.add_scatter()
    fig.update_layout(margin=dict(l=20, r=20, t=20, b=20))
    plot = ui.plotly(fig)

    async def plotlyupdatetest():
        while True:
            data.append(time.time())
            fig.data[0].y = data[:]
            plot.update()
            await asyncio.sleep(0.1)

    asyncio.create_task(plotlyupdatetest())

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
                ui.item('CAN Data', on_click=lambda: set_curr_directory('csv_test_files'))
                ui.item('Camera Stream Recordings')

            ui.button("Download Selected Files", on_click=download_selected_files).classes(f'!bg-[{FRUCD_DARK_BLUE}]')
            ui.button("Preview Selected File", on_click=preview_selected_file).classes(f'!bg-[{FRUCD_DARK_BLUE}]')


# TODO: ADD MAP, PLOTLY, LEFT/RIGHT SHELF, LOG VIEW

@ui.page('/')
def main_page():
    frucd_repeat_background()

    with ui.card(align_items='center').classes('fixed-center'):
        ui.image('/static/FRUCD_GD_White(1).png')
        with ui.row():
            with ui.dropdown_button('Live Data', auto_close=True).classes(f'!bg-[{FRUCD_DARK_BLUE}]'):
                ui.item('All Data', on_click=lambda: ui.navigate.to('/all_data', new_tab=False))
                ui.item('Electrical')
                ui.item('Cooling')
                ui.item('Custom')
            with ui.dropdown_button('Camera Feeds', auto_close=True).classes(f'!bg-[{FRUCD_DARK_BLUE}]'):
                ui.item('GUI', on_click=lambda: ui.navigate.to('/camera_feed_gui', new_tab=False))
                ui.item('No GUI', on_click=lambda: ui.navigate.to('/camera_feed_no_gui', new_tab=False))
            ui.button('Data Explorer', on_click=lambda: ui.navigate.to('/data_explorer', new_tab=False)).classes(f'!bg-[{FRUCD_DARK_BLUE}]')

ui.run(port=8000, show=False)