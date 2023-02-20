

import os
import sys
from pathlib import Path
import math
import cv2
import torch
import numpy as np
FILE = Path(__file__).absolute()
sys.path.append(FILE.parents[0].as_posix())  # add yolov5/ to path
from utils.augmentations import letterbox
from models.experimental import attempt_load
from utils.general import check_img_size, non_max_suppression, scale_coords
from utils.torch_utils import select_device, load_classifier


class SnailModel:

    def __init__(self):
        weights = 'weights/snail.pt'
        self.imgsz = [1280]  
        self.conf_thres = 0.3
        self.iou_thres = 0.25
        self.max_det = 1000
        self.device = ''
        self.classes = None
        self.agnostic_nms = False
        self.augment = False
        self.half = False
        
        self.imgsz *= 2 if len(self.imgsz) == 1 else 1  # expand
    
        # Initialize
        self.device = select_device(self.device)
        self.half &= self.device.type != 'cpu'  # half precision only supported on CUDA
    
        # Load model
        classify = False
        pt = True
        self.stride, self.names = 64, [f'class{i}' for i in range(1000)]  # assign defaults
        if pt:
            self.model = attempt_load(weights, map_location=self.device)  # load FP32 model
            self.stride = int(self.model.stride.max())  # model stride
            self.names = self.model.module.names if hasattr(self.model, 'module') else self.model.names  # get class names
            if self.half:
                self.model.half()  # to FP16
            if classify:  # second-stage classifier
                modelc = load_classifier(name='resnet50', n=2)  # initialize
                modelc.load_state_dict(torch.load('resnet50.pt', map_location=self.device)['model']).to(self.device).eval()
       
        self.imgsz = check_img_size(self.imgsz, s=self.stride)  # check image size
    
        # Run inference
        self.model(torch.zeros(1, 3, *self.imgsz).to(self.device).type_as(next(self.model.parameters())))  # run once
        
        #-----------------------------------------
        lab_classes = self.read_class_names(self.names)
        class_colors = {}
        for i in range(0, len(lab_classes)):
            # This can probably be written in a more elegant manner
            hue = math.pow(i, 9) / len(lab_classes)
            col = np.zeros((1, 1, 3)).astype("uint8")
            col[0][0][0] = hue
            col[0][0][1] = 140  # Saturation
            col[0][0][2] = 220  # Value
            cvcol = cv2.cvtColor(col, cv2.COLOR_HSV2BGR)
            col = (int(cvcol[0][0][0]), int(cvcol[0][0][1]), int(cvcol[0][0][2]))
            class_colors.update({str(lab_classes[i]):col}) 
        #-----------------------------------------
    
    def read_class_names(self, data):
        names = {}
        for ID, name in enumerate(data):
            names[ID] = name.strip('\n')
        return names
    
    def predict(self, im0s):
        # Padded resize
        img = letterbox(im0s, self.imgsz, stride=self.stride, auto=True)[0]

        # Convert
        img = img.transpose((2, 0, 1))[::-1]  # HWC to CHW, BGR to RGB
        img = np.ascontiguousarray(img)
        #----
        
        img = torch.from_numpy(img).to(self.device)
        img = img.half() if self.half else img.float()  # uint8 to fp16/32
        img = img / 255.0  # 0 - 255 to 0.0 - 1.0
        if len(img.shape) == 3:
            img = img[None]  # expand for batch dim

        # Inference
        pred = self.model(img, augment=self.augment, visualize=None)[0]
    
        pred = non_max_suppression(pred, self.conf_thres, self.iou_thres, self.classes, self.agnostic_nms, max_det=self.max_det)
        objects = []
        for i, det in enumerate(pred): 
            im0 = im0s.copy()
            if len(det): 
                det[:,:4] = scale_coords(img.shape[2:], det[:,:4], im0.shape).round()
                for * xyxy, conf, cls in reversed(det):
                    c = int(cls)  
                    c1, c2 = (int(xyxy[0]), int(xyxy[1])), (int(xyxy[2]), int(xyxy[3]))

                    x, y = c1
                    w, h = c2
                    w, h = w - x, h - y
                    
                    objects.append({'label': self.names[c], 'score': float(conf), 'bb_o':[x, y, w, h]})
            
        return objects
    