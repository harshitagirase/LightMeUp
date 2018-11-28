from flask import Flask, render_template, request

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
    mode = request.form('mode')
    red_1 = request.form('red_1')
    green_1 = request.form('green_1')
    blue_1 = request.form('blue_1')
    red_2 = request.form('red_2')
    green_2 = request.form('green_2')
    blue_2 = request.form('blue_2')
    return str(request.form)


if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')


# get and post route for getting and updating some config on the server that the feather can pull from and update values for the fastled library
# a way to pick RGB colors 
# delay for rotating colors 


