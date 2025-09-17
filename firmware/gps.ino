  // Функция для расчета расстояния между двумя точками
  float distanceBetween(float lat1, float lon1, float lat2, float lon2) {
    float dLat = radians(lat2 - lat1);
    float dLon = radians(lon2 - lon1);
    float a = sin(dLat/2) * sin(dLat/2) +
    cos(radians(lat1)) * cos(radians(lat2)) *
    sin(dLon/2) * sin(dLon/2);
    float c = 2 * atan2(sqrt(a), sqrt(1-a));
    return EARTH_RADIUS_M * c;
  }
  
  // Функция для расчета угла между тремя точками (в градусах)
  float calculateAngle(float lat1, float lon1, float lat2, float lon2, float lat3, float lon3) {
    // Вектор 1: от точки 1 к точке 2
    float dx1 = lon2 - lon1;
    float dy1 = lat2 - lat1;
    
    // Вектор 2: от точки 2 к точке 3
    float dx2 = lon3 - lon2;
    float dy2 = lat3 - lat2;
    
    // Скалярное произведение
    float dot = dx1 * dx2 + dy1 * dy2;
    
    // Модули векторов
    float mag1 = sqrt(dx1 * dx1 + dy1 * dy1);
    float mag2 = sqrt(dx2 * dx2 + dy2 * dy2);
    
    // Проверяем деление на ноль
    if (mag1 == 0 || mag2 == 0) return 0;
    
    // Косинус угла
    float cosAngle = dot / (mag1 * mag2);
    
    // Ограничиваем значения косинуса
    if (cosAngle > 1) cosAngle = 1;
    if (cosAngle < -1) cosAngle = -1;
    
    // Возвращаем угол в градусах
    return degrees(acos(cosAngle));
  }
  
  // Конфигурация GPS
  void configureGPS() {
    // TinyGPS++ автоматически обрабатывает NMEA данные
    // Дополнительная настройка не требуется
    // GPS модуль будет работать на стандартной частоте обновления
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("GPS Configuration");
    display.println("=================");
    display.println("Using TinyGPS++");
    display.println("Auto NMEA parsing");
    display.println("Waiting for data...");
    display.display();
    
    delay(2000);
  }
  
  // Чтение UBX данных
  void readUBXData() {
    // TinyGPS++ автоматически обрабатывает NMEA данные
    // Просто читаем данные из Serial и передаем в gps.encode()
    
    while (gpsSerial.available()) {
      char c = gpsSerial.read();
      gps.encode(c);
    }
    
    // Обновляем локальные данные GPS
    if (gps.location.isValid()) {
      gpsData.latitude = gps.location.lat();
      gpsData.longitude = gps.location.lng();
      gpsData.speed = gps.speed.kmph();
      gpsData.hasFix = true;
      
      if (gps.time.isValid()) {
        gpsData.hours = gps.time.hour();
        gpsData.minutes = gps.time.minute();
        gpsData.seconds = gps.time.second();
      }
      
      gpsData.satellites = gps.satellites.value();
    } else {
      gpsData.hasFix = false;
    }
  }