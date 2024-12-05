import cv2 as cv
import time
import numpy as np
from mjpeg_streamer import MjpegServer, Stream
import math

STREAM_QUALITY = 50
STREAM_FPS = 60
stream_gui = Stream("gui", quality = STREAM_QUALITY, fps = STREAM_FPS) # size = (1920, 1080) is optional
stream_no_gui = Stream("no_gui", quality = STREAM_QUALITY, fps = STREAM_FPS)

# server = MjpegServer("10.3.141.1", 8080)
server = MjpegServer("localhost", 8080)
server.add_stream(stream_gui)
server.add_stream(stream_no_gui)
server.start()

def AddImage(frame, image_src, x_pos, y_pos, scale):
    logo = cv.imread(image_src)

    logo_height = int(logo.shape[0] * scale)
    logo_width = int(logo.shape[1] * scale)

    logo = cv.resize(logo, dsize=(logo_width, logo_height), interpolation=cv.INTER_AREA)

    logo_gray = cv.cvtColor(logo, cv.COLOR_BGR2GRAY)
    ret, logo_mask = cv.threshold(logo_gray, 1, 255, type=cv.THRESH_BINARY)

    logo_loc = frame[x_pos:logo_height, y_pos:logo_width]

    logo_loc[np.where(logo_mask)] = 0

    logo_loc += logo

    return frame

def GetCameraFrame():
    capture = cv.VideoCapture(2, cv.CAP_ANY)
    # capture = cv.VideoCapture(0, cv.CAP_V4L2) # Solution: https://stackoverflow.com/questions/77190490/usb-camera-doesnt-work-with-opencv-and-raspberry-pi
    capture.set(cv.CAP_PROP_FRAME_WIDTH, 1920)
    capture.set(cv.CAP_PROP_FRAME_HEIGHT, 1080)

    while True:
        isTrue, frame = capture.read()

        stream_no_gui.set_frame(frame)

        gui_frame = np.copy(frame)

        # logo
        gui_frame = AddImage(gui_frame, "FRUCDHeader.png", 0, 0, 0.3)

        # gauges


        cv.putText(gui_frame, str(time.time()), (255, 255), cv.FONT_HERSHEY_TRIPLEX, 1.0, (0, 0, 0), 2)

        if __name__ == "__main__":
            #cv.imshow('Camera', gui_frame)

            if cv.waitKey(20) == ord(' '):
                break

        stream_gui.set_frame(gui_frame)

    server.stop()
    capture.release()

    if __name__ == "__main__":
        cv.destroyAllWindows()

def main():
   GetCameraFrame()

if __name__ == "__main__":
    main()