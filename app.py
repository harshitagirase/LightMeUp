import os
from flask import Flask, render_template

app = Flask(__name__) #instance of the Flask class

@app.route('/')
def index():
    return render_template('index.html')


@app.route('/data')
def data_endpoint():
    return '$fade@80,127,255:255,10,60:'


if __name__ == '__main__':
    # app.run(debug=True, host='0.0.0.0')
    app.run(host='0.0.0.0', port=os.getenv('PORT', 5000))


# get and post route for getting and updating some config on the server that the feather can pull from and update values for the fastled library
# a way to pick RGB colors 
# delay for rotating colors 


