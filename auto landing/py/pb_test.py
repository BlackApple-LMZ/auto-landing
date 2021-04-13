# -*-coding: utf-8 -*-

import argparse
import os

import tensorflow as tf
from tensorflow.python.framework import graph_util
import reader

def getinout(input_checkpoint):
    saver = tf.train.import_meta_graph(input_checkpoint + '.meta', clear_devices=True)
    graph = tf.get_default_graph()
    input_graph_def = graph.as_graph_def()
    with tf.Session() as sess:
        file=open('./nodes.txt','a+')
    
        for n in tf.get_default_graph().as_graph_def().node:
            file.write(n.name + '\n')
        
        file.close()
        
def freeze_graph_test():
    dataset = reader.Reader(FLAGS.data_dir, FLAGS)
    pb_path = './tools/pbmodel.pb'
    with tf.Graph().as_default():
        output_graph_def = tf.GraphDef()
        with open(pb_path, "rb") as f:
            output_graph_def.ParseFromString(f.read())
            tf.import_graph_def(output_graph_def, name="")
        with tf.Session() as sess:
            sess.run(tf.global_variables_initializer())
 
            # 定义输入的张量名称,对应网络结构的输入张量
            input_image_tensor = sess.graph.get_tensor_by_name("Placeholder:0")
            input_keep_prob_tensor = sess.graph.get_tensor_by_name("Placeholder_2:0")
 
            # 定义输出的张量名称
            output_tensor_name = sess.graph.get_tensor_by_name("output/add:0")
 
            batch_size = 10
            batch_num = 50
            for i in range(batch_num):
                batch_x, batch_y = dataset.test.next_batch(batch_size, shuffle=False)
                res = sess.run(output_tensor_name, feed_dict={input_image_tensor: batch_x,
                                                        input_keep_prob_tensor:1.0})
                print(i, res)
            
 
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--data_dir',
        type=str,
        default=os.path.join('.', 'heading dataset'),
        help='Directory of data')
    parser.add_argument(
        '--model_dir',
        type=str,
        default=os.path.join('.', 'saved_model'),
        help='Directory of saved model')

    FLAGS, unparsed = parser.parse_known_args()
    freeze_graph_test()
    #getinout('./saved_model/saved_model_1209/model.ckpt')
