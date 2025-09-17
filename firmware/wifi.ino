  // Удаление WiFi-сети из EEPROM
  void deleteWiFiCredential(const char* ssid) {
    WiFiCred creds[WIFI_CRED_COUNT];
    loadWiFiCredentials(creds);
    int found = -1;
    for (int i = 0; i < WIFI_CRED_COUNT; i++) {
      if (strncmp(creds[i].ssid, ssid, WIFI_CRED_MAXLEN) == 0) {
        found = i;
        break;
      }
    }
    if (found >= 0) {
      for (int i = found; i < WIFI_CRED_COUNT - 1; i++) {
        creds[i] = creds[i + 1];
      }
      memset(&creds[WIFI_CRED_COUNT - 1], 0, sizeof(WiFiCred));
      EEPROM.begin(EEPROM_SIZE);
      for (int i = 0; i < WIFI_CRED_COUNT; i++) {
        for (int j = 0; j < WIFI_CRED_MAXLEN; j++) {
          EEPROM.write(i * WIFI_CRED_MAXLEN * 2 + j, creds[i].ssid[j]);
          EEPROM.write(i * WIFI_CRED_MAXLEN * 2 + WIFI_CRED_MAXLEN + j, creds[i].pass[j]);
        }
      }
      EEPROM.commit();
    }
  }
  
  // Поиск сохранённого пароля по SSID
  bool getSavedWiFiPassword(const char* ssid, char* password) {
    WiFiCred creds[WIFI_CRED_COUNT];
    loadWiFiCredentials(creds);
    for (int i = 0; i < WIFI_CRED_COUNT; i++) {
      if (strncmp(creds[i].ssid, ssid, WIFI_CRED_MAXLEN) == 0) {
        strncpy(password, creds[i].pass, WIFI_CRED_MAXLEN);
        return true;
      }
    }
    return false;
  }
  
  // Модификация selectWiFiNetwork
  int selectWiFiNetwork() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Scanning...");
    display.display();
  
    networksFound = WiFi.scanNetworks();
    if (networksFound == 0) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("No networks!");
      display.display();
      delay(2000);
      return -1;
    }
  
    for (int i = 0; i < networksFound && i < 10; i++) {
      ssidList[i] = WiFi.SSID(i);
    }
    
    // ВАЖНО: Переключаем WiFi в режим STATION после сканирования
    WiFi.mode(WIFI_STA);
    delay(100); // Небольшая задержка для стабилизации
  
    WiFiCred savedCreds[WIFI_CRED_COUNT];
    loadWiFiCredentials(savedCreds);
    int savedCount = 0;
    for (int i = 0; i < WIFI_CRED_COUNT; i++) {
      if (savedCreds[i].ssid[0] != 0) savedCount++;
    }
  
    int pointer = 0;
    int scroll = 0;
    const int visibleRows = 6; // Увеличено для экрана 64x128
    int totalOptions = networksFound + 1; // только доступные сети + Delete network
  
    while (true) {
      int analog = analogRead(ANALOG_PIN);
      bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
      bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
      bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
      
      up.tick(upPressed);
      ok.tick(okPressed);
      down.tick(downPressed);
  
      if (up.click()) pointer = (pointer + totalOptions - 1) % totalOptions;
      if (down.click()) pointer = (pointer + 1) % totalOptions;
  
      // Скроллинг
      if (pointer < scroll) scroll = pointer;
      if (pointer >= scroll + visibleRows) scroll = pointer - visibleRows + 1;
  
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Select WiFi:");
      int row = 0;
      for (int i = scroll; i < scroll + visibleRows && row < visibleRows; i++) {
        display.setCursor(0, 9 + row * 8);
        if (i == pointer) display.print("> ");
        else display.print("  ");
        if (i < networksFound) {
          display.println(ssidList[i]);
        } else if (i == totalOptions - 1) {
          display.println("Delete network");
        }
        row++;
      }
      display.display();
      if (ok.click()) {
        ok.reset(); 
        if (pointer < networksFound) {
          // --- Подключение к выбранной сети ---
          char ssid[WIFI_CRED_MAXLEN];
          strncpy(ssid, ssidList[pointer].c_str(), WIFI_CRED_MAXLEN);
          char password[WIFI_CRED_MAXLEN] = "";
          if (getSavedWiFiPassword(ssid, password)) {
            // Есть сохранённый пароль — сразу подключаемся
            connectToWiFi(ssid, password);
          } else {
            // Нет пароля — вводим и сохраняем
            inputPassword(password, WIFI_CRED_MAXLEN);
            saveWiFiCredential(ssid, password);
            connectToWiFi(ssid, password);
          }
          return -1;
        } else if (pointer == totalOptions - 1) {
          // --- Удаление сохранённой сети ---
          if (savedCount == 0) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("No saved networks!");
            display.display();
            delay(1500);
            continue;
          }
          int delPointer = 0;
          while (true) {
            int analog = analogRead(ANALOG_PIN);
            bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
            bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
            bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
            
            up.tick(upPressed);
            ok.tick(okPressed);
            down.tick(downPressed);
  
            if (up.click()) delPointer = (delPointer + 1) % savedCount;
            if (down.click()) delPointer = (delPointer + savedCount - 1) % savedCount;
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Delete WiFi:");
            for (int i = 0; i < savedCount; i++) {
              display.setCursor(0, 9 + i * 8);
              if (i == delPointer) display.print("> ");
              else display.print("  ");
              display.println(savedCreds[i].ssid);
            }
            display.display();
            if (ok.hasClicks(2)) {
              deleteWiFiCredential(savedCreds[delPointer].ssid);
              display.clearDisplay();
              display.setCursor(0, 0);
              display.print("Deleted: ");
              display.println(savedCreds[delPointer].ssid);
              display.display();
              delay(1000);
              break;
            }
            if (up.holding()) break;
            delay(1);
          }
        }
      }
      if (up.holding()) return -1; // выход по долгому нажатию
    }
  }
  
  // WiFi: ввод пароля с двойным нажатием для применения
  void inputPassword(char* password, int maxLen) {
    int pos = 0;
    int charIndex = 0;
    bool done = false;
    password[0] = '\0';
  
    while (!done) {
      int analog = analogRead(ANALOG_PIN);
      bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
      bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
      bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
      
      up.tick(upPressed);
      ok.tick(okPressed);
      down.tick(downPressed);
  
      if (up.click()) charIndex = (charIndex + 1) % symbolsCount;
      if (down.click()) charIndex = (charIndex + symbolsCount - 1) % symbolsCount;
      // Обработка двойного нажатия OK (применение пароля)
      if (ok.hasClicks(2)) {
        done = true;
      }
      // Обработка одиночного нажатия OK (добавление символа)
      else if (ok.hasClicks(1) && pos < maxLen - 1) {
        password[pos++] = symbols[charIndex];
        password[pos] = '\0';
      }
      if (ok.holding()) {
        if (pos > 0) {
          pos--;
          password[pos] = '\0';
        }
      }
      if (up.holding() && pos == 0) {
        // Выход без ввода пароля
        password[0] = '\0';
        done = true;
      }
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Password:");
      display.setCursor(0, 10);
      display.print(password);
      display.setCursor(0, 20);
      display.print("Add: ");
      display.print(symbols[charIndex]);
      display.setCursor(80, 24);
      display.print("2xOK");
      display.display();
      delay(1);
    }
  }
  
  // WiFi: подключение
  void connectToWiFi(const char* ssid, const char* password) {
    // Сброс WiFi и принудительный переход в режим STATION
    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_STA);
    delay(100);
    
  
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connecting to WiFi...");
    display.print("SSID: ");
    display.println(ssid);
    display.println("Password: ");
    display.println(password);
    display.display();
    delay(1000);
    
    // Начинаем подключение
    WiFi.begin(ssid, password);
  
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connecting...");
    display.display();
  
    uint8_t retries = 0;
    uint32_t startTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && retries < 40) { // Увеличиваем количество попыток
      delay(500);
      retries++;
      
      // Показываем прогресс
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Connecting to WiFi...");
      display.print("Attempt: ");
      display.print(retries);
      display.print("/40");
      display.setCursor(0, 16);
      display.print("Status: ");
      
      // Показываем детальный статус WiFi
      switch (WiFi.status()) {
        case WL_IDLE_STATUS:
          display.println("Idle");
          break;
        case WL_NO_SSID_AVAIL:
          display.println("SSID not found");
          break;
        case WL_SCAN_COMPLETED:
          display.println("Scan completed");
          break;
        case WL_CONNECTED:
          display.println("Connected!");
          break;
        case WL_CONNECT_FAILED:
          display.println("Connect failed");
          break;
        case WL_CONNECTION_LOST:
          display.println("Connection lost");
          break;
        case WL_DISCONNECTED:
          display.println("Disconnected");
          break;
        default:
          display.print("Unknown: ");
          display.println(WiFi.status());
          break;
      }
      
      display.setCursor(0, 32);
      display.print("RSSI: ");
      if (WiFi.status() == WL_CONNECTED) {
        display.print(WiFi.RSSI());
        display.println(" dBm");
      } else {
        display.println("N/A");
      }
      
      display.setCursor(0, 32);
      display.print("Time: ");
      display.print((millis() - startTime) / 1000);
      display.println("s");
      
      display.display();
    }
    
    // Финальный результат
    display.clearDisplay();
    display.setCursor(0, 0);
    if (WiFi.status() == WL_CONNECTED) {
      saveWiFiCredential(ssid, password);
      wifiConnected = true;
      display.println("WiFi connected!");
      display.println("================");
      display.print("SSID: ");
      display.println(WiFi.SSID());
      display.print("IP: ");
      display.println(WiFi.localIP());
      display.print("Gateway: ");
      display.println(WiFi.gatewayIP());
      display.print("Subnet: ");
      display.println(WiFi.subnetMask());
      display.print("RSSI: ");
      display.print(WiFi.RSSI());
      display.println(" dBm");
    } else {
      wifiConnected = false;
      display.println("WiFi connection failed!");
      display.println("=====================");
      display.print("Last status: ");
      display.println(WiFi.status());
      display.print("Attempts: ");
      display.println(retries);
      display.print("Time: ");
      display.print((millis() - startTime) / 1000);
      display.println("s");
      display.println("");
    }
    display.display();
    delay(3000);
  }
  
  // WiFi: отключение
  void disconnectWiFi() {
    WiFi.disconnect();
    wifiConnected = false;
  }


  // Функция сохранения WiFi credential
  void saveWiFiCredential(const char* ssid, const char* pass) {
    EEPROM.begin(EEPROM_SIZE);
    WiFiCred creds[WIFI_CRED_COUNT];
    // Читаем существующие
    for (int i = 0; i < WIFI_CRED_COUNT; i++) {
      for (int j = 0; j < WIFI_CRED_MAXLEN; j++) {
        creds[i].ssid[j] = EEPROM.read(i * WIFI_CRED_MAXLEN * 2 + j);
        creds[i].pass[j] = EEPROM.read(i * WIFI_CRED_MAXLEN * 2 + WIFI_CRED_MAXLEN + j);
      }
    }
    // Проверяем, есть ли уже такая сеть
    int found = -1;
    for (int i = 0; i < WIFI_CRED_COUNT; i++) {
      if (strncmp(creds[i].ssid, ssid, WIFI_CRED_MAXLEN) == 0) {
        found = i;
        break;
      }
    }
    // Сдвигаем, если новая
    if (found != 0) {
      for (int i = WIFI_CRED_COUNT - 1; i > 0; i--) {
        creds[i] = creds[i - 1];
      }
      strncpy(creds[0].ssid, ssid, WIFI_CRED_MAXLEN);
      strncpy(creds[0].pass, pass, WIFI_CRED_MAXLEN);
    } else {
      // Если уже на первом месте — просто обновим пароль
      strncpy(creds[0].pass, pass, WIFI_CRED_MAXLEN);
    }
    // Записываем обратно
    for (int i = 0; i < WIFI_CRED_COUNT; i++) {
      for (int j = 0; j < WIFI_CRED_MAXLEN; j++) {
        EEPROM.write(i * WIFI_CRED_MAXLEN * 2 + j, creds[i].ssid[j]);
        EEPROM.write(i * WIFI_CRED_MAXLEN * 2 + WIFI_CRED_MAXLEN + j, creds[i].pass[j]);
      }
    }
    EEPROM.commit();
  }
  
  // Функция загрузки WiFi credential
  void loadWiFiCredentials(WiFiCred* creds) {
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < WIFI_CRED_COUNT; i++) {
      for (int j = 0; j < WIFI_CRED_MAXLEN; j++) {
        creds[i].ssid[j] = EEPROM.read(i * WIFI_CRED_MAXLEN * 2 + j);
        creds[i].pass[j] = EEPROM.read(i * WIFI_CRED_MAXLEN * 2 + WIFI_CRED_MAXLEN + j);
      }
    }
  }