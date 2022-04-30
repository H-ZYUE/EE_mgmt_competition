# -*-coding:UTF-8-*-
# 此代码只在树莓派上运行
import serial  # 导入serial库
import datetime  # 获取当前时间
import os
import time
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)  # 打开端口，每一秒返回一个消息
# try模块用来结束循环（靠抛出异常）
try:
    while 1:
        ser.write('s\n'.encode())  # 写s字符
        time.sleep(2)
        os.makedirs('安防情况', exist_ok=True)
        now = datetime.datetime.now()
        print(now)
        response = ser.readall()  # 用response读取端口的返回值
        response = str(response, 'utf-8')
        savedate = datetime.date.today()
        filename = '安防情况/' + str(savedate) + '.txt'
        f = open(filename, "w")
        f.write('***********************安防状况*************************' + "\n")
        f.write(str(now) + '\n')
        f.write(response)
        f.write('***********************安防状况*************************' + "\n")
        f.close()
        print('successful')
except:
    #ser.close()  # 抛出异常后关闭端口
    print("error!!!")