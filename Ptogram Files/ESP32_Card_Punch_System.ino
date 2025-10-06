#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include "time.h"


// ===== WiFi Config =====
const char* ssid = "OPPO A17k";
const char* password = "mbkenovai";

// ===== Telegram Config =====
String BOT_TOKEN = "8150529775:AAGOspqK70VKgVoWbEm8uWiLKEZg5huORMo";
String ADMIN_CHAT_ID = "6108794668";
 
// ===== RFID =====
#define SS_PIN 5
#define RST_PIN 27
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ===== LED & Buzzer =====
#define GREEN_LED 14
#define RED_LED 12
#define BUZZER 13

// ===== LCD =====
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== Debounce =====
const unsigned long MIN_INTERVAL_MS = 5000;

// ===== User struct =====
struct User {
  String uid;
  String name;
  String motherChatId;
};

// ==== Users List ====
User users[] = {
  {"BB7C1005", "Sultanul Arafin", "6108794668"},
  {"92BE4B00", "ESHA", "1888460726"},
  {"7D4C7D21", "ARAF", "6108794668"}
};
const int USER_COUNT = sizeof(users) / sizeof(User);
unsigned long lastPunchTime[USER_COUNT];

// ===== NTP Config =====
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 6 * 3600;
const int daylightOffset_sec = 0;

// ===== Google Form Config =====
String FORM_URL = "https://docs.google.com/forms/d/e/1FAIpQLSdn6UNLD3tDii8YuaXzJ83_p_fyI0OqNwYt1FHTOmnApwHzXg/formResponse";
String NAME_FIELD   = "entry.425856397";
String UID_FIELD   = "entry.226797185";
String STATUS_FIELD = "entry.660540984";
String TIME_FIELD   = "entry.1081983871";

// === Custom WiFi Icon ===
byte wifi_icon[8] = {
  B00000,
  B00100,
  B00101,
  B00111,
  B10111,
  B10111,
  B11111,
  B11111
};

// ===== WiFi status timers =====
unsigned long lastWiFiCheckMs = 0;
const unsigned long WIFI_CHECK_INTERVAL = 1000;       // প্রতিবার 1s এ icon update
unsigned long lastReconnectAttemptMs = 0;
const unsigned long RECONNECT_INTERVAL = 10000;
       // disconnected হলে 10s পর reconnect চেষ্টা

// ===== Helpers =====
String normalizeUID(String raw) {
  String s = "";
  for (unsigned int i = 0; i < raw.length(); i++) {
    char c = raw[i];
    if (c == ' ' || c == ':' ) continue;
    s += (char)toupper(c);
  }
  return s;
}

int findUserIndex(String normUid) {
  for (int i = 0; i < USER_COUNT; i++) {
    if (normUid == users[i].uid) return i;
  }
  return -1;
}

String getTimeStamp() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    Serial.println("⚠️ Failed to obtain time, using millis()");
    return String(millis()/1000) + "s";
  }
  char timeStr[30];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStr);
}

// ===== LCD Display (no full clear; icon handled separately) =====
void showOnLCD(String line1, String line2) {
  lcd.setCursor(0,0);
  for (int i=0;i<16;i++) lcd.print(" ");   // wipe first line
  lcd.setCursor(0,0);
  lcd.print(line1.substring(0,16));

  lcd.setCursor(0,1);
  for (int i=0;i<15;i++) lcd.print(" ");   // wipe second line except last col
  lcd.setCursor(0,1);
  lcd.print(line2.substring(0,15));

  updateWiFiIcon();
}

// ===== Update WiFi Icon (call frequently) =====
void updateWiFiIcon() {
  lcd.setCursor(15,1);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.write(0);  // custom icon
  } else {
    lcd.print("X");
  }
}

// ===== Telegram message =====
bool sendTelegramMessage(const String &chatId, const String &text) {
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClientSecure client;
  client.setInsecure(); // SSL সার্টিফিকেট চেক বাদ

  HTTPClient https;
  String url = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendMessage";

  if (!https.begin(client, url)) {
    Serial.println("❌ HTTPS begin failed!");
    return false;
  }

  https.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String postData = "chat_id=" + chatId + "&text=" + text;

  int code = https.POST(postData);
  Serial.printf("Telegram HTTP code: %d\n", code);

  if (code > 0) {
    Serial.println(https.getString());
  } else {
    Serial.printf("❌ POST error: %s\n", https.errorToString(code).c_str());
  }

  https.end();
  return (code >= 200 && code < 300);
}

// ===== Google Form logging =====
void logToGoogleForm(String name, String uid, String motherChatId, String timeStamp, String status) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi not connected, skipping Google Form log.");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  if (!https.begin(client, FORM_URL)) {
    Serial.println("❌ HTTPS begin failed!");
    return;
  }
  https.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = NAME_FIELD + "=" + name +
                    "&" + UID_FIELD + "=" + uid +
                    "&" + STATUS_FIELD + "=" + status +
                    "&" + TIME_FIELD + "=" + timeStamp;

  Serial.println("GoogleForm POST: " + postData);
  int code = https.POST(postData);
  Serial.printf("Google Form HTTP code: %d\n", code);
  if (code > 0) Serial.println(https.getString());
  else Serial.printf("❌ POST error: %s\n", https.errorToString(code).c_str());
  https.end();
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(500);
  SPI.begin();
  mfrc522.PCD_Init();
  for (int i=0;i<USER_COUNT;i++) lastPunchTime[i]=0;
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, wifi_icon);
  showOnLCD("RFID Attendance", "System Starting");
  delay(2000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int tries=0;
  while (WiFi.status() != WL_CONNECTED && tries < 60) { delay(500); Serial.print("."); tries++; }
  if (WiFi.status()==WL_CONNECTED) {
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    showOnLCD("WiFi Connected", WiFi.localIP().toString());
    delay(2000);   // IP দেখানোর পর অপেক্ষা
    showOnLCD("Punch Card", "Ready...");   // NEW
  } else {
    Serial.println("\nWiFi not connected (offline mode)");
    showOnLCD("WiFi Failed", "Offline Mode");
  }

  lastWiFiCheckMs = millis();
  lastReconnectAttemptMs = millis();
}

// ===== Loop =====
void loop() {
  unsigned long now = millis();

  if (now - lastWiFiCheckMs >= WIFI_CHECK_INTERVAL) {
    lastWiFiCheckMs = now;
    updateWiFiIcon();
  }

  if (WiFi.status() != WL_CONNECTED && (now - lastReconnectAttemptMs >= RECONNECT_INTERVAL)) {
    Serial.println("Attempting WiFi reconnect...");
    lastReconnectAttemptMs = now;
    WiFi.disconnect();
    WiFi.begin(ssid, password);
  }

  if (!mfrc522.PICC_IsNewCardPresent()) { delay(50); return; }
  if (!mfrc522.PICC_ReadCardSerial()) { delay(50); return; }

  String rawUid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (i > 0) rawUid += " ";
    byte b = mfrc522.uid.uidByte[i];
    if (b < 0x10) rawUid += "0";
    rawUid += String(b, HEX);
  }
  rawUid.toUpperCase();
  Serial.println("Raw UID: " + rawUid);

  String normUid = normalizeUID(rawUid);
  Serial.println("Normalized UID: " + normUid);

  String timeStamp = getTimeStamp();
  int idx = findUserIndex(normUid);
  unsigned long nowMs = millis();

  if (idx >= 0) {
    if (nowMs - lastPunchTime[idx] < MIN_INTERVAL_MS) {
      Serial.println("Ignored duplicate punch for " + users[idx].name);
      mfrc522.PICC_HaltA(); mfrc522.PCD_StopCrypto1();
      return;
    }
    lastPunchTime[idx] = nowMs;

    digitalWrite(GREEN_LED, HIGH);
    tone(BUZZER, 1000, 150);

    String msg = "✅ Attendance: " + users[idx].name + " at " + timeStamp;
    Serial.println(msg);

    showOnLCD(users[idx].name, "Present");
    sendTelegramMessage(users[idx].motherChatId, msg);
    logToGoogleForm(users[idx].name, normUid, users[idx].motherChatId, timeStamp, "Present");

    delay(2000);   // তথ্য থাকবে
    digitalWrite(GREEN_LED, LOW);
    showOnLCD("Punch Card", "Ready...");   // আবার Punch Card দেখাবে
  } else {
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 500, 800);

    String msg = "❌ Unauthorized Card (" + normUid + ") at " + timeStamp;
    Serial.println(msg);

    showOnLCD("Access Denied!", normUid);
    sendTelegramMessage(ADMIN_CHAT_ID, msg);
    logToGoogleForm("Unknown", normUid, ADMIN_CHAT_ID, timeStamp, "Denied");

    delay(2000);   // তথ্য থাকবে
    digitalWrite(RED_LED, LOW);
    showOnLCD("Punch Card", "Ready...");   // আবার Punch Card দেখাবে
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(200);
}
