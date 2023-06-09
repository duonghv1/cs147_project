from flask import Flask,render_template,url_for,request,redirect, make_response
import random
import json
from time import time
from random import random
from flask import Flask, render_template, make_response

app = Flask(__name__)

temperature, humidity, lightness, sound, motion, temperature_average, temperature_count= 0, 0, 0, 0, 0, 0, 0


@app.route('/', methods=["GET", "POST"])
def main():
    global temperature, humidity, lightness, sound, temperature_average, temperature_count
    result1 = request.args.get("var1")
    result2 = request.args.get("var2")

    if result1 is not None:
        print("result1: ", result1.split(','))
        temperature, humidity, lightness = result1.split(',')
        if temperature is not None:
            temperature = float(temperature)
        if humidity is not None:
            humidity = float(humidity)
        if lightness is not None:
            lightness = float(lightness)
        temperature_average = ((temperature_average * temperature_count) + temperature) / float(temperature_count + 1)
        temperature_count += 1

    if result2 is not None:
        print("result2: ", result2.split(','))
        sound, motion = result2.split(',')
        if sound is not None:
            sound = int(sound)
        if motion is not None:
            motion = int(motion)
            
    return render_template('index.html')


@app.route('/data', methods=["GET", "POST"])
def data():
    temp = temperature
    hum = humidity
    light = lightness
    noiseFre = sound
    motionFre = motion
    print(temp, type(temp), hum, type(hum), light, type(light), noiseFre, type(noiseFre), motionFre, type(motionFre))
    # # Data Format
    # # [TIME, Temperature, Humidity]

    # Temperature = random() * 100
    # Humidity = random() * 55
    # data = [10, 0, 0]
    
    data = [time() * 1000, temp, hum, light, noiseFre, motionFre]

    response = make_response(json.dumps(data))

    response.content_type = 'application/json'

    return response


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=True)
