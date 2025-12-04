#include <WiFi.h>
#include <Adafruit_AHTX0.h>
#include <ESP32MQTTClient.h>
#include "esp_idf_version.h"
#include <MaxLedControl.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define DIN 23
#define CLK 18
#define CS  2
#define NUM_MODULES 1

#define A 5

const char *ssid = "GuestWLANPortal";
const char *server = "mqtt://10.10.2.127:1883";

const char *pub_topic = "zuerich/glasbox/data";
const char *sub_topic = "zuerich/glasbox/data";

const char *client_id = "ESP32 Glasbox";

Adafruit_AHTX0 aht;
ESP32MQTTClient client;

LedControl display = LedControl(DIN, CLK, CS, 1);
 
void setup() {
  Serial.begin(115200);
  display.begin(15);
  display.clear();
  setup_sensor();
  setup_wifi();
  client.setURI(server);
  client.setMqttClientName(client_id);
  client.loopStart();
  pinMode(A, OUTPUT);
}

void displayString(const char* s) {
  uint16_t len = (uint16_t)strlen(s);

  for (uint16_t i = 0; i < len; i++) {
    display.setChar(0, 8 - i - 1, s[i], false);
  }
  Serial.println(s);
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  const char* s = "67";
  displayString(s);
  float temperature = temp.temperature;
  std::string msg = std::to_string(temperature);
  client.publish(pub_topic, msg);
  if (temperature > 28.5){
    digitalWrite(A, HIGH);
  }
  else {
    digitalWrite(A, LOW);
  }
  delay(1000);
}
 
void setup_sensor() {
  while (!aht.begin()) {
    delay(500);
  }
}
 
void setup_wifi() {
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}
 
void onMqttConnect(esp_mqtt_client_handle_t client_handle) {
  if (client.isMyTurn(client_handle)) {
    client.subscribe(sub_topic, [](const std::string &payload) {});
  }
}
 
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
esp_err_t handleMQTT(esp_mqtt_event_handle_t event) {
  client.onEventCallback(event);
  return ESP_OK;
}
#else
void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  client.onEventCallback(event);
}
#endif