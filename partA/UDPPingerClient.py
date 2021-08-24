#!/usr/bin/env python

import time
import socket

_host = "127.0.0.1"
_port = 12000

min_ping = 1000000
max_ping = 0
pingCounter = 0
ping_received = 0
pingAverage = 0
packets_lost = 0

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(1)

def print_Summary():

    print('*** %s UDP ping statistics ***' % (_host))
    print('%d packets transmitted, %d received, %0.0f%% packet loss' % (pingCounter, ping_received, packets_lost))
    print('RTT min ' + str(min_ping) + ' RTT max ' + str(max_ping) + ' RTT avg ' + str(pingAverage / pingCounter))
    sock.close()

for i in range(10):
    try:
        start = time.time()
        message = 'Ping#' + str(i+1) + " " + time.ctime(start)
        sock.sendto(message.encode(), (_host, _port))
        pingCounter += 1
        print("Sent " + message)
        data, server = sock.recvfrom(1024)
        ping_received += 1
        print("Received " + str(data))
        end = time.time()
        total_time = (end - start)
        print("RTT: " + str(total_time) + " seconds\n")
        if total_time > max_ping:
            max_ping = total_time
        if total_time < min_ping:
            min_ping = total_time
        pingAverage += total_time
    except socket.timeout as e:
        print('udp_seq=%d Request timed out\n' % (i))
        packets_lost += 10
    except KeyboardInterrupt:
        print_Summary()

print_Summary()


