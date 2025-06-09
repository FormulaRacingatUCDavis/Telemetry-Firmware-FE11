import can
import time
import cantools
import multiprocessing as mp

class DBCDecoder:
    def __init__(self):
        self.can_queue = mp.Queue()
        self.can_process = mp.Process(target=self.ReadCan, args=(self.can_queue,))
        self.can_process.start()

    def ReadCan(self, can_queue):
        fe12_dbc_db = cantools.database.load_file('FE12_DBC.dbc')
        mc_dbc_db = cantools.database.load_file('20240129 Gen5 CAN DB.dbc')
        can_bus = can.interface.Bus(channel='can0', interface='socketcan')

        while True:
            can_msg = can_bus.recv()
            try:
                can_msg_name = fe12_dbc_db.get_message_by_frame_id(can_msg.arbitration_id).name
                can_msg_decoded = fe12_dbc_db.decode_message(can_msg.arbitration_id, can_msg.data)
                can_msg_decoded["CAN_MSG_Name"] = can_msg_name
            except KeyError:
                try:
                    can_msg_name = mc_dbc_db.get_message_by_frame_id(can_msg.arbitration_id).name
                    can_msg_decoded = mc_dbc_db.decode_message(can_msg.arbitration_id, can_msg.data)
                    can_msg_decoded["CAN_MSG_Name"] = can_msg_name
                except KeyError:
                    pass

            can_queue.put(can_msg_decoded)
