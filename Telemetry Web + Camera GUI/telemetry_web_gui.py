from nicegui import app, ui
import plotly.graph_objects as go

#FRUCD Brand 2.0 Colors
FRUCD_DARK_BLUE = '#003a70'

app.add_static_files('/static', 'static')

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

    ui.image('http://localhost:8080/no_gui').style('margin:auto; height:85%; width:85%')

@ui.page('/all_data')
def all_data():
    frucd_repeat_background()
    main_navigation_menu()

    cols = 5
    with ui.grid(columns=cols):
        fig = go.Figure(go.Scatter(x=[1,2,3,4], y=[1,3,2,4]))
        ui.plotly(fig)


# TODO: ADD MAP, PLOTLY, LEFT/RIGHT SHELF, LOG VIEW

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
        ui.button('Data Explorer').classes(f'!bg-[{FRUCD_DARK_BLUE}]')

ui.run(port=8000, show=False)