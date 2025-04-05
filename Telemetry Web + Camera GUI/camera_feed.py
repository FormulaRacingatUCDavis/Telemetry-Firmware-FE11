import cv2 as cv
import time
import multiprocessing as mp
import numpy as np
from mjpeg_streamer import MjpegServer, Stream

class CameraFeed:
    def __init__(self, stream_ip="localhost", stream_port=8080, stream_quality=80, stream_fps=30):
        # camera stream
        self.STREAM_QUALITY = stream_quality
        self.STREAM_FPS = stream_fps
        self.STREAM_IP = stream_ip
        self.STREAM_PORT = stream_port
        self.is_recording = False

        STREAM_PROCESS = mp.Process(target=self.CameraMain)
        STREAM_PROCESS.start()

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

    def AddSpeedometer(self, frame, x_pos, y_pos, scale=1):
        normalized_value = ((frame[0,0,0] - 0) / (255 - 0)) * (360 - 180) + 180 # for speedometer testing
        cv.ellipse(frame, (x_pos,y_pos), (100,100), 0, 180, normalized_value, (0,183,255), 10)
        cv.putText(frame, str(frame[0,0,0]), (x_pos - 25, y_pos-25), cv.FONT_HERSHEY_DUPLEX, scale, (0,183,233), 2)

        return frame

    def CameraMain(self):
        # camera stream
        stream_gui = Stream("gui", quality=self.STREAM_QUALITY, fps=self.STREAM_FPS)  # size = (1920, 1080) is optional
        stream_no_gui = Stream("no_gui", quality=self.STREAM_QUALITY, fps=self.STREAM_FPS)
        # self.server = MjpegServer("10.3.141.1", 8080)
        server = MjpegServer(self.STREAM_IP, self.STREAM_PORT)
        server.add_stream(stream_gui)
        server.add_stream(stream_no_gui)
        server.start()

        # camera capture
        capture = cv.VideoCapture(0, cv.CAP_ANY)
        # self.capture = cv.VideoCapture(0, cv.CAP_V4L2) # Solution: https://stackoverflow.com/questions/77190490/usb-camera-doesnt-work-with-opencv-and-raspberry-pi
        capture.set(cv.CAP_PROP_FRAME_WIDTH, 640)
        capture.set(cv.CAP_PROP_FRAME_HEIGHT, 480)
        capture.set(cv.CAP_PROP_FPS, self.STREAM_FPS)
        stream_process = None  # separate stream process

        # stream recording
        fourcc = cv.VideoWriter.fourcc('m', 'p', '4', 'v')
        out = None

        def StartRecording():
            nonlocal out
            if not self.is_recording:
                out = cv.VideoWriter(f"{self.CurrentDateTime()}_WebCam_Video.mp4", fourcc, 20, (int(capture.get(3)), int(capture.get(4))))
                self.is_recording = True

        def EndRecording():
            if self.is_recording:
                self.is_recording = False
                out.release()

        def EndStream():
            server.stop()
            capture.release()
            if self.is_recording:
                out.release()

            if __name__ == "__main__":
                cv.destroyAllWindows()

        while True:
            is_true, frame = capture.read()

            stream_no_gui.set_frame(frame)

            gui_frame = np.copy(frame)
            gui_frame_x_dim = gui_frame.shape[0]
            gui_frame_y_dim = gui_frame.shape[1]

            # logo
            gui_frame = self.AddImage(gui_frame, "static/FRUCDHeader.png", 0, 0, 0.3)

            # gauges
            gui_frame = self.AddSpeedometer(gui_frame, int(gui_frame_y_dim/2), gui_frame_x_dim-10, 1)

            #indicators
            #gui_frame = AddImage(gui_frame, "traction_control.png", 500, 500, 0.2)
            #gui_frame = AddImage(gui_frame, "speed_limiter.png", 100, 100, 0.2) # not working, use alpha channel


            #cv.putText(gui_frame, str(time.time()), (255, 255), cv.FONT_HERSHEY_TRIPLEX, 1.0, (0, 0, 0), 2)

            stream_gui.set_frame(gui_frame)

            if self.is_recording:
                out.write(gui_frame)

            if __name__ == "__main__":
                # cv.imshow('Camera', frame)
                if cv.waitKey(20) == ord(' '):
                    break

def main():
    test = CameraFeed()

if __name__ == "__main__":
    main()
