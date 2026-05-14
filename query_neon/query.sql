-- Creación de la tabla principal
CREATE TABLE mediciones_iot_gps (
    id SERIAL PRIMARY KEY,
    sensor_id VARCHAR(50) NOT NULL,
    temperatura NUMERIC(5,2) NOT NULL,
    humedad NUMERIC(5,2) NOT NULL,
    sensacion_termica NUMERIC(5,2) NOT NULL,
    Latitud NUMERIC(10,7) NOT NULL,
    Longitud NUMERIC(10,7) NOT NULL,
    fecha_hora TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    nivel NUMERIC(5,2) NOT NULL DEFAULT 0
); [cite: 56]

-- ==========================================
-- ROL 1: AWS Lambda Ingestión (Solo Escritura)
-- ==========================================
CREATE ROLE lambda_iot_user WITH LOGIN PASSWORD 'EscribeUnaContraseñaSegura123'; [cite: 59]
GRANT CONNECT ON DATABASE neondb TO lambda_iot_user; [cite: 61]
GRANT USAGE ON SCHEMA public TO lambda_iot_user; [cite: 63]
GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA public TO lambda_iot_user; [cite: 65]
GRANT INSERT ON TABLE mediciones_iot_gps TO lambda_iot_user; [cite: 69]

-- ==========================================
-- ROL 2: Grafana (Solo Lectura)
-- ==========================================
CREATE ROLE grafana_reader WITH LOGIN PASSWORD 'TuClaveSegura'; [cite: 76]
GRANT CONNECT ON DATABASE neondb TO grafana_reader; [cite: 77]
GRANT USAGE ON SCHEMA public TO grafana_reader; [cite: 78]
GRANT SELECT ON TABLE mediciones_iot_gps TO grafana_reader; [cite: 79]

-- ==========================================
-- ROL 3: API Gateway (Solo Lectura)
-- ==========================================
CREATE ROLE api_reader WITH LOGIN PASSWORD 'clave_segura_aqui'; [cite: 82]
GRANT CONNECT ON DATABASE neondb TO api_reader; [cite: 83]
GRANT USAGE ON SCHEMA public TO api_reader; [cite: 85]
GRANT SELECT ON TABLE mediciones_iot_gps TO api_reader; [cite: 87]