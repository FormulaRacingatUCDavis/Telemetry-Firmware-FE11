import cv2 as cv
import time
import multiprocessing as mp
import numpy as np
from mjpeg_streamer import MjpegServer, Stream
from pathlib import Path

class CameraFeed:
    def __init__(self, stream_ip="localhost", stream_port=8080, stream_quality=80, stream_fps=30):
        # camera stream
        self.STREAM_QUALITY = stream_quality
        self.STREAM_FPS = stream_fps
        self.STREAM_IP = stream_ip
        self.STREAM_PORT = stream_port
        self.record_stream = mp.Value('i', False)
        self.data_queue = mp.Queue()

        self.stream_process = mp.Process(target=self.CameraMain, args=(self.record_stream, self.data_queue))
        self.stream_process.start()

    def ToggleRecording(self):
        if self.record_stream.value:
            self.record_stream.value = False
        else:
            self.record_stream.value = True

    def CurrentDateTime(self):
        return time.strftime("%b_%d_%Y_%Hh_%Mm_%Ss", time.localtime())

    def AddImage(self, frame, image_src, x_pos, y_pos, scale=1.0):
        image = cv.imread(image_src)

        image_height = int(image.shape[0] * scale)
        image_width = int(image.shape[1] * scale)

        image = cv.resize(image, dsize=(image_width, image_height), interpolation=cv.INTER_AREA)

        image_gray = cv.cvtColor(image, cv.COLOR_BGR2GRAY)
        ret, image_mask = cv.threshold(image_gray, 1, 255, type=cv.THRESH_BINARY)

        image_loc = frame[x_pos:image_height+x_pos, y_pos:image_width+x_pos]

        image_loc[np.where(image_mask)] = 0

        image_loc += image

        return frame

    def AddSpeedometer(self, frame, x_pos, y_pos, rpm, scale=1):
        speed = rpm * 50.2655 * 60 / 63360 # rpm to mph

        normalized_value = ((speed - 0) / (100 - 0)) * (360 - 180) + 180 # for speedometer testing
        cv.ellipse(frame, (x_pos,y_pos), (100,100), 0, 180, normalized_value, (0,183,255), 10)
        cv.putText(frame, str(speed), (x_pos - 25, y_pos-25), cv.FONT_HERSHEY_DUPLEX, scale, (0,183,233), 2)

        return frame

    def AddThrottleBrakes(self, frame, x_pos, y_pos, throttle_percent, brake_percent):
        throttle_norm = throttle_percent / 100 * (y_pos - (y_pos - 100))
        brake_norm = brake_percent / 100 * (y_pos - (y_pos - 100))
        cv.rectangle(frame, (x_pos,y_pos), (x_pos+20,y_pos-int(throttle_norm)), (0,255,0), -1)
        cv.rectangle(frame, (x_pos+30, y_pos), (x_pos+50,y_pos-int(brake_norm)), (0,0,255), -1)

        return frame

    def CameraMain(self, record_stream, data_queue):
        # camera stream
        stream_gui = Stream("gui", quality=self.STREAM_QUALITY, fps=self.STREAM_FPS)  # size = (1920, 1080) is optional
        stream_no_gui = Stream("no_gui", quality=self.STREAM_QUALITY, fps=self.STREAM_FPS)
        # self.server = MjpegServer("10.3.141.1", 8080)
        server = MjpegServer(self.STREAM_IP, self.STREAM_PORT)
        server.add_stream(stream_gui)
        server.add_stream(stream_no_gui)
        server.start()

        # camera capture
        capture = cv.VideoCapture(0, cv.CAP_DSHOW)
        # self.capture = cv.VideoCapture(0, cv.CAP_V4L2) # Solution: https://stackoverflow.com/questions/77190490/usb-camera-doesnt-work-with-opencv-and-raspberry-pi
        capture.set(cv.CAP_PROP_FRAME_WIDTH, 640)
        capture.set(cv.CAP_PROP_FRAME_HEIGHT, 480)
        capture.set(cv.CAP_PROP_FPS, self.STREAM_FPS)
        stream_process = None  # separate stream process

        # stream recording
        fourcc = cv.VideoWriter.fourcc('m', 'p', '4', 'v')
        out = None
        is_recording = False

        def StartRecording():
            nonlocal out
            nonlocal is_recording
            if not is_recording:
                recordings_dir = Path("stream_recordings")
                recordings_dir.mkdir(parents=True, exist_ok=True)   # creates it (and parents) only if missing

                out = cv.VideoWriter(
                    str(recordings_dir / f"{self.CurrentDateTime()}_WebCam_Video.mp4"),
                    fourcc,
                    20,
                    (int(capture.get(3)), int(capture.get(4)))
                )
                is_recording = True

        def EndRecording():
            nonlocal is_recording
            if is_recording:
                is_recording = False
                out.release()

        def EndStream():
            server.stop()
            capture.release()
            if is_recording:
                out.release()

            if __name__ == "__main__":
                cv.destroyAllWindows()

        vehicle_speed = 0
        throttle_percent = 0
        brake_percent = 0
        while True:
            is_true, frame = capture.read()

            frame = cv.flip(frame, 0)

            stream_no_gui.set_frame(frame)

            gui_frame = np.copy(frame)
            gui_frame_x_dim = gui_frame.shape[0]
            gui_frame_y_dim = gui_frame.shape[1]

            # data updates
            if not data_queue.empty():
                can_data = data_queue.get()
                if can_data.get("Speed") != None:
                    vehicle_speed = can_data["Speed"]
                if can_data.get("Throttle1_Level") != None:
                    throttle_percent = can_data["Throttle1_Level"]
                if can_data.get("Brake_Level") != None:
                    brake_percent = can_data["Brake_Level"]

            # logo
            gui_frame = self.AddImage(gui_frame, "static/FRUCDHeader.png", 0, 0, 0.3)

            # gauges
            gui_frame = self.AddSpeedometer(gui_frame, int(gui_frame_y_dim/2), gui_frame_x_dim-10, vehicle_speed, 1)
            gui_frame = self.AddThrottleBrakes(gui_frame, 580, 470, throttle_percent, brake_percent)

            # vehicle status
            # cv.putText(gui_frame, "Vehicle State: Precharge", (225,20), cv.FONT_HERSHEY_DUPLEX, 0.5, (255,255,255), 1)
            # cv.putText(gui_frame, "BMS Status: Normal/No Error", (225,40), cv.FONT_HERSHEY_DUPLEX, 0.5, (255,255,255), 1)

            #indicators
            #gui_frame = AddImage(gui_frame, "traction_control.png", 500, 500, 0.2)
            #gui_frame = AddImage(gui_frame, "speed_limiter.png", 100, 100, 0.2) # not working, use alpha channel


            #cv.putText(gui_frame, str(time.time()), (255, 255), cv.FONT_HERSHEY_TRIPLEX, 1.0, (0, 0, 0), 2)

            stream_gui.set_frame(gui_frame)

            if record_stream.value:
                StartRecording()
                out.write(gui_frame)

                cv.circle(gui_frame, (620, 20), 10, (0, 0, 255), -1)
            else:
                EndRecording()

            if __name__ == "__main__":
                # cv.imshow('Camera', frame)
                if cv.waitKey(20) == ord(' '):
                    break

def main():
    test = CameraFeed()

if __name__ == "__main__":
    main()
