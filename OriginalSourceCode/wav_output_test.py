"""
*********************************************************************************************************
*
*                                  WAVE OUTPUT FILE TESTING PYTHON CODE
*
*                                            CYCLONE V SOC
*
* Filename      : wav_output_test.py
* Version       : V1.00
* References    : 
* Creation: February 5, 2018
* Created by: Mingjun Zhao (zhao2@ualberta.ca), Daniel Tran (dtran3@ualberta.ca)
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
	fps_c, sound_c = wavfile.read("test_c.wav")
	fps_py, sound_py = wavfile.read("test_py.wav")

	length_error_margin = 2
	value_error_margin = 10

	assert abs(len(sound_c) - len(sound_py)) < length_error_margin

	for i in range(min(len(sound_c), len(sound_py))):
		assert abs(sound_c[i] - sound_py[i]) < value_error_margin

compareResults()