import json
import psycopg2
import os

def lambda_handler(event, context):
    # Conexión a Neon utilizando la variable de entorno [cite: 153, 154]
    neon_url = os.environ['DATABASE_URL'] 
    
    conn = psycopg2.connect(neon_url) [cite: 155]
    cur = conn.cursor() [cite: 156]

    # Consulta: Traer el último dato de la tabla [cite: 157]
    query = "SELECT temperatura, humedad,sensacion_termica, nivel FROM mediciones_iot_gps ORDER BY fecha_hora DESC LIMIT 1" [cite: 158]
    cur.execute(query) [cite: 159]
    row = cur.fetchone() [cite: 160]

    # Cerrar conexión [cite: 161]
    cur.close() [cite: 162]
    conn.close() [cite: 163]

    # Formatear la respuesta para el Frontend convirtiendo Decimal a float normal [cite: 164, 165]
    # Se usa 'if row[X] is not None' para evitar errores si un sensor se desconectó y mandó NULL [cite: 166]
    body = {
        "temperatura": float(row[0]) if row[0] is not None else None, [cite: 168]
        "humedad": float(row[1]) if row[1] is not None else None, [cite: 169]
        "sensacion_termica": float(row[2]) if row[2] is not None else None, [cite: 170]
        "nivel": float(row[3]) if row[3] is not None else None [cite: 171]
    } [cite: 167, 172]

    return {
        'statusCode': 200, [cite: 174]
        'headers': {
            'Access-Control-Allow-Origin': '*', # Permitir acceso desde GitHub Pages (CORS) [cite: 176]
            'Content-Type': 'application/json' [cite: 177]
        }, [cite: 175, 178]
        'body': json.dumps(body) [cite: 179]
    } [cite: 173, 180]