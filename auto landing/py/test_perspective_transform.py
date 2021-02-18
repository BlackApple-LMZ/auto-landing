import numpy as np
import cv2
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import pickle


def perspective_transform(img):
	"""
	Execute perspective transform
	"""
	img_size = (img.shape[1], img.shape[0])

	src = np.float32(
		[[0, 572],
		[1280, 557],
		[572, 465],
		[748, 465]])
	dst = np.float32(
		[[300, 600],
		[980, 600],
		[300, 0],
		[980, 0]])

	m = cv2.getPerspectiveTransform(src, dst)
	m_inv = cv2.getPerspectiveTransform(dst, src)

	warped = cv2.warpPerspective(img, m, img_size, flags=cv2.INTER_LINEAR)
	unwarped = cv2.warpPerspective(warped, m_inv, (warped.shape[1], warped.shape[0]), flags=cv2.INTER_LINEAR)  # DEBUG

	return warped, unwarped, m, m_inv


if __name__ == '__main__':
	img_file = 'test/204.png'

	img = mpimg.imread(img_file)

	warped, unwarped, m, m_inv = perspective_transform(img)

	plt.imshow(warped, cmap='gray', vmin=0, vmax=1)
	plt.show()

	plt.imshow(unwarped, cmap='gray', vmin=0, vmax=1)
	plt.show()
