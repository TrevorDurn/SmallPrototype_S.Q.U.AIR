#include <WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "BME280.h"
#include "PMS5003.h"

// WiFi credentials
char ssid[] = "CU-LIONS";  // Change to your open WiFi SSID
// char password[] = "pledgeyard896"; // comment out if no password

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -14400, 60000); // -14400 seconds for EDT (UTC-4)

// Sensors
BME280 bme280;
PMS5003 pms5003(PB2, PB1); // RX, TX

// AWS IoT credentials
char mqttServer[] = "a2hwu90puqi62x-ats.iot.us-east-1.amazonaws.com";
char clientId[] = "Sensor_2"; // Sensor identifier
char publishTopic[] = "/secondsensor/1mindata"; // Change topics based on student
char publishPayload[MQTT_MAX_PACKET_SIZE];
char* subscribeTopic[5] = {
    "$aws/things/ameba/shadow/update/accepted",
    "$aws/things/ameba/shadow/update/rejected",
    "$aws/things/ameba/shadow/update/delta",
    "$aws/things/ameba/shadow/get/accepted",
    "$aws/things/ameba/shadow/get/rejected"
};

char* rootCABuff = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----\n";

char* certificateBuff = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDWjCCAkKgAwIBAgIVAPdQp2C12/Fm8l9JweWd8nTitpYUMA0GCSqGSIb3DQEB\n" \
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n" \
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNDA4MDcyMDM4\n" \
"MjFaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n" \
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC6l+JRuvSXg/td9Bbb\n" \
"K3HQbZOjNStq/q8EuD6ef0FictC7wNO1RBz4XYmAJgLWgpH+GstS4G617+6LM+On\n" \
"ZXr1vPytHSe0UPNY0rKI5siKleLek8DnkeGtg5I0pNm6Qdz+hyVv8hNuzdk4lHkB\n" \
"fMEzqvnZ/7ctO1M/6sFKqqR4dln+PTF0P3qpUIIe5Wsuekg+koATlI9hv/lOdtgt\n" \
"hHaL+oURRlt3LwyZyJkXaEL7foMn7aL+mKBp8nLxSNqbfcdkK7Ee7v76d/XhIir+\n" \
"WLTULM0fQxQH/wc1dDnWQptqqSuSsPRbG98Y+E+oeawfjCRQiYC0X7mPgiWQmPVD\n" \
"Uq9/AgMBAAGjYDBeMB8GA1UdIwQYMBaAFBRePl1oOIvSB4R/bSwVzus+ICOkMB0G\n" \
"A1UdDgQWBBS0IkrRtMs7MhceC9ye09aX8av4NTAMBgNVHRMBAf8EAjAAMA4GA1Ud\n" \
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEArIkeIf0FqmIc56fGiFJ4Poqw\n" \
"0jrPStHB3icaQLSlECsN9cW6jt4gAxfawjofVXFZn2OuGLeU1mGhGn0xeMmuJN+E\n" \
"U6TCQQpnYIJ4hOgHbMmfM3oAcX7mp/Yo5bZ4dTFwMHXNA9l8EmZMK6ijdPhQGtAw\n" \
"hOcu44LQ+3cRe9c2qeU0AuyGT9zluFnBrwH1MywcjO4ULCLiadGX4WwbvvI4Jgps\n" \
"DEj+BBKFjZ+H7hwhHi/j8F6Z2Hcmg+sO3eu84osks0AY/b4GiFcJ35OWVf5JxugU\n" \
"rkCi6oIwJFLDHQ7vdH3qBLj6jAJlpACofVkq5vGdoWJ1kYySA877PN7dm+b/HA==\n" \
"-----END CERTIFICATE-----\n";

char* privateKeyBuff = \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEogIBAAKCAQEAupfiUbr0l4P7XfQW2ytx0G2TozUrav6vBLg+nn9BYnLQu8DT\n" \
"tUQc+F2JgCYC1oKR/hrLUuBute/uizPjp2V69bz8rR0ntFDzWNKyiObIipXi3pPA\n" \
"55HhrYOSNKTZukHc/oclb/ITbs3ZOJR5AXzBM6r52f+3LTtTP+rBSqqkeHZZ/j0x\n" \
"dD96qVCCHuVrLnpIPpKAE5SPYb/5TnbYLYR2i/qFEUZbdy8MmciZF2hC+36DJ+2i\n" \
"/pigafJy8Ujam33HZCuxHu7++nf14SIq/li01CzNH0MUB/8HNXQ51kKbaqkrkrD0\n" \
"WxvfGPhPqHmsH4wkUImAtF+5j4IlkJj1Q1KvfwIDAQABAoIBAHiq7HNKYg7yDUbv\n" \
"KTDHYa0Wj9mF8vQYi/qTY/t/9DrEYXRp+P9Kcymy4875xfAPNHaNwVtxRGdoKG3h\n" \
"OqHwoJn2g7k4F4smppiUeVzLUnk9ASBLkP6weq+JXK2qxhsLqOz5XR6OAD8x4/4R\n" \
"Fkf987uEIFv4YO9hZC77k9DDleVIF5PIhzfgQXyeIqB4k69LGohoews1/CDfNRUM\n" \
"pphDdolr/EeVV4Y7JCidVaVDm2YR/PR83I0N2LUoeDpxUWjx2idt7GBOMMvXKp/2\n" \
"dhRXfHsAKX9yVBAyER9RSsRuKMJK+DWR1zuA5DNWIgOZKFo1oWgX4HqL6YlQqXGY\n" \
"eQxXXjECgYEA9vEUGbSn/Yeh7al4PTxcDxRPAwNpXJewh+AY6PaNFRTzgz9myBnR\n" \
"0O+nJUp+8a+b073DVcnAcDbjTmrdgqh3uumb89PrfSDogqlCgbKQQUI5AX7iSFbo\n" \
"c4RGcn02FpD4lMuY8T8KC7DlvKft0VPk4DBlN1sJdcA71s7P9Y7z6sMCgYEAwXAZ\n" \
"mXF7VTAVlWvmUqKlqPMoa1GRIFx4GPzMf4xW7vWtXvCPc04sw2pux1ij39AvEI1L\n" \
"9sUuXFLlgSpqjBA9LY9Nq/rqeDWXMOB53rMTJA18Etsmuijd1qWBp19G4p4DZgFM\n" \
"PmrSg18FrRLdkXIpD/mrWzOYRdTwcjxzuuaQBJUCgYBYQtJsXaWteEzoJSpUuL4u\n" \
"pZOYACAoeA8ggXhly7mhT1u6DW8vFgtoIkAblATLXXjUtX6bqwCdGwEjhg6mJJCm\n" \
"vdh/zb7demWgTd73fFsS+Bhn8HHwaR+sKhsD0L0EU8yFYj/H+kAc7Z0vUNYve0zS\n" \
"1HlYz7ER5SN+I20w12dXAQKBgBZO3UTq53RNlU+kH+3LwBsZ38FwqLH4uAj5roPP\n" \
"JX9tac8XIxQDIfpvXdMzj8KR+buI8AfHea9ACCO7RopcnRGSAz0gNMYkOAl0+dkL\n" \
"AdwYigSKrY6ug9brQ2aQAFbGLlInOCnCnAB2husMz+ykI8Qp4O8lphv3kLs76YTW\n" \
"bil5AoGAZPEvySmZctca/zsvEWqac0t1P/kgKAUEnylJolOlV1hc2ZxA7Pnp7PfX\n" \
"dGRWM1fXDsO3RXH3HZk8wX/ik20PYj06bGUjau3N4mOcQOlEE/XCK/6+t64tfLRX\n" \
"RHkNPvyBVqdvyj/VW4UH51RzeL8GHVsdm9hQZmKhDmJ2QzkLqIc=\n" \
"-----END RSA PRIVATE KEY-----\n";

WiFiSSLClient wifiClient;
PubSubClient client(wifiClient);

void epochToDateTime(unsigned long epoch, int &year, int &month, int &day, int &hour, int &minute, int &second) {
    // Debugging epoch time
    Serial.print("Epoch time: ");
    Serial.println(epoch);

    second = epoch % 60;
    epoch /= 60;
    minute = epoch % 60;
    epoch /= 60;
    hour = epoch % 24;
    epoch /= 24;

    unsigned long days = epoch;
    year = 1970;

    while (true) {
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        unsigned long daysInYear = leap ? 366 : 365;
        if (days >= daysInYear) {
            days -= daysInYear;
            year++;
        } else {
            break;
        }
    }

    static const int daysInMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    for (month = 0; month < 12; month++) {
        int dim = daysInMonth[month];
        if (month == 1) {
            bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            if (leap) {
                dim++;
            }
        }
        if ((unsigned long)days >= (unsigned long)dim) {
            days -= dim;
        } else {
            break;
        }
    }
    month++;
    day = days + 1;
}

void callback(char* topic, byte* payload, unsigned int length) {
    char buf[MQTT_MAX_PACKET_SIZE];
    strncpy(buf, (const char *)payload, length);
    buf[length] = '\0';
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.println(buf);
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(clientId)) {
            Serial.println("connected");

            for (int i = 0; i < 5; i++) {
                client.subscribe(subscribeTopic[i]);
                Serial.print("Subscribed to topic: ");
                Serial.println(subscribeTopic[i]);
            }
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(ssid); // Include password if needed in the form (ssid, password);
    Serial.println("Connecting to WiFi...");

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Initialize sensors
    Serial.println("Initializing BME280...");
    if (!bme280.begin()) {
        Serial.println("BME280 initialization failed");
        while (1);
    }
    Serial.println("BME280 initialized successfully");

    Serial.println("Initializing PMS5003...");
    pms5003.begin();
    Serial.println("PMS5003 initialized successfully");

    // Initialize NTP Client
    Serial.println("Initializing NTP client...");
    timeClient.begin();
    Serial.println("NTP client initialized");

    wifiClient.setRootCA((unsigned char*)rootCABuff);
    wifiClient.setClientCertificate((unsigned char*)certificateBuff, (unsigned char*)privateKeyBuff);

    client.setServer(mqttServer, 8883);
    client.setCallback(callback);

    delay(1500);
    Serial.println("Setup complete.");
}

void loop() {
    if (!client.connected()) {
        Serial.println("MQTT client not connected. Attempting reconnect...");
        reconnect();
    }
    client.loop();

    // Update NTP Client
    Serial.println("Updating NTP client...");
    timeClient.update();

    // Get current time
    unsigned long epochTime = timeClient.getEpochTime();
    Serial.print("Epoch time received: ");
    Serial.println(epochTime);

    // Convert epoch time to local time
    int year, month, day, hour, minute, second;
    epochToDateTime(epochTime, year, month, day, hour, minute, second);

    // Print current date and time
    Serial.print("Current NYC time: ");
    Serial.print(month);
    Serial.print("/");
    Serial.print(day);
    Serial.print("/");
    Serial.print(year);
    Serial.print(" ");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(second);

    // Read PMS5003 data
    int pm10, pm25, pm100;
    Serial.println("Reading PMS5003 data...");
    if (pms5003.read(pm10, pm25, pm100)) {
        Serial.print("PM2.5: ");
        Serial.print(pm25);
        Serial.println(" ug/m3");
    } else {
        Serial.println("Failed to read PMS5003 data");
    }

    // Read BME280 data
    float temperature, humidity;
    Serial.println("Reading BME280 data...");
    bme280.read(temperature, humidity);
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Publish sensor data to AWS IoT
    Serial.println("Publishing data to AWS IoT...");
    sprintf(publishPayload, "{\"state\":{\"reported\":{\"time\":\"%02d/%02d/%04d %02d:%02d:%02d\",\"pm25\":%d,\"temperature\":%.2f,\"humidity\":%.2f}},\"clientToken\":\"%s\"}",
        month, day, year, hour, minute, second, pm25, temperature, humidity, clientId);
    client.publish(publishTopic, publishPayload);
    Serial.print("Publish [");
    Serial.print(publishTopic);
    Serial.print("] ");
    Serial.println(publishPayload);

    delay(10000);
}
