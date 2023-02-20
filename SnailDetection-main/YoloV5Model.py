
class YoloV5Model:
    def __init__(self, CHOOSE_MODEL):
        
        if CHOOSE_MODEL == "snail":
            from SnailModel import SnailModel
            self.model = SnailModel()
        
    def predict(self, frame):
        return self.model.predict(frame)
