from flask import Flask,render_template,url_for,request,redirect, make_response
import random
import json
from time import time
from random import random
from flask import Flask, render_template, make_response, url_for
from collections import defaultdict
from metric import Metric

app = Flask(__name__)

# motion and sound: upload rate = 20 sec
# temp and humidity: upload rate = 30 minutes

temperature = Metric("float")
humidity = Metric("float")
light = Metric("float")
motion_freq = Metric("int")
noise_freq = Metric("int")

environ_metrics = [temperature, humidity, light]
human_metrics = [noise_freq, motion_freq ] # order matters


@app.route('/', methods=["GET", "POST"])
def main():
    global temperature, humidity, light, sound_freq, motion_freq
    result1 = request.args.get("var1")
    result2 = request.args.get("var2")

    if result1 is not None:
        print("result1: ", result1.split(','))
        result1 = result1.split(',')
        assert len(result1) == 3, f"result1: {result1} is not matching all environMetrics"
        for i, val in enumerate(result1):
            environ_metrics[i].update_value(val)


    if result2 is not None:
        print("result2: ", result2.split(','))
        result2 = result2.split(',')
        assert len(result2) == 2, f"result2: {result2} is not matching all humanMetrics"
        for i, val in enumerate(result2):
            human_metrics[i].update_value(val)
            
    return render_template('index.html')


@app.route('/data', methods=["GET", "POST"])
def data():
    temp = temperature.current_value
    hum = humidity.current_value
    lightness = light.current_value
    noiseFre = noise_freq.current_value
    motionFre = motion_freq.current_value
    # print(noiseFre, noise_freq.avg_value, noise_freq.count)
    
    data = [time() * 1000, temp, hum, lightness, noiseFre, motionFre]
    data += [metric.avg_value for metric in environ_metrics]
    data += [metric.avg_value for metric in human_metrics]
    print(data)

    response = make_response(json.dumps(data))

    response.content_type = 'application/json'

    return response


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=True)
