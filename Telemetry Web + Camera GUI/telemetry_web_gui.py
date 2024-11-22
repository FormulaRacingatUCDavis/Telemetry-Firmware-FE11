import justpy as jp

def ShowStream():
    wp = jp.WebPage()

    # img = jp.Img(src='http://10.3.141.1:8080/camera', a=wp)
    img = jp.Img(src='http://localhost:8080/gui', a=wp, style = "margin:auto; height:85%; width:85%")

    return wp

#jp.justpy(ShowStream, host='10.3.141.1')
jp.justpy(ShowStream)