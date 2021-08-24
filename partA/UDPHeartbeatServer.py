import time
from socket import *
# Server
_host = "127.0.0.1"
_port = 12000
# make a UDP socket
sock = socket(AF_INET, SOCK_DGRAM)
# bind
sock.bind((_host, _port))
countHeartbeat = 1

while 1:
    # get the client packet
    message, addr = sock.recvfrom(1024)

    message = message.upper().decode()
    myMsg = str(message)

    myMsg = myMsg.split(' ')
    received_msg = myMsg[0]
    received_ping_number = myMsg[1]
    total_time = time.time() - float(myMsg[2])

    # when the time more then 10 sec -> the client application is stop
    if total_time > 10:
        msgRecv = "heartbeat number : "+str(countHeartbeat) + " ,Cause : application of the client has stopped"
        print(msgRecv)
        sock.sendto(msgRecv.encode(), addr)

    # when the time more then 4 sec and less then 10 sec -> the client application is missing
    elif total_time > 4:
        msgRecv = "heartbeat number :" + str(countHeartbeat) + " ,Cause : missing"
        print(msgRecv)
        sock.sendto(msgRecv.encode(), addr)

    # when the client application is received
    else:
        msgRecv = "heartbeat number : " + received_ping_number + " ,Cause : received"
        print(msgRecv)
        sock.sendto(msgRecv.encode(), addr)

    countHeartbeat+=1