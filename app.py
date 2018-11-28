import os
from flask import Flask, render_template, request, redirect

app = Flask(__name__) #instance of the Flask class
mode = 'default'
red_1 = '0'
green_1 = '0'
blue_1 = '200'
red_2 = '255'
green_2 = '200'
blue_2 = '0'

@app.route('/')
def index():
    return render_template('index.html', mode = mode, red_1 = red_1, green_1 = green_1, blue_1 = blue_1, red_2 = red_2, green_2 = green_2, blue_2 = blue_2 )


@app.route('/data')
def data_endpoint():
    return '$'+mode+'@'+red_1+','+green_1+','+blue_1+':'+red_2+','+green_2+','+blue_2+':'


@app.route('/form', methods=['POST']) #allow POST requests
def result():
    global mode
    mode = request.form['mode']
    global red_1
    red_1 = request.form['red1']
    global green_1
    green_1 = request.form['green1']
    global blue_1
    blue_1 = request.form['blue1']
    global red_2
    red_2 = request.form['red2']
    global green_2
    green_2 = request.form['green2']
    global blue_2
    blue_2 = request.form['blue2']
    return redirect("/")


if __name__ == '__main__':
    # app.run(debug=True, host='0.0.0.0')
    app.run(host='0.0.0.0', port=os.getenv('PORT', 5000))

