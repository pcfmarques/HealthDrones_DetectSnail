# coding: utf-8

import sys
import cv2
import json
import base64
import argparse
import tornado.web
import numpy as np
import tornado.ioloop
from YoloV5Model import YoloV5Model


# pip install tornado
# fuser -k -n tcp 8888

#python ServiceDetectionAPI.py --port 88

#start args--------------------------------------------

parser = argparse.ArgumentParser(description='Parameters to start the service.')
parser.add_argument('--port', type=str, default='80', help='Value of the port on which the service to operate!')
parser.add_argument('--model', type=str, default='snail', help='Name of model to upload!')
args = parser.parse_args()

#configs--------------------------------------------

PORT = int(args.port)
CHOOSE_MODEL = str(args.model)

#--------------------------------------------

detector = YoloV5Model(CHOOSE_MODEL)

# globals variables
app = None

class Predict(tornado.web.RequestHandler):
    
    def set_default_headers(self):
        # print("setting headers!!!")
        self.set_header("Access-Control-Allow-Origin", "*")
        self.set_header("Access-Control-Allow-Headers", "x-requested-with")
        self.set_header('Access-Control-Allow-Methods', 'POST, GET, OPTIONS')

    def post(self):
        req = self.request
        protocol = req.protocol
        host = req.host
        method = req.method
        uri = req.uri
        version = req.version
        remote_ip = req.remote_ip
        
        try:
            image = self.get_argument('image')
            img_recv = self.decode_img(image)  
            outputs = detector.predict(img_recv)    
            self.write(json.dumps(outputs))
        except:
            e = sys.exc_info()[0]
            msg = "Status: {}, Error: {}, protocol:{}, host:{}, method:{}, uri:{}, version:{}, remote_ip:{}".\
                format(self.get_status(), e, protocol, host, method, uri, version, remote_ip)
            print(msg)
            
    def decode_img(self, img):
        raw_data = img
        img_recv = base64.b64decode(raw_data)
        img_recv = np.fromstring(img_recv, dtype=np.uint8)
        img_recv = np.asarray(img_recv, dtype=np.uint8)
        img_recv = cv2.imdecode(img_recv, 1)
        
        return img_recv
             
def make_app():
    global app

    hand = []
    # hand.append((r"/\w+", Predict))
    #hand.append((r"/.*", Predict))
    hand.append((r"/predict", Predict))
    
    settings = {
        "cookie_secret": "__TODO:_GENERATE_YOUR_OWN_RANDOM_VALUE_HERE__",
        "login_url": "/login",
        "xsrf_cookies": False,
    }
    
    app = tornado.web.Application(hand, **settings)
    
    return app
    
if __name__ == "__main__":
    try:
        msg = "Service up ..."
        print(msg)
        
        app = make_app()
        app.listen(PORT)
        tornado.ioloop.IOLoop.current().start()
    except:
        e = sys.exc_info()[0]
        msg = "{} - {}".format("Error starting service!", e)
        print(msg)
        
