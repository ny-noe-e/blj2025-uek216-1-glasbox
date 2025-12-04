#include <WiFi.h>
#include <Adafruit_AHTX0.h>
#include <ESP32MQTTClient.h>
#include "esp_idf_version.h"

const char *ssid = "GuestWLANPortal";
const char *server = "mqtt://10.10.2.127:1883";

const char *pub_topic = "zuerich/glasbox/glasbox";
const char *sub_topic = "zuerich/glasbox/glasbox";

const char *client_id = "ESP32 Glasbox";

Adafruit_AHTX0 aht;
ESP32MQTTClient client;

void setup() {
  Serial.begin(115200);

  setup_sensor();
  setup_wifi();

  client.setURI(server);
  client.setMqttClientName(client_id);
  client.loopStart();
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  std::string msg = std::to_string(temp.temperature);
  client.publish(pub_topic, msg);

  delay(1000);
}

void setup_sensor() {
  Serial.print("Initialising AHT10 / AHT20...");

  while (!aht.begin()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("done!");
}

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("done!");
}

void onMqttConnect(esp_mqtt_client_handle_t client_handle) {
  if (client.isMyTurn(client_handle)) {
    client.subscribe(sub_topic, [](const std::string &payload) {
      Serial.printf("%s: %s\n", sub_topic, payload.c_str());
    });
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