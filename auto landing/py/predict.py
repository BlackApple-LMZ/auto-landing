import argparse
import os
import time

import cv2

import tensorflow as tf
import numpy as np

from model import Nivdia_Model

num = 0

def splitFrames(videoFileName):
    cap = cv2.VideoCapture(videoFileName + 'test.avi') # 打开视频文件
    global num
    while True:
        # success 表示是否成功，data是当前帧的图像数据；.read读取一帧图像，移动到下一帧
        success, data = cap.read()
        if not success:
            break
        
        cv2.imwrite(videoFileName + str(num) + ".jpg", data)

        print(num)
        num = num + 1
        
    cap.release()


def read_an_image(filename):
    img = cv2.imread(filename + '.png')

    if FLAGS.model == 'alexnet':
        img = cv2.resize(img[-480:], (227, 227))

    elif FLAGS.model == '':
        image_mean = [175.365, 175.257, 163.012] #850
        #image_mean = [163.481, 163.017, 150.835] #486
        img = img - image_mean
        img = cv2.resize(img[-480:], (200, 66))
        #img = cv2.cvtColor(img, cv2.COLOR_BGR2YUV)
    return img
    
def test():

    y = tf.placeholder(tf.float32, [None, 1])
    keep_prob = tf.placeholder(tf.float32)

    if FLAGS.model == 'alexnet':
        x_image = tf.placeholder(tf.float32, [None, 227, 227, 3])
        train_layers = []
        model = AlexNet(x_image, y, keep_prob, FLAGS, train_layers, is_pretrained=False, is_train=False)

    elif FLAGS.model == '':
        x_image = tf.placeholder(tf.float32, [None, 66, 200, 3])
        model = Nivdia_Model(x_image, y, keep_prob, FLAGS, False)

    # model saver used to resore model from model dir
    saver = tf.train.Saver()

    with tf.Session() as sess:
        path = tf.train.latest_checkpoint(FLAGS.model_dir)
        if not (path is None):
            saver.restore(sess, path)
        else:
            print("There is not saved model in the directory of model.")

        start = time.clock()

        f = open(os.path.join(FLAGS.data_dir, "data.txt"), 'w+')
        batch_num = 14
        for i in range(batch_num):
            batch_x = read_an_image(os.path.join(FLAGS.data_dir, str(i+204)))
            batch_x = np.expand_dims(batch_x, 0)
            batch_x = batch_x/255 
            #print(batch_x.shape)
            #batch_y = 0.0;
            res = sess.run(
                model.prediction, feed_dict={
                    x_image: batch_x,
                    keep_prob: 1.0
                })
                
            f.write(str(i+204) + ' ' + str(float(res) * 180 / np.pi) + '\n')
            print("image: ", i, float(res) * 180 / np.pi)


        end = time.clock()
        print("The function run time is : %.03f seconds" %(end-start)) 

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--data_dir',
        type=str,
        default=os.path.join('.', 'test'),
        help='Directory of data')
    parser.add_argument(
        '--model_dir',
        type=str,
        default=os.path.join('.', 'saved_model'),
        help='Directory of saved model')
    parser.add_argument(
        '--model',
        type=str,
        default='',
        help='switch model')

    FLAGS, unparsed = parser.parse_known_args()
    
    #splitFrames(FLAGS.data_dir)
    
    test()
