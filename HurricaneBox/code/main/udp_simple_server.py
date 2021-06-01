import socket as s
b = s.socket(s.AF_INET, s.SOCK_DGRAM)
b.bind( ("10.0.0.30",1130))
while True:
    msg,ca = b.recvfrom(100)
    print(msg)

# import socket

# localIP     = "127.0.0.1"
# localPort   = 20001
# bufferSize  = 1024

# msgFromServer       = "Hello UDP Client"
# bytesToSend         = str.encode(msgFromServer)

# # Create a datagram socket
# UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# # Bind to address and ip
# UDPServerSocket.bind((localIP, localPort))

# print("UDP server up and listening")

# # Listen for incoming datagrams
# while(True):

#     bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)

#     message = bytesAddressPair[0]

#     address = bytesAddressPair[1]

#     clientMsg = "Message from Client:{}".format(message)
#     clientIP  = "Client IP Address:{}".format(address)
    
#     print(clientMsg)
#     print(clientIP)

#     # Sending a reply to client
#     UDPServerSocket.sendto(bytesToSend, address)