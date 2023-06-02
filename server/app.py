from flask import Flask,render_template,url_for,request,redirect, make_response
import random
import json
from time import time
from random import random
from flask import Flask, render_template, make_response

app = Flask(__name__)

temperature = 0
humidity = 0

@app.route('/', methods=["GET", "POST"])
def main():
    global temperature
    global humidity
    result = request.args.get("var")
    if result is not None:
        print("result: ", result.split(','))
        temperature, humidity = result.split(',')
    return render_template('index.html')


@app.route('/data', methods=["GET", "POST"])
def data():
    temp = 0
    hum = 0
    if temperature is not None:
        temp = float(temperature)
    if humidity is not None:
        hum = float(humidity)
    print(temp, type(temp), hum, type(hum))
    # # Data Format
    # # [TIME, Temperature, Humidity]

    # Temperature = random() * 100
    # Humidity = random() * 55
    # data = [10, 0, 0]
    
    data = [time() * 1000, temp, hum]

    response = make_response(json.dumps(data))

    response.content_type = 'application/json'

    return response


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=True)
