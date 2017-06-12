import socket  

print '[start]'
msg = raw_input("[Run?]:")

address1 = ('127.0.0.1', 6001)  
s1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
s1.connect(address1)  
s1.send('Cmd:reg\r\nName:test\r\nPort:10086\r\n\r\n')
print 'send success'
s1.close()  

#===========================================

address2 = ('127.0.0.1', 10086)  
s2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s2.bind(address2)  
s2.listen(20)  
print 'server listening.'

while True:  
    ss, addr = s2.accept()  
    print '[Recv]:from ',addr  
    ra = ss.recv(2048)  
    print ra  
    ss.send('this is test server.')    
    ss.close()  

s2.close() 