// ESP32-CAM Number Plate Reader with 128x32 LCD Display
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "lcd128_32_io.h"

// ========== PIN CONFIGURATION ==========
#define LCD_SDA 15        // LCD SDA (connected to IO15)
#define LCD_SCL 16        // LCD SCL (connected to IO16)
#define TRIGGER_BTN 2     // Trigger button (connected to IO2)
#define FLASH_LED 4       // Built-in flash LED

// ========== LCD DISPLAY SETUP ==========
lcd lcdDisplay(LCD_SDA, LCD_SCL);

// ========== WIFI & SERVER CONFIGURATION ==========
const char* ssid = "YOUR_WIFI_SSID";         // ← Change this
const char* password = "YOUR_WIFI_PASSWORD";  // ← Change this
String serverName = "www.circuitdigest.cloud";
String serverPath = "/readnumberplate";
const int serverPort = 443;
String apiKey = "YOUR_API_KEY";              // ← Change this

// ========== CAMERA PINS (ESP32-CAM Standard) ==========
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ========== GLOBAL VARIABLES ==========
WiFiClientSecure client;
int uploadCount = 0;
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 500;

// ========== FUNCTION DECLARATIONS ==========
void displayLCD(int line, const char* text);
String extractJsonStringValue(const String& jsonString, const String& key);
int captureAndSendPhoto();

// ========================================================
// SETUP
// ========================================================
void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  // Initialize Serial
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n========================================");
  Serial.println("ESP32-CAM Number Plate Reader with LCD");
  Serial.println("========================================\n");
  
  // Configure GPIO pins
  pinMode(FLASH_LED, OUTPUT);
  pinMode(TRIGGER_BTN, INPUT_PULLUP);
  digitalWrite(FLASH_LED, LOW);
  Serial.println("✓ GPIO pins configured");
  
  // Initialize LCD Display
  Serial.println("Initializing LCD...");
  lcdDisplay.Init();
  delay(500);
  lcdDisplay.Clear();
  Serial.println("✓ LCD initialized");
  
  displayLCD(0, "Initializing");
  displayLCD(1, "System...");
  delay(1000);
  
  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");
  displayLCD(0, "WiFi...");
  displayLCD(1, "Connecting");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n✗ WiFi connection FAILED!");
    displayLCD(0, "WiFi Failed");
    displayLCD(1, "Check Setup");
    delay(5000);
    ESP.restart();
  }
  
  Serial.println("\n✓ WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  displayLCD(0, "WiFi OK");
  displayLCD(1, WiFi.localIP().toString().c_str());
  delay(2000);
  
  // Initialize Camera
  displayLCD(0, "Starting");
  displayLCD(1, "Camera...");
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  
  // Quality settings based on PSRAM
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    Serial.println("✓ PSRAM found - High quality mode");
  } else {
    config.frame_size = FRAMESIZE_SVGA;  // 800x600
    config.jpeg_quality = 12;
    config.fb_count = 1;
    Serial.println("⚠ No PSRAM - Standard quality");
  }
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("✗ Camera init FAILED! Error: 0x%x\n", err);
    displayLCD(0, "Camera Fail");
    displayLCD(1, "Restart ESP");
    while (true) delay(1000);
  }
  Serial.println("✓ Camera initialized");
  
  // Optimize camera sensor settings
  sensor_t* s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);
    s->set_whitebal(s, 1);
    s->set_awb_gain(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_aec2(s, 0);
    s->set_gain_ctrl(s, 1);
    s->set_bpc(s, 0);
    s->set_wpc(s, 1);
    s->set_lenc(s, 1);
    s->set_hmirror(s, 0);
    s->set_vflip(s, 0);
    Serial.println("✓ Camera settings optimized");
  }
  
  // Warm up camera
  for (int i = 0; i < 3; i++) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) esp_camera_fb_return(fb);
    delay(100);
  }
  
  Serial.println("\n========================================");
  Serial.println("SYSTEM READY!");
  Serial.println("Press button on GPIO 2 to capture");
  Serial.println("========================================\n");
  
  displayLCD(0, "READY");
  displayLCD(1, "Press Button");
}

// ========================================================
// MAIN LOOP
// ========================================================
void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠ WiFi disconnected! Reconnecting...");
    displayLCD(0, "WiFi Lost");
    displayLCD(1, "Reconnecting");
    WiFi.reconnect();
    delay(5000);
    return;
  }
  
  // Check button press (LOW = pressed with INPUT_PULLUP)
  if (digitalRead(TRIGGER_BTN) == LOW) {
    unsigned long currentTime = millis();
    
    // Debounce check
    if (currentTime - lastButtonPress > debounceDelay) {
      lastButtonPress = currentTime;
      
      Serial.println("\n>>> Button pressed! Starting capture...");
      
      // Visual feedback
      digitalWrite(FLASH_LED, HIGH);
      delay(100);
      digitalWrite(FLASH_LED, LOW);
      
      // Capture and send photo
      int result = captureAndSendPhoto();
      
      // Handle results
      if (result == -1) {
        Serial.println("✗ Camera capture failed");
        displayLCD(0, "ERROR:");
        displayLCD(1, "Camera Fail");
      } else if (result == -2) {
        Serial.println("✗ Server connection failed");
        displayLCD(0, "ERROR:");
        displayLCD(1, "Server Fail");
      } else {
        Serial.println("✓ Process completed");
      }
      
      // Show result, then return to ready
      delay(3000);
      displayLCD(0, "READY");
      displayLCD(1, "Press Button");
    }
  }
  
  delay(50);
}

// ========================================================
// DISPLAY TEXT ON LCD
// ========================================================
void displayLCD(int line, const char* text) {
  lcdDisplay.Cursor(line, 0);
  
  // Create a padded string to clear the line (17 chars max for 128x32)
  char paddedText[18] = "                 "; // 17 spaces
  int len = strlen(text);
  if (len > 17) len = 17;
  
  // Copy text to padded string
  for (int i = 0; i < len; i++) {
    paddedText[i] = text[i];
  }
  
  lcdDisplay.Display(paddedText);
}

// ========================================================
// EXTRACT JSON VALUE
// ========================================================
String extractJsonStringValue(const String& jsonString, const String& key) {
  int keyIndex = jsonString.indexOf(key);
  if (keyIndex == -1) return "";
  
  int startIndex = jsonString.indexOf(':', keyIndex) + 2;
  int endIndex = jsonString.indexOf('"', startIndex);
  
  if (startIndex == -1 || endIndex == -1) return "";
  return jsonString.substring(startIndex, endIndex);
}

// ========================================================
// CAPTURE AND SEND PHOTO
// ========================================================
int captureAndSendPhoto() {
  camera_fb_t* fb = NULL;
  
  // Turn on flash
  digitalWrite(FLASH_LED, HIGH);
  displayLCD(0, "Capturing");
  displayLCD(1, "Image...");
  delay(200);
  
  // Capture image
  fb = esp_camera_fb_get();
  digitalWrite(FLASH_LED, LOW);
  
  if (!fb) {
    Serial.println("✗ Camera capture failed!");
    return -1;
  }
  
  Serial.printf("✓ Image captured: %d bytes\n", fb->len);
  displayLCD(0, "Captured");
  displayLCD(1, "Sending...");
  delay(500);
  
  // Connect to server
  Serial.println("Connecting to: " + serverName);
  client.setInsecure();
  
  if (!client.connect(serverName.c_str(), serverPort)) {
    Serial.println("✗ Server connection failed!");
    esp_camera_fb_return(fb);
    return -2;
  }
  
  Serial.println("✓ Connected to server");
  displayLCD(0, "Uploading");
  displayLCD(1, "Wait...");
  
  // Prepare multipart form data
  uploadCount++;
  String filename = apiKey + "_" + String(uploadCount) + ".jpeg";
  String boundary = "----WebKitFormBoundary";
  
  String head = "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"imageFile\"; filename=\"" + filename + "\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";
  
  String tail = "\r\n--" + boundary + "--\r\n";
  uint32_t totalLen = head.length() + fb->len + tail.length();
  
  // Send HTTP POST request
  client.println("POST " + serverPath + " HTTP/1.1");
  client.println("Host: " + serverName);
  client.println("Content-Length: " + String(totalLen));
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.println("Authorization: " + apiKey);
  client.println("Connection: close");
  client.println();
  client.print(head);
  
  // Send image data in chunks
  uint8_t* fbBuf = fb->buf;
  size_t fbLen = fb->len;
  const size_t chunkSize = 1024;
  
  Serial.print("Uploading");
  for (size_t n = 0; n < fbLen; n += chunkSize) {
    size_t remaining = fbLen - n;
    size_t toSend = (remaining > chunkSize) ? chunkSize : remaining;
    client.write(fbBuf + n, toSend);
    if (n % 10240 == 0) Serial.print(".");
  }
  Serial.println(" Done!");
  
  client.print(tail);
  
  displayLCD(0, "Processing");
  displayLCD(1, "Wait...");
  
  // Wait for server response
  String response = "";
  unsigned long timeout = millis();
  
  while (client.connected() && millis() - timeout < 15000) {
    if (client.available()) {
      char c = client.read();
      response += c;
    }
  }
  
  client.stop();
  esp_camera_fb_return(fb);
  
  // Parse response
  Serial.println("\n--- Server Response ---");
  Serial.println(response);
  Serial.println("--- End Response ---\n");
  
  String plateNumber = extractJsonStringValue(response, "\"number_plate\"");
  String imageLink = extractJsonStringValue(response, "\"view_image\"");
  
  if (plateNumber.length() > 0) {
    Serial.println("✓ Plate detected: " + plateNumber);
    displayLCD(0, "PLATE:");
    displayLCD(1, plateNumber.c_str());
    
    if (imageLink.length() > 0) {
      Serial.println("Image URL: " + imageLink);
    }
  } else {
    Serial.println("⚠ No plate detected");
    displayLCD(0, "No Plate");
    displayLCD(1, "Detected");
  }
  
  return 0;
}