import socket
import sys



# 接收小于 1024 字节的数据
msg = s.recv(1024)

print (msg.decode('utf-8'))
s.close()

if __name__=='__main__':

    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    host= socket.gethostname()
    port = 5024
    
    while True:
        try:
            client.connect((host, port))
            print ('Connected',(ip,port))
            break
        except:
            print ('Failed to connect to server')
            print ('Will retry in 3 second')
            time.sleep(3)
    client.send('Connected\n'.encode('utf-8'))
    
    while True:
        data = client.recv(1024)
        data = data.decode('utf-8')
        print('Recv:',data)
        if data =='startProcess':
        
            tic = time.time()
            os.system('cd /home/xcy/bindRobot/py/py && ./1.sh')
            #os.system('./1.sh')
            #img = cv2.imread('/home/xcy/bindRobot/build/image1.png')
            #img = img[240-208:240+208,320-208:320+208]
            #img = np.expand_dims(mg,0)/255.
            #out = net.predict(img)
            #print ('Predict cost:',time.time() - tic,'Img shape:',out.shape)
            print ('Predict cost:',time.time() - tic)


            client.send('sendok\n'.encode('utf-8'))
            print ('sendok')
            #cv2.waitKey()
        time.sleep(1)

