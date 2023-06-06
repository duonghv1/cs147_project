from flask import Flask,render_template,url_for,request,redirect, make_response
import random
import json
from time import time
from random import random
from flask import Flask, render_template, make_response

app = Flask(__name__)

temperature = 0
humidity = 0
lightness = 0
sound = 0
temperature_average = 0
temperature_count = 0

@app.route('/', methods=["GET", "POST"])
def main():
    global temperature, humidity, lightness, sound, temperature_average, temperature_count
    # global humidity
    result = request.args.get("var")
    if result is not None:
        print("result: ", result.split(','))
        temperature, humidity, lightness, sound = result.split(',')
        if temperature is not None:
            temperature = float(temperature)
        if humidity is not None:
            humidity = float(humidity)
        if lightness is not None:
            lightness = float(lightness)
        if sound is not None:
            sound = float(sound)

        temperature_average = ((temperature_average * temperature_count) + temperature) / float(temperature_count + 1)
        temperature_count += 1
    return render_template('index.html')


@app.route('/data', methods=["GET", "POST"])
def data():
    temp = temperature
    hum = humidity
    light = lightness
    noiseFre = sound;
    print(temp, type(temp), hum, type(hum), light, type(light), noiseFre, type(noiseFre))
    # # Data Format
    # # [TIME, Temperature, Humidity]

    # Temperature = random() * 100
    # Humidity = random() * 55
    # data = [10, 0, 0]
    
    data = [time() * 1000, temp, hum, light, noiseFre]

    response = make_response(json.dumps(data))

    response.content_type = 'application/json'

    return response


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=True)
