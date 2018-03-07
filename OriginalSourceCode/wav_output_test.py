"""
*********************************************************************************************************
*
*                                  WAVE OUTPUT FILE TESTING PYTHON CODE
*
*                                            CYCLONE V SOC
*
* Filename      : wav_output_test.py
* Version       : V1.00
* Creation 		: February 5, 2018
* Created by 	: Mingjun Zhao (zhao2@ualberta.ca), Daniel Tran (dtran3@ualberta.ca)
*********************************************************************************************************
* Note(s)       : This code is used to test the output wave files produced from the referenced python 
				  code and our implementation of the sample based synthesizer. The two output files are 
				  compared to see if the output data between the two files are similar.
* 
*********************************************************************************************************
"""

from scipy.io import wavfile
import numpy as np

def compareResults():
	# Read in the data for the wav file generated from the c code
	fps_c, sound_c = wavfile.read("test_c.wav")
	# Read in the data for the wav file generated from the python code
	fps_py, sound_py = wavfile.read("test_py.wav")

	# Define the error margins
	length_error_margin = 2
	value_error_margin = 10

	# Compare the length of the sound files and check if they are within the same margin
	assert abs(len(sound_c) - len(sound_py)) < length_error_margin

	# Iterate through each data point in both sound waves and compare the data values to 
	# see if they are within the error margin
	for i in range(min(len(sound_c), len(sound_py))):
		assert abs(sound_c[i] - sound_py[i]) < value_error_margin

# Run the compareResults() function
compareResults()