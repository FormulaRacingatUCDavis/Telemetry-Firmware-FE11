import cv2 as cv
import time
import numpy as np
from mjpeg_streamer import MjpegServer, Stream

stream = Stream("camera", quality = 50, fps = 30) # size = (1920, 1080) is optional

server = MjpegServer("10.3.141.1", 8080)
server.add_stream(stream)
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
    # capture = cv.VideoCapture('http://192.168.137.216:8081')
    capture = cv.VideoCapture(0, cv.CAP_V4L2) # Solution: https://stackoverflow.com/questions/77190490/usb-camera-doesnt-work-with-opencv-and-raspberry-pi

    while True:
        isTrue, frame = capture.read()

        #frame = AddImage(frame, "FRUCDHeader.png", 0, 0, 0.3)

        cv.putText(frame, str(time.time()), (255, 255), cv.FONT_HERSHEY_TRIPLEX, 1.0, (0, 0, 0), 2)

        if __name__ == "__main__":
            #cv.imshow('Camera', frame)

            if cv.waitKey(20) == ord(' '):
                break

        stream.set_frame(frame)

    server.stop()
    capture.release()

    if __name__ == "__main__":
        cv.destroyAllWindows()

def main():
   GetCameraFrame()

if __name__ == "__main__":
    main()