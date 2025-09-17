  // Функция для посимвольного ввода строки (логин/пароль)
  void inputString(char* buffer, int maxLen, const char* label, bool hide) {
    int pos = 0;
    int charIndex = 0;
    bool done = false;
    buffer[0] = '\0';
  
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
      if (ok.hasClicks(1) && pos < maxLen - 1) {
        buffer[pos++] = symbols[charIndex];
        buffer[pos] = '\0';
      }
      if (ok.holding()) {
        if (pos > 0) {
          pos--;
          buffer[pos] = '\0';
        }
      }
      if (ok.hasClicks(2)) {
        done = true;
      }
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print(label);
      display.setCursor(0, 10);
      for (int i = 0; i < pos; i++) display.print(hide ? '*' : buffer[i]);
      display.setCursor(0, 24);
      display.print("Add: ");
      display.print(symbols[charIndex]);
      display.setCursor(80, 24);
      display.print("2xOK");
      display.display();
      delay(1);
    }
  }
  
  // Функция авторизации на сервере
  bool authorizeOnServer(const char* login, const char* pass, String& token, String& message) {
    if (WiFi.status() != WL_CONNECTED) {
      message = "WiFi not connected!";
      return false;
    }
    HTTPClient http;
    http.begin(SERVER_URL + String(API_AUTH_ENDPOINT));
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
    String body = "username=" + String(login) + "&password=" + String(pass);
  
    int httpCode = http.POST(body);
  
    if (httpCode == 200) {
      String payload = http.getString();
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        token = doc["access_token"].as<String>();
        message = "Token OK";
        http.end();
      return true;
      } else {
        message = "JSON error";
      }
    } else {
      message = "HTTP: " + String(httpCode);
    }
    http.end();
    return false;
  }
  