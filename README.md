<img width="200" height="200" alt="reduce" src="https://github.com/user-attachments/assets/eb6b79a9-7ebc-42f6-8037-52edfe68f03c" />


# Telemetry-hub
# Serverless-IoT-Telemetry

Arquitectura Full-Stack en la nube para la ingesta, almacenamiento y visualización de telemetría IoT en tiempo real. 

Este proyecto captura datos ambientales y de geolocalización desde dispositivos ESP32, los procesa mediante infraestructura Serverless de AWS y los expone a través de un dashboard en Grafana y una aplicación web estática alojada en GitHub Pages / Cloudflare.

## 🏗 Arquitectura del Sistema

![Arquitectura IoT](./assets/arquitectura.png)
*(Nota: Asegúrate de crear una carpeta `assets` y subir allí la imagen de tu arquitectura)*

El flujo de datos sigue esta ruta:
1. **Dispositivos Edge (ESP32):** Capturan temperatura, humedad, sensación térmica, nivel y coordenadas GPS.
2. **AWS IoT Core:** Actúa como Broker MQTT y enruta los mensajes entrantes mediante reglas SQL.
3. **AWS Lambda (Ingesta):** Procesa el payload JSON y ejecuta la inserción en la base de datos.
4. **Neon (PostgreSQL):** Almacenamiento transaccional Serverless.
5. **Capa de Presentación:**
   - **Grafana:** Dashboard de monitoreo en tiempo real conectado directamente a la base de datos.
   - **AWS Lambda + API Gateway:** API REST pública para exponer los últimos registros.
   - **GitHub Pages:** Portafolio web estático que consume la API mediante `fetch`.

## 🚀 Guía de Despliegue

### 1. AWS IoT Core
* Registrar el dispositivo (Thing) y generar los certificados correspondientes.
* Crear una regla en el *Message Routing* con la siguiente sentencia SQL para capturar el payload y el topic:
  > `SELECT *, topic() as topic FROM 'esp32/+'`

### 2. Base de Datos (Neon PostgreSQL)
* Ejecutar el script `query_neon/query.sql` para crear la tabla principal `mediciones_iot_gps`.
* Por seguridad (Principio de Mínimos Privilegios), se crean tres usuarios distintos:
  * `lambda_iot_user`: Permisos exclusivos para hacer `INSERT`.
  * `grafana_reader`: Permisos exclusivos para hacer `SELECT` y alimentar el dashboard.
  * `api_reader`: Permisos exclusivos para hacer `SELECT` desde la API web.

### 3. AWS Lambda
* **Dependencias:** Es necesario compilar la librería `psycopg2` para el entorno Linux de AWS. Se deben ejecutar los comandos de empaquetado y publicación de la *Layer* utilizando AWS CloudShell.
* Configurar las Lambdas (`AWS_lambda/lambda_ingestion.py` y `AWS_lambda/lambda_api.py`) con la variable de entorno `DATABASE_URL` correspondiente a cada usuario de Neon.

### 4. Visualización (Grafana)
* Añadir PostgreSQL como *Data Source* usando la cadena de conexión del usuario `grafana_reader` con TLS/SSL requerido.
* Crear paneles de visualización usando la variable `$__timeFilter(fecha_hora)` para el manejo dinámico del tiempo.
* Implementar variables de dashboard (Ej. `sensor_id`) para filtrar datos de nodos específicos.

### 5. Frontend
* El sitio web estático (`index.html`) se despliega activando GitHub Pages desde la rama `main`.
