import cv2
import numpy as np
import pyzbar.pyzbar as pyzbar
import urllib.request
import requests
import time
import mysql.connector

def agregar_libro(codigo):
    try:
        conn = mysql.connector.connect(**db_config)
        cursor = conn.cursor()
        query = "SELECT id, cantidad, cant_max FROM libros WHERE codigo = %s"
        cursor.execute(query, (codigo,))
        resultado = cursor.fetchone()

        if resultado:
            libro_id, cantidad_actual, cantidad_maxima = resultado
            if cantidad_actual + 1 > cantidad_maxima:
                print("Error: Se excede la cantidad máxima de libros.")
                error_led()
            else:
                cursor.execute("UPDATE libros SET cantidad = cantidad + 1 WHERE codigo = %s", (codigo,))
                conn.commit()
                print("Libro agregado exitosamente.")
                query_pedido = """
                    INSERT INTO pedido (libro_id, fecha_hora, estado) 
                    VALUES (%s, NOW(), 'agregado')
                """
                cursor.execute(query_pedido, (libro_id,))
                conn.commit()
                leido_led()
        else:
            print("Código QR no encontrado.")
            error_led()
    finally:
        cursor.close()
        conn.close()

def quitar_libro(codigo):
    try:
        conn = mysql.connector.connect(**db_config)
        cursor = conn.cursor()
        query = "SELECT id, cantidad FROM libros WHERE codigo = %s"
        cursor.execute(query, (codigo,))
        resultado = cursor.fetchone()

        if resultado:
            libro_id, cantidad_actual = resultado
            if cantidad_actual == 0:
                print("Error: No hay libros para quitar.")
                error_led()
            else:
                cursor.execute("UPDATE libros SET cantidad = cantidad - 1 WHERE codigo = %s", (codigo,))
                conn.commit()
                print("Libro quitado exitosamente.")
                query_pedido = """
                    INSERT INTO pedido (libro_id, fecha_hora, estado) 
                    VALUES (%s, NOW(), 'sacado')
                """
                cursor.execute(query_pedido, (libro_id,))
                conn.commit()
                
                leido_led()
        else:
            print("Código QR no encontrado.")
            error_led()
    finally:
        cursor.close()
        conn.close()

def leido_led():
    response = requests.get(f"{url}leido/on")
    print(response.text)  # Muestra la respuesta del servidor (LED Encendido)
def error_led():
    response = requests.get(f"{url}error/on")
    print(response.text)  # Muestra la respuesta del servidor (LED Encendido)

def boton_funcion():
    response = requests.get(f"{url}botonfuncion/on")
    return response.text
    
 
#cap = cv2.VideoCapture(0)
font = cv2.FONT_HERSHEY_PLAIN
 
url='http://192.168.1.42/'
cv2.namedWindow("live transmission", cv2.WINDOW_AUTOSIZE)
 
prev=""
pres=""
last_scan_time = 0 
scan_interval = 4

db_config = {
    'host': 'bvwcwoictlrqnykvv7dv-mysql.services.clever-cloud.com',
    'database': 'bvwcwoictlrqnykvv7dv',
    'user': 'ud2nxn5xt1xxrc4z',
    'password': 'GV15dGwYl5OieOUimgqu',
    'port': 3306
}

while True:
    img_resp=urllib.request.urlopen(url+'cam-hi.jpg')
    imgnp=np.array(bytearray(img_resp.read()),dtype=np.uint8)
    frame=cv2.imdecode(imgnp,-1)
    #_, frame = cap.read()
    decodedObjects = pyzbar.decode(frame)
    for obj in decodedObjects:
        if obj.type == 'QRCODE':
            pres=obj.data
            current_time = time.time()
            if pres != prev or (current_time - last_scan_time > scan_interval):
                if boton_funcion() == "0":
                    agregar_libro(obj.data.decode('utf-8'))
                elif boton_funcion() == "1":
                    quitar_libro(obj.data.decode('utf-8'))
                print("Type:", obj.type)
                print("Data:", obj.data)
                prev = pres
                last_scan_time = current_time 
            
                cv2.putText(frame, str(obj.data), (50, 50), font, 2,
                        (255, 0, 0), 3)
            
    cv2.imshow("live transmission", frame)
 
    key = cv2.waitKey(1)
    if key == 27:
        break
 
cv2.destroyAllWindows()
