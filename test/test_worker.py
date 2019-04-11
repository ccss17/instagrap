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
    result = soc.recv(1).decode() 
    print("RESULT:", result)
    if result == '0':
        print("PROGRAM EXIT NORMALLY")
        data = soc.recv(1024).decode() 
        print("OUTPUT:" + data)
    elif result == '1':
        print("BUILD FAILED")
    elif result == '2':
        print("RUNTIME ERROR")
    elif result == '3':
        print("TIMEOUT ERROR")
    else:
        print("UNKNOWN ERROR : ", result)

def test():
    # host = '54.180.132.66'
    host = 'localhost'
    port = 8001
    mySocket = socket.socket()
    mySocket.connect((host,port))

    c_src = r'''
#include <stdio.h>

int main(){
    puts("TEST");
    //int a,b;
    //scanf("%d %d", &a, &b);
    //printf("%d", 1+2);
    return 0;
}
'''
    test_case = r'''1 2'''
    send_data_chunk(mySocket, test_case)
    send_data_chunk(mySocket, c_src)

    get_result(mySocket)

    mySocket.close()

if __name__ == '__main__':
    test()
