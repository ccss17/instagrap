import socket

def get_size_indicator(data):
    size = str(len(data))
    if len(size) > 4: return -1
    while 1:
        if len(size) == 4:
            return size
        size = '0'+size

def send_data_chunk(soc, data):
    soc.send(get_size_indicator(data).encode())
    soc.send(data.encode())

def get_result(soc):
    result = soc.recv(1024).decode() 
    if result == '0':
        print("PROGRAM EXIT NORMALLY")
        data = soc.recv(1024).decode() 
        print("receive output : ", data)
    elif result == '1':
        print("BUILD FAILED")
    elif result == '2':
        print("RUNTIME ERROR")
    elif result == '3':
        print("TIMEOUT ERROR")

def test():
    host = '54.180.132.66'
    port = 8000
    mySocket = socket.socket()
    mySocket.connect((host,port))

    c_src = r'''
#include <stdio.h>

void main(){
puts("this is C source code");
//sleep(1000);
int * p = 0;
*p = 10;
//main();
//printf("%d\n", 11 / 0);
}'''
    test_case = r'''
10 20 30
'''
    send_data_chunk(mySocket, c_src)
    send_data_chunk(mySocket, test_case)

    get_result(mySocket)

    mySocket.close()

if __name__ == '__main__':
    test()
