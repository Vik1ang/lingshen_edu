import socket

client = socket.socket()
client.connect(('127.0.0.1', 9999))

while True:
    info = input('>>>')
    if input == 'bye':
        break
    info = info.encode('utf-8')
    client.send(info)
client.close()
