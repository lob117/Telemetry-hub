import json
import os
import psycopg2

def lambda_handler(event, context):
    try:
        # 1. Extraer datos del JSON enviado por el ESP32 [cite: 22]
        # El .get(key, default) evita que la Lambda falle si falta un dato [cite: 23]
        humedad = event.get('humidity', 0) [cite: 24]
        temperatura = event.get('temperature', 0) [cite: 25]
        sensacion = event.get('thermal_sensation', 0) [cite: 26]
        latitud = event.get('lat', 0) [cite: 27]
        longitud = event.get('lng', 0) [cite: 28]
        nivel = event.get('nivel', 0) [cite: 29]
        
        # Extraer el topic para saber qué sensor es [cite: 30]
        sensor_id = event.get('topic', 'sensor_desconocido') [cite: 31]
        print(f"Recibido de {sensor_id}: Temp {temperatura}°C, Hum {humedad}%") [cite: 32]

        # 2. Conexión a Neon (Usa la variable DATABASE_URL) [cite: 33]
        conexion = psycopg2.connect(os.environ['DATABASE_URL']) [cite: 34]
        cursor = conexion.cursor() [cite: 35]
        
        # 3. Consulta SQL para inserción [cite: 36]
        query = """
        INSERT INTO mediciones_iot_gps (sensor_id, temperatura, humedad, sensacion_termica, latitud, longitud)
        VALUES (%s, %s, %s, %s, %s, %s)
        """ [cite: 37, 38, 39, 40]

        cursor.execute(query, (sensor_id, temperatura, humedad, sensacion, latitud, longitud)) [cite: 41]
        conexion.commit() [cite: 42]
        
        cursor.close() [cite: 43]
        conexion.close() [cite: 44]
        
        return {'statusCode': 200, 'body': json.dumps('GPS Guardado en mediciones_iot_gps')} [cite: 45]
        
    except Exception as e:
        print(f"ERROR: {e}") [cite: 47]
        return {'statusCode': 500, 'body': json.dumps(f"Fallo: {str(e)}")} [cite: 48]