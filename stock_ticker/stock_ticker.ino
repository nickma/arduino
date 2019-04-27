
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>

//#include <WiFiClient.h>

// Wifi credential information
const String kSsid = "Pwn135";
const String kPassword = "my.little.pwnies";

// Yahoo Finance API information
const char* kYahooApi = "/market/get-summary?region=US&lang=en";
const char* kApiHost = "apidojo-yahoo-finance-v1.p.rapidapi.com";
const char* kApiUri = "https://apidojo-yahoo-finance-v1.p.rapidapi.com/market/get-summary?region=US&lang=en";
//const char* kApiUri = "https://canhazip.com";
const uint16_t kApiPort = 443;
const char* kApiKey = "2822998c09msh41e18f03cdf82dep115067jsndc7284e628ad";

// Max document size that we can parse for JSON
const size_t kMaxJsonDoc = 0x4096;

// Main loop pause
const size_t kMainDelay = 10000;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* kYahooApiFingerprint = "01 3E 44 23 62 2D 92 54 6D 75 7B 2C 60 A7 33 A2 FB 60 C1 3E";
/*
const uint8_t kCanHazFingerprint[20] = {
  0x3, 0x11, 0x70, 0xBA, 0x7D, 0x5D, 0xD1, 0xE0, 0x3C, 0x80, 
  0xA8, 0x5B, 0xB2, 0x8F, 0x28, 0x57, 0x67, 0xA1, 0xFC, 0x4A
};
*/
const char* kCanHazFingerprint = "39 11 70 BA 7D 5D D1 E0 3C 80 A8 5B B2 8F 28 57 67 A1 FC 4A";

// TODO: A logo?
static const unsigned char kOsqueryLogo [] = {
  0x7F, 0x00, 0x80, 0x00, 0x3F, 0x80, 0xC0, 0x01, 0x1F, 0xC0, 0xE0, 0x03, 0x0F, 0xE0, 0xF0, 0x07,
  0x07, 0xF0, 0xF8, 0x0F, 0x03, 0xF8, 0xFC, 0x1F, 0x01, 0xFC, 0xFE, 0x3F, 0x00, 0xFE, 0xFF, 0x7F,
  0x01, 0xFF, 0xFF, 0xFE, 0x03, 0xFC, 0x7F, 0xFC, 0x07, 0xF8, 0x3F, 0xF8, 0x0F, 0xF0, 0x1F, 0xF0,
  0x1F, 0xE0, 0x0F, 0xE0, 0x3F, 0xC0, 0x07, 0xC0, 0x7F, 0x80, 0x03, 0x80, 0xFF, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x01, 0xFF, 0x01, 0xC0, 0x03, 0xFE, 0x03, 0xE0, 0x07, 0xFC, 0x07, 0xF0, 0x0F, 0xF8,
  0x0F, 0xF8, 0x1F, 0xF0, 0x1F, 0xFC, 0x3F, 0xE0, 0x3F, 0xFE, 0x7F, 0xC0, 0x7F, 0xFF, 0xFF, 0x80,
  0xFE, 0xFF, 0x7F, 0x00, 0xFC, 0x7F, 0x3F, 0x80, 0xF8, 0x3F, 0x1F, 0xC0, 0xF0, 0x1F, 0x0F, 0xE0,
  0xE0, 0x0F, 0x07, 0xF0, 0xC0, 0x07, 0x03, 0xF8, 0x80, 0x03, 0x01, 0xFC, 0x00, 0x01, 0x00, 0xFE
};

Adafruit_SSD1306 display = Adafruit_SSD1306();

ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
 
  Serial.println();
  Serial.println("Stock Ticker Board v0.1");
  Serial.print("Connecting to wifi: ");
  Serial.print(kSsid);

  // initialize with the I2C addr 0x3C (for the 128x32)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  auto msg = "Connecting to " + kSsid;
  display.print(msg);
  display.setCursor(0,0);
  display.display();
  delay(500);
  
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(kSsid.c_str(), kPassword.c_str());
  
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(" .");
  }

  Serial.println();
  Serial.print("Connected to ");
  Serial.print(kSsid);
  Serial.print(" with IP address: ");
  Serial.println(WiFi.localIP());
  display.setCursor(0,15);
  display.print("IP: " + WiFi.localIP());
  display.display();
  delay(2000);
}

// Fetches a string response from a remote URI
int getMktSummary(String& resp) {

  std::unique_ptr<BearSSL::WiFiClientSecure>secure_client(new BearSSL::WiFiClientSecure);
  
  //secure_client->setFingerprint(kCanHazFingerprint);
  secure_client->setFingerprint(kYahooApiFingerprint);
  secure_client->setTimeout(3000);

  /*
   * For generic HTTP requests:
     HTTPClient http;
     WiFiClient client;
     Serial.println("Beginning connection to API endpoint");
     auto ret = http.begin(client, kApiUri);
   */
  
  HTTPClient https;
  Serial.println("Beginning connection to API endpoint");
  auto ret = https.begin(*secure_client, kApiUri);

  if (!ret) {
    Serial.println("Failed to connect to URI with " + String(ret));
    return 1;
  }
  
  Serial.println("Sending HTTP GET request");
  auto status = https.GET();
  if (status != HTTP_CODE_OK) {
    Serial.println("HTTPS GET failed with code " + String(status));
    return status;
  }

  resp = https.getString();
  Serial.println("Successfully connected to " + String(kApiHost));
  Serial.println(resp);

  return status;
}
 
void loop() {
  
  Serial.println("Fetching Market Summary");
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Fetching summary...\n");
  display.display();
  delay(2000);
  String summary;
  auto ret = getMktSummary(summary);
  Serial.println(summary);

  auto msg = "ret: " + String(ret);
  display.setCursor(0,15);
  display.print(msg);
  display.display();

  delay(kMainDelay);
}
