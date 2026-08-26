#include <Arduino.h>
#include <IO7F32.h>
#include <DHTesp.h>
#include <TFT_eSPI.h>

String user_html = "" ;

char*               ssid_pfix = (char*)"IOTThermo";
unsigned long       lastPublishMillis = -pubInterval;
const int           DHT22_PIN = 17;

DHTesp              dht;
int                 interval = 2000;
unsigned long       lastDHTReadMillis = 0;
float               humidity = 0;
float               temperature = 0;
char                t_buffer[10];
char                h_buffer[10];

TFT_eSPI            tft = TFT_eSPI();

const int           pulseA = 44;
const int           pulseB = 43;
volatile int        lastEncoded = 0;
volatile int        encoderValue = 0;
volatile long       lastChanged = 0;
int                 target = 0;
int                 oldTarget = 0;
char                target_buf[20];

void readDHT22() {
    unsigned long currentMillis = millis();

    if(currentMillis - lastDHTReadMillis >= interval) {
        lastDHTReadMillis = currentMillis;

        humidity = dht.getHumidity();              // Read humidity (percent)
        temperature = dht.getTemperature();             // Read temperature as Fahrenheit
    }
}

IRAM_ATTR void handleRotary() {
    // Never put any long instruction
    int MSB = digitalRead(pulseA); //MSB = most significant bit
    int LSB = digitalRead(pulseB); //LSB = least significant bit

    int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
    int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;
    lastEncoded = encoded; //store this value for next time
    encoderValue = encoderValue > 255 ? 255 : (encoderValue < 0 ? 0 : encoderValue);
    lastChanged = millis();
}

void publishData() {
    StaticJsonDocument<512> root;
    JsonObject data = root.createNestedObject("d");

    sprintf(t_buffer, "%.1f", temperature);
    sprintf(h_buffer, "%.1f", humidity);
    sprintf(target_buf, "%d", target);
    data["humidity"] = h_buffer;
    data["temperature"] = t_buffer;
    data["target"] = target_buf;

    serializeJson(root, msgBuffer);
    client.publish(evtTopic, msgBuffer);
}

void handleUserCommand(char* topic, JsonDocument* root) {
    JsonObject d = (*root)["d"];
}

void setup() {
    Serial.begin(115200);
    dht.setup(DHT22_PIN, DHTesp::DHT22);

    initDevice();
    JsonObject meta = cfg["meta"];
    pubInterval = meta.containsKey("pubInterval") ? meta["pubInterval"] : 0;
    lastPublishMillis = -pubInterval;

    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    // main setup
    Serial.printf("\nIP address : ");
    Serial.println(WiFi.localIP());

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Thermostat", 80, 20, 4);
    tft.drawString("Temp : ", 30, 60, 2);
    tft.drawString("Set  : ", 30, 90, 2);
    pinMode(pulseA, INPUT_PULLUP);
    pinMode(pulseB, INPUT_PULLUP);
    attachInterrupt(pulseA, handleRotary, CHANGE);
    attachInterrupt(pulseB, handleRotary, CHANGE);

    userCommand = handleUserCommand;
    set_iot_server();
    iot_connect();
}

void loop() {
    if (!client.connected()) {
        iot_connect();
    }
    target = map(encoderValue, 0, 255, 0, 60);
    client.loop();
    if (((pubInterval != 0) && (millis() - lastPublishMillis > pubInterval))
        || ((target != oldTarget) && (millis() - lastChanged > 200))) {
        oldTarget = target;
        readDHT22();
        sprintf(t_buffer, "%.1f", temperature);
        sprintf(target_buf, "%d", target);
        tft.fillRect(90, 60, 90, 50, TFT_BLACK);
        tft.drawString(t_buffer, 90, 60, 2);
        tft.drawString(target_buf, 90, 90, 2);

        publishData();
        lastPublishMillis = millis();
    }
}