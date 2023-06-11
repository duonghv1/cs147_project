
class Metric:
    def __init__(self, value_type: str):
        self.value_type = value_type
        self.current_value = 0 if value_type == 'int' else 0.0
        self.avg_value = 0.0
        self.count = 0

    
    def update_value(self, new_value):
        if new_value is None:
            return
        if type(new_value) is str:
            new_value = int(new_value) if self.value_type == 'int' else float(new_value)
        self.current_value = new_value
        self.avg_value = ( (self.avg_value * self.count + new_value) / float(self.count + 1) )
        self.count += 1
            
    
    def reset_metric(self):
        if self.value_type == 'int':
            self.current_value = 0
        elif self.value_type == 'float':
            self.current_value = 0.0
        self.avg_value = 0.0
        self.count = 0
        

        
    
