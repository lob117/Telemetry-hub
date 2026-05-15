#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>
#include <rom/rtc.h>

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_BAUD 9600
#define LOG_FILE "/gps_log.txt"
#define BOOT_FILE "/boot_count.txt"
#define LOG_INTERVAL 5000
#define MAX_FILE_KB 800

TinyGPSPlus gps;
HardwareSerial GPS_Serial(2);

unsigned long lastLog = 0;
unsigned long lastInfo = 0;
unsigned long startTime = 0;
bool gotFix = false;
bool rawMode = false;
unsigned long rawUntil = 0;
uint32_t bootCount = 0;

// ─── Causa del reset ─────────────────────────────────────
String getResetReason()
{
    switch (rtc_get_reset_reason(0))
    {
    case 1:
        return "Power on";
    case 3:
        return "Software reset";
    case 4:
        return "Legacy watchdog";
    case 5:
        return "Deep sleep";
    case 12:
        return "BROWNOUT (voltaje bajo)";
    case 14:
        return "WATCHDOG (codigo colgado)";
    default:
        return "Otro(" + String(rtc_get_reset_reason(0)) + ")";
    }
}

// ─── SPIFFS ──────────────────────────────────────────────
void initSPIFFS()
{
    if (!SPIFFS.begin(false))
    {
        Serial.println("[SPIFFS] Formateando por primera vez...");
        SPIFFS.begin(true);
    }
    Serial.printf("[SPIFFS] Montado. Usado: %u KB / %u KB\n",
                  SPIFFS.usedBytes() / 1024, SPIFFS.totalBytes() / 1024);
}

// ─── Contador de arranques ───────────────────────────────
uint32_t readBootCount()
{
    if (!SPIFFS.exists(BOOT_FILE))
        return 0;
    File f = SPIFFS.open(BOOT_FILE, "r");
    if (!f)
        return 0;
    uint32_t n = f.readString().toInt();
    f.close();
    return n;
}

void writeBootCount(uint32_t n)
{
    File f = SPIFFS.open(BOOT_FILE, "w");
    if (!f)
    {
        Serial.println("[BOOT] Error guardando contador");
        return;
    }
    f.print(n);
    f.close();
}

// ─── Log de arranques ────────────────────────────────────
void logBoot(uint32_t n, String reason)
{
    File f = SPIFFS.open(LOG_FILE, "a");
    if (!f)
        return;
    char buf[120];
    snprintf(buf, sizeof(buf),
             "# ARRANQUE %lu | causa: %s\n", (unsigned long)n, reason.c_str());
    f.print(buf);
    f.close();
}

void writeHeader()
{
    if (!SPIFFS.exists(LOG_FILE))
    {
        File f = SPIFFS.open(LOG_FILE, "w");
        if (f)
        {
            f.println("millis,lat,lon,alt_m,satelites,hdop,age_ms,arranque");
            f.close();
        }
    }
}

// ─── Guardar punto GPS ───────────────────────────────────
void logPoint(float lat, float lon, float alt,
              uint32_t sats, float hdop, uint32_t age)
{
    if (SPIFFS.exists(LOG_FILE))
    {
        File f = SPIFFS.open(LOG_FILE, "r");
        size_t sz = f ? f.size() : 0;
        if (f)
            f.close();
        if (sz > MAX_FILE_KB * 1024)
        {
            SPIFFS.remove(LOG_FILE);
            Serial.println("[SPIFFS] Archivo rotado por tamaño");
            writeHeader();
            logBoot(bootCount, "rotacion de archivo");
        }
    }
    File f = SPIFFS.open(LOG_FILE, "a");
    if (!f)
    {
        Serial.println("[SPIFFS] Error escribiendo");
        return;
    }
    char buf[140];
    snprintf(buf, sizeof(buf),
             "%lu,%.6f,%.6f,%.1f,%u,%.2f,%lu,%lu\n",
             millis(), lat, lon, alt, sats, hdop,
             (unsigned long)age, (unsigned long)bootCount);
    f.print(buf);
    f.close();
}

// ─── Comandos Serial ─────────────────────────────────────
void cmdDump()
{
    if (!SPIFFS.exists(LOG_FILE))
    {
        Serial.println("[LOG] No existe archivo todavia.");
        return;
    }
    File f = SPIFFS.open(LOG_FILE, "r");
    Serial.printf("\n=== LOG (%u bytes) ===\n", f.size());
    while (f.available())
        Serial.write(f.read());
    Serial.println("=== FIN ===");
    f.close();
}

void cmdErase()
{
    SPIFFS.remove(LOG_FILE);
    SPIFFS.remove(BOOT_FILE);
    bootCount = 0;
    Serial.println("[LOG] Archivos borrados. Contador de arranques a 0.");
    writeHeader();
}

void cmdStatus()
{
    Serial.println("\n─── Estado ───────────────────────────");
    Serial.printf("  Arranque numero  : %lu\n", (unsigned long)bootCount);
    Serial.printf("  Tiempo encendido : %lu s\n", millis() / 1000);
    Serial.printf("  Causa ultimo boot: %s\n", getResetReason().c_str());
    Serial.printf("  Chars GPS        : %lu\n", gps.charsProcessed());
    Serial.printf("  Fix obtenido     : %s\n", gotFix ? "SI" : "NO");
    if (gps.satellites.isValid())
        Serial.printf("  Satelites        : %u\n", gps.satellites.value());
    if (gps.location.isValid())
        Serial.printf("  Posicion         : %.6f, %.6f\n",
                      gps.location.lat(), gps.location.lng());
    if (SPIFFS.exists(LOG_FILE))
    {
        File f = SPIFFS.open(LOG_FILE, "r");
        Serial.printf("  Archivo log      : %u bytes\n", f ? f.size() : 0);
        if (f)
            f.close();
    }
    else
    {
        Serial.println("  Archivo log      : no existe aun");
    }
    Serial.printf("  SPIFFS libre     : %u KB\n",
                  (SPIFFS.totalBytes() - SPIFFS.usedBytes()) / 1024);
    Serial.println("──────────────────────────────────────");
}

void cmdRaw()
{
    rawMode = true;
    rawUntil = millis() + 10000;
    Serial.println("[RAW] Mostrando NMEA crudo por 10 segundos...");
}

void handleSerial()
{
    if (!Serial.available())
        return;
    char c = Serial.read();
    while (Serial.available())
        Serial.read();
    switch (c)
    {
    case 'd':
    case 'D':
        cmdDump();
        break;
    case 'e':
    case 'E':
        cmdErase();
        break;
    case 's':
    case 'S':
        cmdStatus();
        break;
    case 'r':
    case 'R':
        cmdRaw();
        break;
    default:
        Serial.println("Comandos: d=dump  e=erase  s=status  r=raw");
    }
}

// ─── Setup ───────────────────────────────────────────────
void setup()
{
    Serial.begin(115200);
    delay(600);

    String reason = getResetReason();

    initSPIFFS();

    // Leer contador, incrementar y guardar ANTES de cualquier otra cosa
    bootCount = readBootCount() + 1;
    writeBootCount(bootCount);
    writeHeader();
    logBoot(bootCount, reason);

    Serial.println("\n==============================");
    Serial.printf("  ARRANQUE #%lu\n", (unsigned long)bootCount);
    Serial.printf("  Causa: %s\n", reason.c_str());
    Serial.println("  Comandos: d e s r");
    Serial.println("==============================\n");

    GPS_Serial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    startTime = millis();
}

// ─── Loop ────────────────────────────────────────────────
void loop()
{
    handleSerial();

    while (GPS_Serial.available())
    {
        char c = GPS_Serial.read();
        if (rawMode)
            Serial.write(c);
        gps.encode(c);
    }

    if (rawMode && millis() > rawUntil)
    {
        rawMode = false;
        Serial.println("\n[RAW] Fin. Comandos: d e s r");
    }

    if (!gotFix && millis() - lastInfo > 8000)
    {
        lastInfo = millis();
        Serial.printf("[%3lu s] Chars: %lu | Sats: ",
                      millis() / 1000, gps.charsProcessed());
        Serial.println(gps.satellites.isValid() ? String(gps.satellites.value()) : "--");
    }

    if (gps.location.isValid() && gps.location.age() < 2000)
    {
        if (!gotFix)
        {
            gotFix = true;
            Serial.printf("\n[FIX] Primera posicion a los %lu s\n",
                          millis() / 1000);
        }
        if (millis() - lastLog >= LOG_INTERVAL)
        {
            lastLog = millis();
            float lat = gps.location.lat();
            float lon = gps.location.lng();
            float alt = gps.altitude.isValid() ? gps.altitude.meters() : 0;
            uint32_t s = gps.satellites.isValid() ? gps.satellites.value() : 0;
            float hdop = gps.hdop.isValid() ? gps.hdop.hdop() : 99.9;
            logPoint(lat, lon, alt, s, hdop, gps.location.age());
            Serial.printf("[LOG] Arranque #%lu | %.6f, %.6f | Sats: %u\n",
                          (unsigned long)bootCount, lat, lon, s);
        }
    }
}