#!/usr/bin/python

# Let's import the libraries we're going to use
from flask import Flask, request
import json
from numpy import *
from scipy import *


app = Flask("My Great App")

@app.route("/")
def index():
	return "Hello there!"

@app.route("/blah", methods=['POST'])
def blah():
	return "You asked for blah, and you sent in: " + request.data

@app.route("/fft", methods=['POST'])
def calc_fft():
	# Read in data, convert it to an array from json
	data = array(json.loads(request.data))

	# Convert that array to the magnitude-fourier domain
	DATA = abs(fft(data));

	# Return a json representation of that
	return json.dumps(list(DATA));


app.run(host="0.0.0.0")