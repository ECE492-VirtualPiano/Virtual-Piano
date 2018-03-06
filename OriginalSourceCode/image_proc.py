"""
*********************************************************************************************************
*
*                                          IMAGE PROCESSING CODE
*
*                                            CYCLONE V SOC
*
* Filename      : image_proc.py
* Version       : V1.00
* References    : 
* Creation: February 17, 2018
* Created by: Mingjun Zhao (zhao2@ualberta.ca)
*********************************************************************************************************
* Note(s)       : This code is python throw away code that will be ported into ucos-ii in the future. 
*				  The functionality of the code provides the ability to convert a jpg image into hsv, hsl, 
*				  or grayscale for easier processing of light levels in the image. The program colors 
*				  pixels in the image that pass a threshold value. The pixels in the image that are 
*				  compared to the threshold value can be set.
* 
*********************************************************************************************************
"""
from PIL import Image
import sys


def rgb2hsv(pixel):
	"""
	Convert RGB format pixel to HSV format
	"""
	r = pixel[0]/255.0
	g = pixel[1]/255.0
	b = pixel[2]/255.0

	cmax = max(r, g, b)
	cmin = min(r, g, b)
	delta = cmax - cmin

	if delta == 0:
		h = 0
	elif cmax == r:
		h = 60 * ((g-b) / delta % 6)
	elif cmax == g:
		h = 60 * ((b-r) / delta + 2)
	elif cmax == b:
		h = 60 * ((r-g) / delta + 4)

	if cmax == 0:
		s = 0
	else:
		s = delta / cmax

	v = cmax

	return (h, s, v)

def rgb2hsl(pixel):
	"""
	Convert RGB format pixel to HSL format
	"""
	r = pixel[0]/255.0
	g = pixel[1]/255.0
	b = pixel[2]/255.0

	cmax = max(r, g, b)
	cmin = min(r, g, b)
	delta = cmax - cmin

	if delta == 0:
		h = 0
	elif cmax == r:
		h = 60 * ((g-b) / delta % 6)
	elif cmax == g:
		h = 60 * ((b-r) / delta + 2)
	elif cmax == b:
		h = 60 * ((r-g) / delta + 4)

	if cmax == 0:
		s = 0
	else:
		s = delta / cmax

	l = (cmax + cmin) /2

	return (h, s, l)


def rgb2gs(pixel):
	"""
	Convert RGB format pixel to Grayscale format
	"""
	r = pixel[0]
	g = pixel[1]
	b = pixel[2]

	return int(0.2126 * r + 0.7152 * g + 0.0722 * b)

def hsl2rgb(pixel):
	"""
	Convert HSL format pixel to RGB format
	"""
	h = pixel[0]
	s = pixel[1]
	l = pixel[2]

	c = (1-abs(2*l-1))*s
	x = c*(1-abs((h/60)%2-1))
	m = l-c/2

	if h>=0 and h<60:
		r,g,b = c,x,0
	elif h>=60 and h<120:
		r,g,b = x,c,0
	elif h>=120 and h<180:
		r,g,b = 0,c,x
	elif h>=180 and h<240:
		r,g,b = 0,x,c
	elif h>=240 and h<300:
		r,g,b = x,0,c
	elif h>=300 and h<360:
		r,g,b = c,0,x

	r,g,b = (r+m)*255, (g+m)*255, (b+m)*255

	return (int(r),int(g),int(b))
	
def highlight_bright_points(img):
	"""
	Highlight the bright points in a given image with
	red and return the number of bright points.
	"""
	threshold = 0.95
	count = 0

	pix = img.load()

	# loop through every points of the image
	for x in range(img.size[0]):
		for y in range(img.size[1]):
			point = rgb2hsl(pix[x, y])

			# the point is bright if its luminous value
			# is greater than threshold
			if point[2] > threshold:
				pix[x,y] = (255,0,0)
				count += 1

	img.show()

	return count

def led_on_off_test():
	"""
	Test on led on/off picture pairs viewing from different
	angles. The number of bright points in 'led on' a picture 
	should be way bigger than it in a 'led off' picture
	"""
	img_1_on = Image.open('img_1_on.jpg')
	img_2_on = Image.open('img_2_on.jpg')
	img_3_on = Image.open('img_3_on.jpg')
	img_1_off = Image.open('img_1_off.jpg')
	img_2_off = Image.open('img_2_off.jpg')
	img_3_off = Image.open('img_3_off.jpg')

	assert highlight_bright_points(img_1_on) > 2*highlight_bright_points(img_1_off)
	assert highlight_bright_points(img_2_on) > 2*highlight_bright_points(img_2_off)
	assert highlight_bright_points(img_3_on) > 2*highlight_bright_points(img_3_off)

if __name__ == "__main__":
	led_on_off_test()

