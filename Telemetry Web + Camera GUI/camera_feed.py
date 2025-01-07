import cv2 as cv
import time
import numpy as np
from mjpeg_streamer import MjpegServer, Stream

STREAM_QUALITY = 50
STREAM_FPS = 60
stream_gui = Stream("gui", quality = STREAM_QUALITY, fps = STREAM_FPS) # size = (1920, 1080) is optional
stream_no_gui = Stream("no_gui", quality = STREAM_QUALITY, fps = STREAM_FPS)

# server = MjpegServer("10.3.141.1", 8080)
server = MjpegServer("localhost", 8080)
server.add_stream(stream_gui)
server.add_stream(stream_no_gui)
server.start()

def AddImage(frame, image_src, x_pos, y_pos, scale = 1):
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

def AddSpeedometer(frame, x_pos, y_pos, scale = 1):
    normalized_value = ((frame[0,0,0] - 0) / (255 - 0)) * (360 - 180) + 180 # for speedometer testing
    cv.ellipse(frame, (x_pos,y_pos), (100,100), 0, 180, normalized_value, (0,183,255), 10)
    cv.putText(frame, str(frame[0,0,0]), (x_pos - 25,y_pos-25), cv.FONT_HERSHEY_DUPLEX, scale, (0,183,233), 2)

    return frame

def GetCameraFrame():
    capture = cv.VideoCapture(0, cv.CAP_DSHOW)
    # capture = cv.VideoCapture(0, cv.CAP_V4L2) # Solution: https://stackoverflow.com/questions/77190490/usb-camera-doesnt-work-with-opencv-and-raspberry-pi
    capture.set(cv.CAP_PROP_FRAME_WIDTH, 1920)
    capture.set(cv.CAP_PROP_FRAME_HEIGHT, 1080)
    capture.set(cv.CAP_PROP_FPS, STREAM_FPS)

    while True:
        isTrue, frame = capture.read()

        stream_no_gui.set_frame(frame)

        gui_frame = np.copy(frame)
        gui_frame_x_dim = gui_frame.shape[0]
        gui_frame_y_dim = gui_frame.shape[1]

        # logo
        gui_frame = AddImage(gui_frame, "static/FRUCDHeader.png", 0, 0, 0.3)

        # gauges
        gui_frame = AddSpeedometer(gui_frame, int(gui_frame_y_dim/2), gui_frame_x_dim-10, 1)

        #indicators
        gui_frame = AddImage(gui_frame, "traction_control.png", 500, 500, 0.2)
        gui_frame = AddImage(gui_frame, "speed_limiter.png", 100, 100, 0.2) # not working, use alpha channel


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