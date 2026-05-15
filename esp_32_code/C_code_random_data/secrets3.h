#include <pgmspace.h>
// #define AWS_IOT_PUBLISH_TOPIC "esp32/pub"
// #define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

#define SECRET
#define THINGNAME "esp32_DTH_n3" // change this

const char WIFI_SSID[] = "";        // wifi
const char WIFI_PASSWORD[] = "";    // wifi pastword
const char AWS_IOT_ENDPOINT[] = ""; // aws_endpoint

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";

// Device Certificate                                               //change this
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)KEY";

// Device Private Key                                               //change this
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
)KEY";