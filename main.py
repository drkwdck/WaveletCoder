import numpy as np
import pywt
import cv2
import sys


output_dir = "wavelet_out/"
mode = sys.argv[1]
file_name = sys.argv[2]
image = cv2.imread(file_name, 0)
rows, cols = image.shape
# Convert to float for more resolution for use with pywt
image = np.float32(image)
image /= 255

x = image
shape = x.shape
level = 2
# compute the 2D DWT
coefs = pywt.wavedec2(x, 'db2', mode='periodization', level=level)

# 1 4 7 7
# 2 3 7 7
# 5 5 6 6
# 5 5 6 6
[first, (second, third, fourth), (fifth, sixth, seventh)] = coefs

# Номировать не нунжно, это все равно кодируется в рандомный поток байт, а не пикселей
# TODO записывать в bin, а не в png
cv2.imwrite("wavelet_out/first.png", first.round())
cv2.imwrite("wavelet_out/second.png", second.round())
cv2.imwrite("wavelet_out/third.png", third.round())
cv2.imwrite("wavelet_out/fourth.png", fourth.round())
cv2.imwrite("wavelet_out/fifth.png", fifth.round())
cv2.imwrite("wavelet_out/sixth.png", sixth.round())
cv2.imwrite("wavelet_out/seventh.png", seventh.round())


