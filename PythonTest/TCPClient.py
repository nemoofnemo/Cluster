import socket  

print '[start]'
while True:
    msg = raw_input("[Run?]:")
    address = ('127.0.0.1', 6001)  
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
    s.connect(address)  
    s.send('Cmd:reg\r\nName:test\r\nPort:10086\r\n\r\n')
    print 'send success'
    s.close()  