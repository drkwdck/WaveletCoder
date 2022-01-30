import numpy as np
import pywt
import cv2

image = cv2.imread('/home/drkwdck/WaveletCoder/images/8-bit-256-x-256-Grayscale-Lena-Image.png', 0)
rows, cols = image.shape

# Convert to float for more resolution for use with pywt
image = np.float32(image)
image /= 255

x = image
shape = x.shape
level = 1
# compute the 2D DWT
c = pywt.wavedec2(x, 'db2', mode='periodization', level=level)
[arr, (a, b, e)] = c

# Номировать не нунжно, это все равно кодируется в рандомный поток байт, а не пикселей
cv2.imwrite("asd.png", (arr*255).round())
cv2.imwrite("a.png", (a*255).round())
cv2.imwrite("b.png", (b*255).round())
cv2.imwrite("e.png", (e*255).round())

orig = pywt.waverec2([arr, (a, b, e)], 'db2')
cv2.imwrite("or.png", orig*255)
