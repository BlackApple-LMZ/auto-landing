import socket
import sys
import time
import traceback

import os
import argparse

import cv2

import numpy as np

import tensorflow as tf
from model import Nivdia_Model


def read_an_image(filename):
    img = cv2.imread(filename)

    if FLAGS.model == 'alexnet':
        img = cv2.resize(img[-480:], (227, 227))

    elif FLAGS.model == '':
        image_mean = [175.365, 175.257, 163.012] #850
        #image_mean = [163.481, 163.017, 150.835] #486
        #img = img - image_mean
        img = cv2.resize(img[-480:], (200, 66))
        
        #img = cv2.cvtColor(img, cv2.COLOR_BGR2YUV)
    return img

    
if __name__=='__main__':

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
    
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    host= socket.gethostname()
    port = 5024

    # prepare model
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

        # connect server
        while True:
            try:
                client.connect((host, port))
                time.sleep(1)
                print('-----------------------------------------')
                print('client start connect to host/port:{}/{}'.format(host, port))
                print('-----------------------------------------')
                break
            except ConnectionRefusedError:
                print('socket server refused or not started, reconnect to server in 3s .... host/port:{}/{}'.format(
                    host, port))
                time.sleep(3)

            except Exception as e:
                traceback.print_exc()
                print('do connect error:{}'.format(str(e)))
                time.sleep(5)

        i = 0
        
        while True:
            
            data = client.recv(1024)
            data = data.decode('utf-8')
            print('Recv:',data)
            if data =='startProcess\n':

                tic = time.time()
                
                f = open("..\image\data.txt", 'w+')
                
                batch_x = read_an_image("..\image\image.png")
                print(batch_x)
                batch_x = np.expand_dims(batch_x, 0)/255
                #print(batch_x.shape)
                #batch_y = 0.0;
                res = sess.run(
                    model.prediction, feed_dict={
                        x_image: batch_x,
                        keep_prob: 1.0
                    })
                        
                f.write(str(float(res) * 180 / np.pi) + '\n')
                print("image: ", i, float(res) * 180 / np.pi)
                i = i + 1
                f.close()

                print ('Predict cost:',time.time() - tic)

                client.send('iFinish\n'.encode('utf-8'))
                print ('finish')
                
            time.sleep(1)

