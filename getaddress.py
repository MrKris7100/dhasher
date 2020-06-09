import socket, hashlib, os, urllib.request, time # Only python3 included libraries


soc = socket.socket()

# This sections grabs pool adress and port from Duino-Coin GitHub file
while True: # Grab data grom GitHub section
    serverip = "https://raw.githubusercontent.com/revoxhere/duino-coin/gh-pages/serverip.txt" # Serverip file
    with urllib.request.urlopen(serverip) as content:
        content = content.read().decode().splitlines() #Read content and split into lines
    pool_address = content[0] #Line 1 = pool address
    pool_port = content[1] #Line 2 = pool port
    break # Continue

outp = open("outadd.txt","w")
outp.write(content[0])
outp.write('\n')
outp.write(content[1])
