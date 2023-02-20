
import cv2
from SnailModel import SnailModel


cap = cv2.VideoCapture("/home/avantia/Pictures/ProjetoCara/caramujosFiocruz.mp4")
model=SnailModel()

cap.set(cv2.CAP_PROP_POS_FRAMES, 500)

_, frame = cap.read()
height, width, _ = frame.shape
fourcc = cv2.VideoWriter_fourcc(*'xvid')
out = cv2.VideoWriter("snail_video.avi", fourcc, 14.0, (width, height))


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
    
while True:
    _, frame = cap.read()
    
    result=model.predict(frame)
    frame=draw_img(result, frame)
    
    out.write(frame) 
    cv2.imshow("frame", frame)
    cv2.waitKey(1)  
        
       
    