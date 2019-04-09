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
 
def test():
    host = '54.180.132.66'
    port = 8000
    mySocket = socket.socket()
    mySocket.connect((host,port))

    c_src = '''\
#include <stdio.h>

void main(){
    puts("this is C source code");
    puts("this is C source code");
    puts("this is C source code");
    puts("this is C source code");
    puts("this is C source code");
    puts("this is C source code");
    puts("this is C source code");
}'''
    test_case = '''\
10 20 30
'''
    send_data_chunk(mySocket, c_src)
    send_data_chunk(mySocket, test_case)

    mySocket.close()
 
if __name__ == '__main__':
    test()