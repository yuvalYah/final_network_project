#!/usr/bin/env python

# from __future__ import print_function
# from socket import *
#
# bind = '' #listen on any
# port = 25050
#
# serverSocket = socket(AF_INET, SOCK_DGRAM)
# serverSocket.bind((bind, port))
#
# while True:
#     message, address = serverSocket.recvfrom(2048)
#     print(".", end='', flush=True)
#     serverSocket.sendto(message, address)


# UDPPingerServer.py
# We will need the following module to generate randomized lost packets
import random
from socket import *
# Create a UDP socket
# Notice the use of SOCK_DGRAM for UDP packets
serverSocket = socket(AF_INET, SOCK_DGRAM)
# Assign IP address and port number to socket
serverSocket.bind(('', 12000))
while True:
    # Generate random number in the range of 0 to 10
    rand = random.randint(0, 10)
    # Receive the client packet along with the address it is coming from
    message, address = serverSocket.recvfrom(1024)
    # Capitalize the message from the clientmessage = message.upper()
    # If rand is less is than 4, we consider the packet lost and do not respond

    if rand < 4:
        continue
    else:
    #     # Otherwise, the server responds
        serverSocket.sendto(message, address)
