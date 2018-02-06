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