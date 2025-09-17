  // Начало записи трека
  void startTrackRecording() {
    // Инициализируем SD карту
    if (!SD.begin(SD_CS_PIN)) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("=====================");
      display.println("");
      display.println("");
      display.println("ERROR: SD not initialized!");
      display.println("");
      display.println("");
      display.println("=====================");
      display.display();
      delay(2000);
      return;
    }
    
    // Создаем имя файла с временной меткой
    uint32_t timestamp = millis();
    currentTrackFile = "tr_" + String(timestamp) + ".json";
    
    // Показываем информацию о создании файла
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("START TRACK RECORDING");
    display.println("=====================");
    display.println("");
    display.println("");
    display.println(currentTrackFile);
    display.println("");
    display.println("");
    display.println("=====================");
    display.display();
    delay(1000);
    
    // Детальная диагностика создания файла трека
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("CREATING TRACK FILE");
    display.println("=====================");
    display.println("");
    display.println("");
    display.println(currentTrackFile);
    display.println("");
    display.println("");
    display.println("=====================");
    display.display();
    delay(500);
    
    // Проверяем SD карту перед созданием
    if (!SD.begin(SD_CS_PIN)) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("=====================");
      display.println("");
      display.println("");
      display.println("ERROR: SD not ready!");
      display.println("");
      display.println("");
      display.println("=====================");
      display.display();
      delay(2000);
      return;
    }
    
    // Пробуем создать файл с детальной диагностикой
    File file = SD.open("/" + currentTrackFile, FILE_WRITE);
    if (!file) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("=====================");
      display.println("");
      display.println("");
      display.println("ERROR: Failed to create:");
      display.println(currentTrackFile);
      display.println("");
      display.println("");
      display.println("=====================");
      display.display();
      delay(1000);
    }
    
    if (!file) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("All attempts failed!");
      display.println("SD Status: " + String(SD.cardType()));
      display.println("Free space: " + String(SD.totalBytes() - SD.usedBytes()));
      display.display();
      delay(2000);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("ERROR: File creation failed!");
      display.display();
      delay(2000);
      return;
    }
    
    // Файл создался - записываем начало JSON массива
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("File created! Writing...");
    display.display();
    
    file.println("{\"track\":["); // Начинаем JSON массив точек
    file.flush(); // Принудительная синхронизация
    file.close();
    
    // Проверяем, что файл создался
    if (SD.exists("/" + currentTrackFile)) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("TRACK FILE CREATED!");
      display.println("=====================");
      display.println("       Name: ");
      display.println("");
      display.println(currentTrackFile);
      display.println("");
      display.println(" READY FOR RECORDING");
      display.println("=====================");
      display.display();
      delay(1000);
      
      // Инициализируем переменные записи трека
      trackPoints = 0;
      lastTrackWrite = 0;
      trackStartTime = millis(); // Запоминаем время начала записи
      isRecording = true; // Устанавливаем флаг записи
      flag_track = true; // Также устанавливаем старый флаг для совместимости
      
      // Сброс фильтрации по расстоянию и углу
      hasLastPoint = false;
      hasPrevPoint = false;
      lastLat = 0.0;
      lastLon = 0.0;
      prevLat = 0.0;
      prevLon = 0.0;
      
    } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("=====================");
      display.println("      ERROR: ");
      display.println("");
      display.println("   File exists ");
      display.println("  but not found!");
      display.println("");
      display.println("=====================");
      display.display();
      delay(2000);
      return;
    }
  }
  
  // Завершение записи трека
  void stopTrackRecording() {
    if (currentTrackFile == "") {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("ERROR: No active track!");
      display.display();
      delay(2000);
      return;
    }
    
    // Показываем информацию о завершении
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("STOP TRACK RECORDING");
    display.println("=====================");
    display.println("");
    display.println(currentTrackFile);
    display.println("");
    display.print("Points recorded: ");
    display.println(trackPoints);
    display.println("");
    display.println("=====================");
    display.display();
    delay(1000);
    
    // Завершаем запись трека - закрываем JSON массив
    File file = SD.open("/" + currentTrackFile, FILE_WRITE);
    
    if (file) {
      // Закрываем JSON массив
      if (trackPoints > 0) {
        file.print("\n]}"); // Закрываем массив с точками и объект
      } else {
        file.print("]}"); // Закрываем пустой массив и объект
      }
      file.flush(); // Принудительная синхронизация
      file.close();
      
      // Проверяем, что файл сохранился
      if (SD.exists("/" + currentTrackFile)) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("  TRACK FILE SAVED!");
        display.println("=====================");
        display.println("");
        display.println(currentTrackFile);
        display.println("");
        display.println("Points: ");
        display.println(trackPoints);
        display.println("");
        display.println("=====================");
        display.display();
        delay(1000);
        
        // Сбрасываем флаг записи и очищаем имя файла
        isRecording = false;
        flag_track = false; // Также сбрасываем старый флаг
        currentTrackFile = "";
        trackPoints = 0;
      } else {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("=====================");
        display.println("        ERROR:");
        display.println("");
        display.println(" Track file not saved!");
        display.println("");
        display.println("");
        display.println("=====================");
        display.display();
        delay(2000);
        return;
      }
    } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("=====================");
      display.println("        ERROR:");
      display.println("     Cannot close");
      display.println("      track file!");
      display.println("");
      display.println("=====================");
      display.display();
      delay(2000);
      return;
    }
    
    // Вычисляем время записи (разность между текущим временем и временем начала)
    uint32_t recordTime = (millis() - trackStartTime) / 1000; // Время записи в секундах
    
    // Показываем результат с информацией о записи
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("     TRACK SAVED!");
    display.println("=====================");
    display.println("");
    display.println("Time: ");
    uint8_t m = recordTime / 60;
    uint8_t s = recordTime % 60;
    if (m < 10) display.print("0"); display.print(m);
    display.print(":");
    if (s < 10) display.print("0"); display.print(s);
    display.setCursor(0, 16);
    display.print("Points: ");
    display.println(trackPoints);
    display.println("");
    display.println("=====================");
    display.display();
    
    // Ждем нажатия с максимально быстрой реакцией
    while (!ok.click()) { 
      int analog = analogRead(ANALOG_PIN);
      bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
      bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
      bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
      
      up.tick(upPressed);
      ok.tick(okPressed);
      down.tick(downPressed);
      delay(5); // Минимальная задержка для лучшей реакции
      
      // Дополнительная обработка для выхода по долгому нажатию
      if (up.holding()) {
        break; // Выходим по долгому нажатию
      }
    }
    
    currentTrackFile = "";
    trackPoints = 0;
    trackStartTime = 0; // Сбрасываем время начала
  }
  
  // Запись точки трека
  void writeTrackPoint() {
    if (!isRecording || currentTrackFile == "") return;
    
    // Обработка нажатия кнопки во время записи
    int analog = analogRead(ANALOG_PIN);
    bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
    bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
    bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
    
    up.tick(upPressed);
    ok.tick(okPressed);
    down.tick(downPressed);
  
    if (ok.click()) {
      stopTrackRecording();
      return;
    }
    
    // Быстрое чтение GPS данных без блокировки
    while (gpsSerial.available()) {
      char c = gpsSerial.read();
      gps.encode(c);
    }
    
    // Записываем точку только если есть GPS фикс
    if (gps.location.isValid()) {
      bool shouldWrite = false;
      
      // Проверяем расстояние до предыдущей точки
      if (hasLastPoint) {
        float dist = distanceBetween(lastLat, lastLon, gps.location.lat(), gps.location.lng());
        
        // Если расстояние >= 2 метра - записываем
        if (dist >= 2.0) {
          shouldWrite = true;
        }
        // Если расстояние < 2 метра, но есть резкий поворот - тоже записываем
        else if (hasPrevPoint) {
          float angle = calculateAngle(prevLat, prevLon, lastLat, lastLon, gps.location.lat(), gps.location.lng());
          if (angle >= 20.0) { // Угол >= 20 градусов считается резким поворотом
            shouldWrite = true;
          }
        }
      } else {
        // Первая точка - всегда записываем
        shouldWrite = true;
      }
      
      if (!shouldWrite) return; // Не записываем точку
      
      // Открываем файл для записи в JSON формате
      File file = SD.open("/" + currentTrackFile, FILE_WRITE);
      
      if (file) {
        // Добавляем запятую перед точкой если это не первая точка
        if (trackPoints > 0) {
          file.print(",");
        }
        
        // Записываем точку в JSON формате согласно API
        file.print("{\"latitude\":");
        file.print(gps.location.lat(), 6);
        file.print(",\"longitude\":");
        file.print(gps.location.lng(), 6);
        file.print(",\"timestamp\":\"");
        
        // Формируем timestamp в ISO 8601 формате
        if (gps.date.isValid()) {
          file.print(gps.date.year());
          file.print("-");
          if (gps.date.month() < 10) file.print("0");
          file.print(gps.date.month());
          file.print("-");
          if (gps.date.day() < 10) file.print("0");
          file.print(gps.date.day());
        } else {
          file.print("2025-01-01"); // Fallback дата
        }
        
        file.print("T");
        
        if (gps.time.isValid()) {
          if (gps.time.hour() < 10) file.print("0");
          file.print(gps.time.hour());
          file.print(":");
          if (gps.time.minute() < 10) file.print("0");
          file.print(gps.time.minute());
          file.print(":");
          if (gps.time.second() < 10) file.print("0");
          file.print(gps.time.second());
        } else {
          file.print("00:00:00"); // Fallback время
        }
        
        file.print(".000Z\",\"altitude\":");
        
        // Добавляем altitude
        if (gps.altitude.isValid()) {
          file.print(gps.altitude.meters(), 1);
        } else {
          file.print("0.0");
        }
        
        file.print(",\"speed\":");
        
        // Добавляем speed
        if (gps.speed.isValid()) {
          file.print(gps.speed.mps(), 1); // метры в секунду
        } else {
          file.print("0.0");
        }
        
        file.print("}");
        
        file.flush(); // Принудительная синхронизация
        file.close();
        trackPoints++;

        // Обновляем историю точек для расчета угла
        if (hasLastPoint) {
          prevLat = lastLat;
          prevLon = lastLon;
          hasPrevPoint = true;
        }
        
        // Сохраняем координаты последней записанной точки
        lastLat = gps.location.lat();
        lastLon = gps.location.lng();
        hasLastPoint = true;
      }
    }
  }
  
  // Отправка трека чанками
  void sendTrackToServer(const char* filename) {
    if (WiFi.status() != WL_CONNECTED || jwtToken == "") {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("No WiFi or token!");
      display.display();
      delay(2000);
      return;
    }

    File file = SD.open(filename, FILE_READ);
    if (!file) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("File not found!");
      display.println(filename);
      display.display();
      delay(2000);
      return;
    }

    // Пропускаем до начала массива track
    String buffer = "";
    char c;
    while (file.available()) {
      c = file.read();
      buffer += c;
      if (buffer.endsWith("\"track\":[")) {
        break;
      }
      if (buffer.length() > 20) {
        buffer = buffer.substring(1); // Сдвигаем буфер
      }
    }

    int pointsInChunk = 0;
    String chunkPoints = "";
    bool isFirstChunk = true;
    bool isLastChunk = false;
    int trackId = -1;
    int totalPoints = 0;
    int sentPoints = 0;

    // Для индикации
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Sending: ");
    display.println(filename);
    display.display();

    while (file.available()) {
      // Читаем одну точку (объект {...})
      String point = "";
      int braceLevel = 0;
      bool pointStarted = false;
      
      while (file.available()) {
        char ch = file.read();
        if (ch == '{') {
          braceLevel++;
          pointStarted = true;
        }
        if (pointStarted) point += ch;
        if (ch == '}') {
          braceLevel--;
          if (braceLevel == 0) break;
        }
      }
      if (point.length() == 0) break; // конец файла

      // Добавляем точку в чанк
      if (pointsInChunk > 0) chunkPoints += ",";
      chunkPoints += point;
      pointsInChunk++;
      totalPoints++;

      // Проверяем, пора ли отправлять чанк
      bool endOfArray = false;
      // Пропускаем пробелы и запятые до следующей точки или ]}
      String endBuffer = "";
      while (file.available()) {
        char peek = file.peek();
        if (peek == ',' || peek == ' ' || peek == '\n' || peek == '\r') {
          file.read(); // пропускаем
        } else if (peek == ']') {
          // Проверяем следующий символ
          file.read(); // читаем ]
          if (file.available() && file.peek() == '}') {
            endOfArray = true;
            break;
          }
        } else {
          break;
        }
      }

      if (pointsInChunk >= CHUNK_SIZE || endOfArray) {
        isLastChunk = endOfArray;

        // Формируем JSON чанка согласно API
        String json = "{";
        if (isFirstChunk) {
          // Убираем расширение .json из имени файла для трека
          String trackName = String(filename);
          if (trackName.endsWith(".json")) {
            trackName = trackName.substring(0, trackName.length() - 5);
          }
          json += "\"name\":\"" + trackName + "\",";
          json += "\"description\":\"Track recorded by GPS tracker\",";
          json += "\"is_first_chunk\":true,";
        } else {
          json += "\"track_id\":" + String(trackId) + ",";
          json += "\"is_first_chunk\":false,";
        }
        json += "\"is_last_chunk\":" + String(isLastChunk ? "true" : "false") + ",";
        json += "\"points\":[" + chunkPoints + "]}";

        // Отправляем HTTP POST
        HTTPClient http;
        http.begin(SERVER_URL + String(API_TRACKS_CHUNK_ENDPOINT));
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", "Bearer " + jwtToken);
        
        int httpCode = http.POST(json);

        if (httpCode == 200) {
          String response = http.getString();
          
          // Парсим track_id из JSON ответа более надежно
          if (isFirstChunk && trackId == -1) {
            int trackIdStart = response.indexOf("\"track_id\":");
            if (trackIdStart != -1) {
              trackIdStart += 11; // длина "track_id":
              // Пропускаем пробелы
              while (trackIdStart < response.length() && response.charAt(trackIdStart) == ' ') {
                trackIdStart++;
              }
              // Находим конец числа
              int trackIdEnd = trackIdStart;
              while (trackIdEnd < response.length() && 
                     (response.charAt(trackIdEnd) >= '0' && response.charAt(trackIdEnd) <= '9')) {
                trackIdEnd++;
              }
              if (trackIdEnd > trackIdStart) {
                String trackIdStr = response.substring(trackIdStart, trackIdEnd);
                trackId = trackIdStr.toInt();
              }
            }
          }
          
          // Индикация прогресса
          sentPoints += pointsInChunk;
          display.clearDisplay();
          display.setCursor(0, 0);
          display.print("Sent: ");
          display.print(sentPoints);
          if (isLastChunk) {
            display.print(" (done)");
          }
          display.display();
          
        } else {
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Send failed!");
          display.print("HTTP: ");
          display.println(httpCode);
          if (httpCode > 0) {
            String errorResponse = http.getString();
            display.println("Response:");
            display.println(errorResponse.substring(0, 20)); // Показываем первые 20 символов ошибки
          }
          display.display();
          http.end();
          file.close();
          delay(3000);
          return;
        }
        http.end();

        // Сброс для следующего чанка
        chunkPoints = "";
        pointsInChunk = 0;
        isFirstChunk = false;
        if (isLastChunk) break;
        delay(200); // Для стабильности
      }
    }
    file.close();
  
    // Финальное сообщение
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Track sent!");
    display.println("Push to continue");
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