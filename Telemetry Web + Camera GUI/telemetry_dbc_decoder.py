import can
import cantools
import multiprocessing as mp

# DBC file taken from FE11 Google Drive

class DBCDecoder:
    def __init__(self):
        self.can_queue = mp.Queue()
        self.can_process = mp.Process(target=self.ReadCan, args=(self.can_queue,))
        self.can_process.start()

    def ReadCan(self, can_queue):
        dbc_db = cantools.database.load_file('FRUCD_DBC.dbc')
        print(dbc_db)
        can_bus = can.interface.Bus( )  # TODO: Figure out params

        while True:
            can_msg = can_bus.recv()
            can_msg_decoded = dbc_db.decode_message(can_msg.arbitration_id, can_msg.data)

            can_queue.put(can_msg_decoded)