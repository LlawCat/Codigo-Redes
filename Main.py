import network
import socket
import time
from machine import Pin

# Configuración WiFi
WIFI_SSID = "Lanita"
WIFI_PASSWORD = "lanita16ali"

# Configuración del relé
rele = Pin(16, Pin.OUT)  # Cambia el número de pin según tu conexión
rele.value(0)  # Inicia con relé apagado

# Conectar a WiFi
def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(WIFI_SSID, WIFI_PASSWORD)
    
    max_attempts = 10
    for _ in range(max_attempts):
        if wlan.isconnected():
            print("Conectado a WiFi")
            print("IP:", wlan.ifconfig()[0])
            return True
        print("Conectando...")
        time.sleep(1)
    
    print("Error de conexión")
    return False

# Servidor TCP para recibir comandos
def start_server():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(('0.0.0.0', 80))
    s.listen(1)
    print("Servidor iniciado. Esperando conexiones...")
    
    while True:
        conn, addr = s.accept()
        print("Conexión desde:", addr)
        request = conn.recv(1024)
        
        if b'HIGH' in request:
            print("Recibido HIGH - Encendiendo relé")
            rele.value(1)
            conn.send("HTTP/1.1 200 OK\r\n\r\nRele encendido")
        elif b'LOW' in request:
            print("Recibido LOW - Apagando relé")
            rele.value(0)
            conn.send("HTTP/1.1 200 OK\r\n\r\nRele apagado")
        else:
            conn.send("HTTP/1.1 200 OK\r\n\r\nComando no reconocido")
        
        conn.close()

if connect_wifi():
    start_server()
