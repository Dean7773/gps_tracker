#include "EncButton.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <FS.h>
#include <SD.h>
#include <HardwareSerial.h>
#include <math.h>

#include "config.h"

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================================

// Инициализация OLED, GPS
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

// Инициализация кнопок
VirtButton up;
VirtButton ok;
VirtButton down;

// ============================================================================
// СТРУКТУРЫ И ПЕРЕЧИСЛЕНИЯ
// ============================================================================

// Структура для хранения WiFi credentials
struct WiFiCred {
  char ssid[WIFI_CRED_MAXLEN];
  char pass[WIFI_CRED_MAXLEN];
};

// Перечисление для уровней меню
enum MenuLevel { MAIN, WIFI, TRACK, SDCARD, GPS };
MenuLevel menuLevel = MAIN;

// Переменные для указателей меню
uint8_t mainPointer = 0; // 0-GPS, 1-WiFi, 2-Track, 3-SD Card
uint8_t wifiPointer = 0; // 0-Connect/Connected, 1-Auth, 2-Send track, 3-Send all tracks
uint8_t sdPointer = 0;   // 0-Files, 1-Format, 2-Test
uint32_t elapsedTime = 0; // Оставляем для возможного использования в будущем
bool flag_track = false;
bool wifiConnected = false;

// WiFi меню
String ssidList[10];
int networksFound = 0;

// Символы для ввода пароля
const char symbols[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()-_=+[]{};:,.<>/?";
const int symbolsCount = sizeof(symbols) - 1;

// Глобальные переменные для авторизации
String jwtToken = "";
String authUser = "";
String authPass = "";

// Переменные для отображения
static uint32_t lastDisplayUpdate = 0;
const uint32_t DISPLAY_UPDATE_INTERVAL = 500; // Обновление каждые 500мс

// Добавляем флаг для отрисовки
bool needRedraw = true;

// GPS данные для собственной обработки
struct GPSData {
  float latitude = 0;
  float longitude = 0;
  float speed = 0;
  uint8_t hours = 0;
  uint8_t minutes = 0;
  uint8_t seconds = 0;
  uint8_t satellites = 0;
  bool hasFix = false;
} gpsData;

// Переменные для записи трека
String currentTrackFile = "";
uint32_t trackPoints = 0;
uint32_t lastTrackWrite = 0;
uint32_t trackStartTime = 0; // Время начала записи трека
bool isRecording = false; // Флаг активной записи трека

// Переменные для фильтрации точек
float lastLat = 0.0, lastLon = 0.0;
float prevLat = 0.0, prevLon = 0.0; // Предыдущая точка для расчета угла
bool hasLastPoint = false;
bool hasPrevPoint = false; // Есть ли предыдущая точка для расчета угла


// --- Прототипы ---
int selectWiFiNetwork();
void inputPassword(char* password, int maxLen);
void connectToWiFi(const char* ssid, const char* password);
void disconnectWiFi();
void drawMainMenu();
void drawWiFiMenu();
void drawTrackMenu();
void drawSDCardMenu();
void inputString(char* buffer, int maxLen, const char* label, bool hide = false);
bool authorizeOnServer(const char* login, const char* pass, String& token, String& message);
void saveWiFiCredential(const char* ssid, const char* pass);
void loadWiFiCredentials(WiFiCred* creds);
void viewSDFiles();
void formatSDCard();
void testSDCard();
void deleteWiFiCredential(const char* ssid);
bool getSavedWiFiPassword(const char* ssid, char* password);
void configureGPS();
void readUBXData();
void startTrackRecording();
void stopTrackRecording();
void writeTrackPoint();
void sendTrackToServer(const char* filename);


void setup() {
  // Настройка аналогового входа для кнопок
  analogReadResolution(12); // 12-битное разрешение (0-4095)
  
  // Настройка задержки для hasClicks() - можно изменить по желанию
  ok.setClickTimeout(200);    // 400мс для OK (средне)

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("Location tracker");
  display.display();
  delay(500);
  
  // Инициализация GPS через HardwareSerial для ESP32
  gpsSerial.begin(115200, SERIAL_8N1, GPS_RX, GPS_TX);
  delay(500);

  display.clearDisplay();
  display.display();
  drawMainMenu();
}

void loop() {

  // Читаем аналоговое значение кнопок
  int analog = analogRead(ANALOG_PIN);
  
  // Определяем нажатые кнопки
  bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
  bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
  bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
  
  // Обновляем состояния кнопок
  up.tick(upPressed);
  ok.tick(okPressed);
  down.tick(downPressed);
  
  // Дополнительная обработка кнопок для лучшей реакции
  if (flag_track) {
    up.tick(upPressed);
    ok.tick(okPressed);
    down.tick(downPressed);
  }


  // MAIN MENU
  if (menuLevel == MAIN) {
    if (up.click()) { mainPointer = (mainPointer + 3) % 4; needRedraw = true; }
    if (down.click()) { mainPointer = (mainPointer + 1) % 4; needRedraw = true; }
    if (ok.click()) {
      switch (mainPointer) {
        case 0: 
          menuLevel = GPS; 
          needRedraw = true;
          ok.reset(); 
          break;
        case 1: 
          menuLevel = WIFI; 
          wifiPointer = 0; 
          needRedraw = true; 
          ok.reset();
          break;
        case 2: 
          menuLevel = TRACK; 
          needRedraw = true; 
          ok.reset();
          break;
        case 3: 
          menuLevel = SDCARD; 
          sdPointer = 0; 
          needRedraw = true; 
          ok.reset();
          break;
      }
    }
    static uint32_t uiTimer = 0;
    if (millis() - uiTimer > 5000) { uiTimer = millis(); needRedraw = true; }
  }

  // WIFI SUBMENU
  if (menuLevel == WIFI) {
    int maxPointer = wifiConnected ? 4 : 1;
    if (up.click()) { wifiPointer = (wifiPointer + maxPointer - 1) % maxPointer; needRedraw = true; }
    if (down.click()) { wifiPointer = (wifiPointer + 1) % maxPointer; needRedraw = true; }
    if (ok.click()) {
      if (!wifiConnected) {
        int net = selectWiFiNetwork();
        if (net >= 0) {
          char password[33] = "";
          inputPassword(password, 33);
          connectToWiFi(ssidList[net].c_str(), password);
        }
        needRedraw = true;
      } else {
        switch (wifiPointer) {
          case 0: { // Connected
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("    Connected!");
            display.println("=====================");
            display.print("  IP: ");
            display.println(WiFi.localIP());
            display.println("");
            display.println(" Push to disconnect");
            display.println("");
            display.println("=====================");
            display.println("  Hold UP to return");
            display.display();
            while (true) {
              int analog = analogRead(ANALOG_PIN);
              bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
              bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
              
              up.tick(upPressed);
              ok.tick(okPressed);
              
              if (ok.click()) { disconnectWiFi(); needRedraw = true; break; }
              if (up.holding()) { needRedraw = true; break; }
              delay(1);
            }
            break;
          }
          case 1: { // Auth
            char login[33] = "";
            inputString(login, 33, "Login:");
            char pass[33] = "";
            inputString(pass, 33, "Password:", true);
            String token, msg;
            bool ok_auth = authorizeOnServer(login, pass, token, msg);
            display.clearDisplay();
            display.setCursor(0, 0);
            if (ok_auth) {
              display.println("Auth OK!");
              jwtToken = token;
              authUser = login;
              authPass = pass;
            } else {
              display.println("Auth failed!");
            }
            display.setCursor(0, 10);
            display.println(msg);
            display.setCursor(0, 24);
            display.println("Push to return");
            display.display();
            while (!ok.click()) { 
              int analog = analogRead(ANALOG_PIN);
              bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
              bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
              bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
              
              up.tick(upPressed);
              ok.tick(okPressed);
              down.tick(downPressed);
              delay(1); 
            }
            needRedraw = true;
            break;
          }
          case 2: { // Send track
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Select track to send");
            display.println("Scanning SD...");
            display.display();
            
            // Сканируем SD карту на наличие треков
            if (SD.begin(SD_CS_PIN)) {
              File root = SD.open("/");
              if (root) {
                File entry = root.openNextFile();
                char trackFiles[20][32]; // Максимум 20 треков, каждый до 31 символа
                int trackCount = 0;
                
                // Собираем список треков
                while (entry && trackCount < 20) {
                  if (!entry.isDirectory()) {
                    strncpy(&trackFiles[trackCount][0], entry.name(), 31);
                    trackFiles[trackCount][31] = '\0'; // Гарантируем завершение строки
                    trackCount++;
                  }
                  entry.close();
                  entry = root.openNextFile();
                }
                root.close();
                
                if (trackCount > 0) {
                  // Показываем список треков для выбора
                  int selectedTrack = 0;
                  int scroll = 0;
                  const int visibleRows = 6; // Увеличено для экрана 64x128
                  
                  while (true) {
                    int analog = analogRead(ANALOG_PIN);
                    bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
                    bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
                    bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
                    
                    up.tick(upPressed);
                    ok.tick(okPressed);
                    down.tick(downPressed);
                    
                    if (up.click()) selectedTrack = (selectedTrack + trackCount - 1) % trackCount;
                    if (down.click()) selectedTrack = (selectedTrack + 1) % trackCount;
                    
                    // Скроллинг
                    if (selectedTrack < scroll) scroll = selectedTrack;
                    if (selectedTrack >= scroll + visibleRows) scroll = selectedTrack - visibleRows + 1;
                    
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.println("Select track:");
                    
                    for (int i = 0; i < visibleRows; i++) {
                      int idx = scroll + i;
                      if (idx >= trackCount) break;
                      display.setCursor(0, 9 + i * 8);
                      if (idx == selectedTrack) display.print("> ");
                      else display.print("  ");
                      
                      // Обрезаем длинные имена
                      char shortName[20];
                      strncpy(shortName, trackFiles[idx], 18);
                      shortName[18] = '\0';
                      if (strlen(trackFiles[idx]) > 18) {
                        strcpy(shortName + 15, "...");
                      }
                      display.println(shortName);
                    }

                    display.display();
                    
                    if (ok.click()) {
                      // Отправляем выбранный трек
                      sendTrackToServer(trackFiles[selectedTrack]);
                      break;
                    }
                    if (up.holding()) break;
                    delay(1);
                  }
                }
              }
            }
            needRedraw = true;
            break;
          }
          case 3: { // Send all tracks
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Send all tracks");
            display.println("Scanning SD...");
            display.display();
            
            // Сканируем SD карту на наличие треков
            if (SD.begin(SD_CS_PIN)) {
              File root = SD.open("/");
              if (root) {
                File entry = root.openNextFile();
                int trackCount = 0;
                while (entry) {
                  if (!entry.isDirectory()) {
                    trackCount++;
                  }
                  entry.close();
                  entry = root.openNextFile();
                }
                root.close();
                
                if (trackCount > 0) {
                  display.clearDisplay();
                  display.setCursor(0, 0);
                  display.print("Found ");
                  display.print(trackCount);
                  display.println(" tracks");
                  display.println("Push to send all");
                  display.println("Hold to cancel");
                  display.display();
                  
                  bool sendTracks = false;
                  while (true) {
                    int analog = analogRead(ANALOG_PIN);
                    bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
                    bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
                    bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
                    
                    up.tick(upPressed);
                    ok.tick(okPressed);
                    down.tick(downPressed);
                    
                    if (ok.click()) { sendTracks = true; break; }
                    if (up.holding()) break;
                    delay(1);
                  }
                  
                  if (sendTracks) {
                    // Отправляем все треки
                    root = SD.open("/");
                    if (root) {
                      entry = root.openNextFile();
                      while (entry) {
                        if (!entry.isDirectory()) {
                          sendTrackToServer(entry.name());
                        }
                        entry.close();
                        entry = root.openNextFile();
                      }
                      root.close();
                    }
                  }
                } else {
                  display.clearDisplay();
                  display.setCursor(0, 0);
                  display.println("No tracks found!");
                  display.println("Push to return");
                  display.display();
                  while (!ok.click()) { 
                    int analog = analogRead(ANALOG_PIN);
                    bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
                    bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
                    bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
                    
                    up.tick(upPressed);
                    ok.tick(okPressed);
                    down.tick(downPressed);
                    delay(1); 
                  }
                }
              }
            }
            needRedraw = true;
            break;
          }
        }
      }
    }
    if (up.holding()) { menuLevel = MAIN; needRedraw = true; }
  }

  // TRACK MENU
  if (menuLevel == TRACK) {
    // Приоритетная обработка кнопок
    if (ok.click()) { 
      if (!flag_track) {
        // Начинаем запись трека
        flag_track = true;
        startTrackRecording();
      } else {
        // Останавливаем запись трека - немедленно
        flag_track = false;
        stopTrackRecording();
      }
      needRedraw = true; 
    }
    if (up.holding()) { menuLevel = MAIN; needRedraw = true; }
    
    // Запись GPS точек во время трека - 2 раза в секунду для лучшей реакции
    if (isRecording && millis() - lastTrackWrite > 500) { // 1000ms / 2 = 500ms
      lastTrackWrite = millis();
      writeTrackPoint();
      needRedraw = true;
    }
    
    // Дополнительная обработка нажатий во время записи
    if (isRecording) {
      int analog = analogRead(ANALOG_PIN);
      bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
      bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
      bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
      
      up.tick(upPressed);
      ok.tick(okPressed);
      down.tick(downPressed);
    }
  }

  // GPS MENU
  if (menuLevel == GPS) {
    // Отладка входа в GPS меню (только один раз)
    static bool gpsEntered = false;
    if (!gpsEntered) {
      gpsEntered = true;
      needRedraw = true;

    }
    
    if (ok.click() || up.holding()) { 
      menuLevel = MAIN; 
      needRedraw = true; 
      gpsEntered = false;
    }
    
    // Безопасное чтение GPS данных
    static uint32_t lastGPSUpdate = 0;
    if (millis() - lastGPSUpdate > 2000) { // Обновляем каждые 2 секунды
      lastGPSUpdate = millis();
      
      // Читаем GPS данные
      while (gpsSerial.available()) {
        char c = gpsSerial.read();
        gps.encode(c);
      }
      needRedraw = true;
    }
  }

  // SDCARD MENU
  if (menuLevel == SDCARD) {
    if (up.click()) { sdPointer = (sdPointer + 2) % 3; needRedraw = true; }
    if (down.click()) { sdPointer = (sdPointer + 1) % 3; needRedraw = true; }
    if (ok.click()) {
      switch (sdPointer) {
        case 0: viewSDFiles(); needRedraw = true; break;
        case 1: formatSDCard(); needRedraw = true; break;
        case 2: testSDCard(); needRedraw = true; break;
      }
    }
    if (up.holding()) { menuLevel = MAIN; needRedraw = true; }
  }

  // ОТРИСОВКА ТОЛЬКО ЕСЛИ НУЖНО
  if (needRedraw) {
    switch (menuLevel) {
      case MAIN: drawMainMenu(); break;
      case WIFI: drawWiFiMenu(); break;
      case TRACK: drawTrackMenu(); break;
      case SDCARD: drawSDCardMenu(); break;
      case GPS: drawGPSMenu(); break;
    }
    needRedraw = false;
  }
}
