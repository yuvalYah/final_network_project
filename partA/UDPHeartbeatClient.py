import time
from socket import *
import random
# Client
_host = "127.0.0.1"
_port = 12000
# make UDP socket
# Notice the use of SOCK_DGRAM for UDP packets
client_socket = socket(AF_INET, SOCK_DGRAM)

counterHeartbeat = 1
while 1:
    randNumber = random.randint(0, 20)
    start_time = time.time()
    HBmsg = 'Heartbeat ' + str(counterHeartbeat) + ' ' + str(start_time)

    # that mean the packet Is missing
    if randNumber in range(0,10):
        time.sleep(4)
        client_socket.sendto(HBmsg.encode(), (_host, _port))

    # that mean the packet is missing piriod of time
    # so we assume that the client application has stopped
    elif randNumber in range(18, 20):
        time.sleep(10)
        client_socket.sendto(HBmsg.encode(), (_host, _port))

    # packet send regular
    else:
        client_socket.sendto(HBmsg.encode(), (_host, _port))

    msg, address = client_socket.recvfrom(_port)
    print("server msg: ", msg.decode())
    counterHeartbeat += 1

client_socket.close()