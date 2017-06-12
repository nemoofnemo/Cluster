import socket  
  
address = ('127.0.0.1', 6004)  
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  
print "client running" 

while True:  
    msg = raw_input("[Input]:")  
    if not msg:  
        break  
    s.sendto(msg, address)  
    data, addr = s.recvfrom(2048)
    print "[received]:%s" % data , " from", addr 
  
s.close()  