import time
import os
import subprocess
import random

folder_name = "logs/logs_mil_carros" + str(time.time()).split('.')[0].zfill(12)
if not os.path.exists("logs"):
    os.makedirs("logs")
os.makedirs(folder_name)
print "Insira o numero de carros desejados: "
num_carros = input()
# init servers process
server_log = open(folder_name + '/server.log', 'wb')
server_log_error = open(folder_name + '/server_error.log', 'wb')
server = subprocess.Popen(["./server"], stdout=server_log)
server_media_log = open(folder_name + '/server_media.log', 'wb')
server_media_log_error = open(folder_name + '/server_media_error.log', 'wb')
server_media = subprocess.Popen(["./server_media"], stdout=server_media_log)
clients = []
digits = len(str(num_carros))
for i in range(0, num_carros):
    tipo = random.randint(0, 2)  # gera um tipo aleatorio
    eixo = random.randint(0, 1)  # gera um tipo aleatorio
    sentido = random.randint(0, 1)  # gera um tipo aleatorio
    client_log = open(folder_name + '/client' + str(i).zfill(digits) + '.log', 'w')
    clients.append(subprocess.Popen(["./client", "localhost", "localhost", str(tipo), str(eixo), str(sentido)], stdout=client_log))

command = str(raw_input())
while command != 'kill':
    print "nada a fazer\n"
    command = str(raw_input())

server.kill()
server_media.kill()

