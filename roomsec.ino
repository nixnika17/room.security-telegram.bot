#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- NETWORK AND TELEGRAM SETTINGS ---
const char* WIFI_SSID = "YOUR_WIFI_NAME";     // Your Wi-Fi network name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // Your Wi-Fi password
const char* BOT_TOKEN = "TOKEN";         // Token from BotFather
const char* CHAT_ID = "CHAT_ID";             // Your Telegram Chat ID

// --- PIN AND TIMER SETTINGS ---
const int PIR_PIN = 18;

bool motionDetected = false;
unsigned long lastMotionTime = 0;
const unsigned long cooldownPeriod = 10000; // 10 seconds of silence after motion detection

// Check Telegram every 2 seconds to avoid overloading the CPU while waiting for motion
unsigned long lastCheckBotTime = 0;
const unsigned long checkInterval = 2000; 

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Processing incoming Telegram commands
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (chat_id == CHAT_ID) {
      if (text == "/status") {
        Serial.println("[SERIAL] Received /status command");

        // Instant snapshot of the sensor status
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
        Serial.println("[SERIAL] Received /start command");

        String welcome = "hello. I'm securitybot. 🛡️\n created be Nix.\n social media: \n telegram chanel: https://t.me/nixinworld \n github: https://github.com/nixnika17
\n youtube: www.youtube.com/@nixvaslgen\n Want to message me directly? You can find all my personal accounts (Telegram, Gmail, Insta, TikTok) in my bio on GitHub or Telegram channel. Good Luck! \n\n";
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
