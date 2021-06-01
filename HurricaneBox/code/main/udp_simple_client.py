import socket as s
a=s.socket(s.AF_INET, s.SOCK_DGRAM)
a.bind( ("10.0.0.30",1131))
a.sendto(b"0", ("10.0.0.145",1130))