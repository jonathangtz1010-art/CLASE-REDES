import socket
import serial
import time

HOST = "0.0.0.0"
PORT = 5001
SERIAL_PORT = "COM9"
BAUDRATE = 9600

arduino = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=2)
time.sleep(2)

def leer_arduino():
    arduino.reset_input_buffer()
    arduino.write(b"GET_DATA\n")
    respuesta = arduino.readline().decode("utf-8").strip()
    return respuesta

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(5)

print(f"Servidor TCP escuchando en {HOST}:{PORT}")

while True:
    conn, addr = server.accept()
    print("Conexion desde:", addr)

    try:
        data = conn.recv(1024).decode("utf-8").strip()

        if data == "GET_DATA":
            respuesta = leer_arduino()

            if respuesta:
                conn.sendall(respuesta.encode("utf-8"))
            else:
                conn.sendall(b'{"error":"Sin respuesta del Arduino"}')
        else:
            conn.sendall(b'{"error":"Comando no valido"}')

    except Exception as e:
        mensaje = f'{{"error":"{str(e)}"}}'
        conn.sendall(mensaje.encode("utf-8"))

    finally:
        conn.close()
