#if SK_MQTT
#include "mqtt_task.h"

#include "motor_task.h"
#include "secrets.h"


MQTTTask::MQTTTask(const uint8_t task_core, MotorTask& motor_task, Logger& logger) :
        Task("MQTT", 4096, 1, task_core),
        motor_task_(motor_task),
        logger_(logger),
        wifi_client_(),
        mqtt_client_(wifi_client_) {
    auto callback = [this](char *topic, byte *payload, unsigned int length) { mqttCallback(topic, payload, length); };
    mqtt_client_.setCallback(callback);
}

void MQTTTask::connectWifi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        logger_.log("Establishing connection to WiFi..");
    }

    char buf[256];
    snprintf(buf, sizeof(buf), "Connected to network %s", WIFI_SSID);
    logger_.log(buf);
}

void MQTTTask::mqttCallback(char *topic, byte *payload, unsigned int length) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Received mqtt callback for topic %s, length %u", topic, length);
    logger_.log(buf);
}

void MQTTTask::connectMQTT() {
    char buf[256];
    mqtt_client_.setServer(MQTT_SERVER, 1883);
    logger_.log("Attempting MQTT connection...");
    if (mqtt_client_.connect(HOSTNAME "-" MQTT_USER, MQTT_USER, MQTT_PASSWORD)) {
        logger_.log("MQTT connected");
        mqtt_client_.subscribe(MQTT_COMMAND_TOPIC);
    } else {
        snprintf(buf, sizeof(buf), "MQTT failed rc=%d will try again in 5 seconds", mqtt_client_.state());
        logger_.log(buf);
    }
}

void MQTTTask::run() {
    connectWifi();
    connectMQTT();

    while(1) {
        long now = millis();
        if (!mqtt_client_.connected() && (now - mqtt_last_connect_time_) > 5000) {
            logger_.log("Reconnecting MQTT");
            mqtt_last_connect_time_ = now;
            connectMQTT();
        }
        mqtt_client_.loop();
        delay(1);
    }
}
#endif
