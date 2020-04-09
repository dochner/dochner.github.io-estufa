import serial
import time
from datetime import datetime

temp_old = 0
ilum_old = 0
sec_old = 0
ilum = 0
temp = 0

while 1:
    # INICIA COMUNICACAO COM O ARDUINO
    ser = serial.Serial('COM3', 9600)
    time.sleep(0.5)
    # LEITURA DE INFORMACOES DO ARDUINO
    arq0 = open('C:\\dev\\www\\estufaio\\arquivo.txt', 'w+')
    arq0.seek(0)
    lido = ser.readline()
    if (lido.startswith(b'T')):
        arq0.write(lido)
        arq0.seek(0)
        print(arq0.readline())
        arq0.close()
        # CONTROLE DE TEMPERATURA
        arq1 = open('C:\\dev\\www\\estufaio\\controle_temperatura.txt', 'r')
        arq1.seek(0)
        temp = arq1.read()
    if temp != temp_old:
        # ENVIA COMANDO AO ARDUINO
        ser.flushOutput()
        ser.flushInput()
        ser.write("temperatura" + temp)
        ser.flush()
        time.sleep(0.1)
        # ACK
        print(ser.readline())
        # ATUALIZA TEMP_OLD
        temp_old = temp
        arq1.close()
        # CONTROLE DE IRRIGACAO
    arq2 = open('C:\\dev\\www\\estufaio\\controle_irrigacao.txt', 'r+')
    arq2.seek(0)
    if (arq2.read() == "1"):
        # ENVIA COMANDO AO ARDUINO
        ser.flushOutput()
        ser.flushInput()
        ser.write("irrigar")
        ser.flush()
        time.sleep(0.1)
        # ACK
        print(ser.readline())
        # RESETA O ARQUIVO
        arq2.seek(0)
        arq2.write("0")
        arq2.close()
        # CONTROLE DE ILUMINACAO
    arq3 = open('C:\\dev\\www\\estufaio\\controle_iluminacao.txt', 'r')
    arq3.seek(0)
    ilum = arq3.read()
    if (ilum != ilum_old):
        # ENVIA COMANDO AO ARDUINO
        ser.flushOutput()
        ser.flushInput()
        ser.write("luz" + ilum)
        ser.flush()
        time.sleep(0.1)
        # ACK
        print(ser.readline())
        # ATUALIZA ILUM_OLD
        ilum_old = ilum
        arq3.close()
    ser.close()
