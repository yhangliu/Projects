import socket as s
b = s.socket(s.AF_INET, s.SOCK_DGRAM)
b.bind( ("10.0.0.30",1131))
while True:
    msg,ca = b.recvfrom(100)
    print(msg)