import cv2
import ast
import base64
import imutils
import requests as req

end_point = "http://192.168.15.23:88/predict"
url_img = "imgs/DJI_0066_1218.jpg"

frame = cv2.imread(url_img)
    

def encod_img(frame):
    _, frame = cv2.imencode('.jpg', frame)
    frame = frame.tostring()
    frame = base64.b64encode(frame)
    return frame

def predict():
    #frame = imutils.resize(frame, height=647)
    img_send = encod_img(frame)
    data_send = dict(image=img_send)
    result=req.post(end_point, data=data_send)
    print(result)
    return result.json()

def draw_img(result, frame):    
    color=(255, 0, 255)                                    
    for item in result:
        
        label=item['label']
        score=item['score']
        box=item['bb_o']
        x,y,w,h=box
        
        display_txt = '{} {:0.2f}'.format(label, score)
        
        cv2.rectangle(frame, (x, y - 20), (x + 50 + len(label) * 6, y), color, -1)
        cv2.putText(frame, display_txt, (x, y - 5), cv2.FONT_HERSHEY_PLAIN, 0.85, (0, 0, 0), 1)                    
        cv2.rectangle(frame, (x, y), (x + w, y + h), color, 2)
    
    
    cv2.putText(frame, "Total: {}".format(len(result)), (20, 45), cv2.FONT_HERSHEY_DUPLEX, 1.5, (255, 255, 255), 1)
    cv2.putText(frame, "Total: {}".format(len(result)), (21, 46), cv2.FONT_HERSHEY_DUPLEX, 1.5, (0, 0, 0), 1)
    
    return frame

#-----------------------------------------

result=predict()
frame=draw_img(result, frame)

cv2.imshow("frame", frame)
cv2.waitKey(0)

