# Authors: Hayato Nakamura
#             Yang Hang Liu
#             Arnaud Harmange 
import json
import numpy as np

with open('datafile.json', 'r') as f:
  data = f.read()
obj = json.loads(data)

ir = []
ultrasonic = []

for i in obj:
    if i['type'] == 'IR':
        ir.append(float(i['value']))
    elif i['type'] == 'UL':
        ultrasonic.append(float(i['value']))
ir = np.asarray(ir)
ultrasonic = np.asarray(ultrasonic)
print("Ultrasonic Sensor : mean=" + str(np.round(np.mean(ultrasonic),5)) + ", std=" + str(np.round(np.std(ultrasonic), 5)))
print("IR Rangefinder    : mean=" + str(np.round(np.mean(ir), 5)) + ", std=" + str(np.round(np.std(ir), 5)))