<img width="2132" height="2016" alt="reduce" src="https://github.com/user-attachments/assets/eb6b79a9-7ebc-42f6-8037-52edfe68f03c" />
# Telemetry-hub
# Serverless-IoT-Telemetry

Arquitectura Full-Stack en la nube para la ingesta, almacenamiento y visualización de telemetría IoT en tiempo real. 

[cite_start]Este proyecto captura datos ambientales y de geolocalización desde dispositivos ESP32, los procesa mediante infraestructura Serverless de AWS y los expone a través de un dashboard en Grafana y una aplicación web estática alojada en GitHub Pages / Cloudflare[cite: 138, 190].

## 🏗 Arquitectura del Sistema

![Arquitectura IoT](./assets/arquitectura.png)
*(Nota: Sube la imagen de tu diagrama a una carpeta llamada `assets` en este repositorio)*

El flujo de datos sigue esta ruta:
1. [cite_start]**Dispositivos Edge (ESP32):** Capturan temperatura, humedad, sensación térmica, nivel y coordenadas GPS[cite: 24, 25, 26, 27, 28, 29].
2. [cite_start]**AWS IoT Core:** Actúa como Broker MQTT y enruta los mensajes entrantes mediante reglas SQL[cite: 1, 4].
3. [cite_start]**AWS Lambda (Ingesta):** Procesa el payload JSON y ejecuta la inserción en la base de datos[cite: 16, 22, 38].
4. [cite_start]**Neon (PostgreSQL):** Almacenamiento transaccional Serverless[cite: 52].
5. [cite_start]**Capa de Presentación:** - **Grafana:** Dashboard de monitoreo en tiempo real conectado directamente a la base de datos[cite: 88, 106].
   - [cite_start]**AWS Lambda + API Gateway:** API REST pública para exponer los últimos registros[cite: 148, 183].
   - [cite_start]**GitHub Pages:** Portafolio web estático que consume la API mediante `fetch`[cite: 136, 176].

## 🚀 Guía de Despliegue

### 1. AWS IoT Core
* [cite_start]Registrar el dispositivo (Thing) y generar los certificados correspondientes[cite: 2].
* [cite_start]Crear una regla en el *Message Routing* con la siguiente sentencia SQL para capturar el payload y el topic[cite: 4]:
  ```sql
  SELECT *, topic() as topic FROM 'esp32/+'
