#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- НАЛАШТУВАННЯ МЕРЕЖІ ТА ТЕЛЕГРАМ ---
const char* WIFI_SSID = "Xiaomi_3582";         // Назва твого Wi-Fi
const char* WIFI_PASSWORD = "12345678"; // Пароль від Wi-Fi
const char* BOT_TOKEN = "7995017565:AAHgtRC0CJlRMoOU9P14VyovTLcWWkImrCE";         // Токен від BotFather
const char* CHAT_ID = "1687970826";             // Твій Telegram Chat ID

// --- НАЛАШТУВАННЯ ПІНІВ ТА ТАЙМЕРІВ ---
const int PIR_PIN = 18;

bool motionDetected = false;
unsigned long lastMotionTime = 0;
const unsigned long cooldownPeriod = 10000; // 10 секунд тиші після руху

// Перевіряємо Telegram раз на 2 секунди, щоб не завантажувати процесор під час очікування руху
unsigned long lastCheckBotTime = 0;
const unsigned long checkInterval = 2000; 

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Обробка команд із ТВОЇМ текстом
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (chat_id == CHAT_ID) {
      if (text == "/status") {
        Serial.println("[SERIAL] Recieved /status command");

        // Миттєвий зріз стану датчика
        int currentSensor = digitalRead(PIR_PIN);
        String livePresence = (currentSensor == HIGH) ? "🚨 *motion detected*" : "🟢 *no movement detected*";

        String statusMsg = "* system status:*\n";
        statusMsg += " security: *active*\n";
        statusMsg += " powerbank: *active* \n\n";
        statusMsg += "🔍 *check:*\n";
        statusMsg += livePresence;
        
        bot.sendMessage(CHAT_ID, statusMsg, "Markdown");
      }
      else if (text == "/start") {
        Serial.println("[SERIAL] Recieved /start command");

        String welcome = "hello. I'm securitybot. 🛡️\n\n";
        welcome += "Comands:\n";
        welcome += "/status — check system and power bank";
        bot.sendMessage(CHAT_ID, welcome, "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- STARTING ESP32 ---");

  pinMode(PIR_PIN, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client.setInsecure();

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Калібрування PIR-датчика при старті
  Serial.println("Calibrating PIR sensor (15s)...");
  delay(15000);

  bot.sendMessage(CHAT_ID, "🛡️ * security system active!*  /status to check.", "Markdown");
  Serial.println("System ready!");
}

void loop() {
  // 1. МИТТЄВА ПЕРЕВІРКА ДАТЧИКА (Без жодних затримок delay)
  int sensorValue = digitalRead(PIR_PIN);

  if (sensorValue == HIGH) {
    lastMotionTime = millis();

    if (!motionDetected) {
      motionDetected = true;
      Serial.println("[SERIAL] 🚨 MOTION DETECTED! Sending Telegram alert...");

      // Відправляємо тривогу максиматьно швидко
      bot.sendMessage(CHAT_ID, "🚨 *emergency!* motion detected!", "Markdown");
    }
  }

  // Скидання стану після періоду тиші
  if (motionDetected && (millis() - lastMotionTime > cooldownPeriod)) {
    motionDetected = false;
    Serial.println("[SERIAL] 🟢 Quiet again.");
  }

  // 2. Перевірка команд у Telegram
  if (!motionDetected && (millis() - lastCheckBotTime > checkInterval)) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastCheckBotTime = millis();
  }
}