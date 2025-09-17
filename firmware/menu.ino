  // Меню
  void drawMainMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Main Menu");
    display.println("=====================");
    display.println(mainPointer == 0 ? "> GPS Status" : "  GPS Status");
    display.println(mainPointer == 1 ? "> WiFi Settings" : "  WiFi Settings");
    display.println(mainPointer == 2 ? "> Track Recording" : "  Track Recording");
    display.println(mainPointer == 3 ? "> SD Card" : "  SD Card");
    display.println("=====================");
    display.println("  Hold UP to return");
    display.display();
  }
  
  // Меню GPS
  void drawGPSMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("GPS ");
    display.println("=====================");
    
    if (!gps.location.isValid()) {
      display.setCursor(0, 19);
      display.println("   Searching GPS...");
      display.print("   Satellites: ");
      display.println(gps.satellites.value());
      display.print("   Chars: ");
      display.println(gps.charsProcessed());
    } else {
      // Форматирование координат для удобного отображения
      double lat = gps.location.lat();
      double lng = gps.location.lng();
      char latDir = (lat >= 0) ? 'N' : 'S';
      char lngDir = (lng >= 0) ? 'E' : 'W';
      lat = fabs(lat);
      lng = fabs(lng);
  
      // Координаты
      display.setCursor(0, 16);
      display.print("Lat: ");
      display.print(lat, 6);
      display.print(latDir);
      
      display.setCursor(0, 24);
      display.print("Lng: ");
      display.print(lng, 6);
      display.print(lngDir);
      
      // Спутники и высота
      display.setCursor(0, 32);
      display.print("Sats: ");
      display.print(gps.satellites.value());
      
      display.setCursor(64, 32);
      display.print("Alt: ");
      display.print(gps.altitude.meters(), 0);
      display.print("m");
      
      // Скорость
      display.setCursor(0, 40);
      display.print("Speed: ");
      if (gps.speed.kmph() > 0) {
        display.print(gps.speed.kmph(), 1);
        display.print(" km/h");
      } else {
        display.print("0 km/h");
      }
      
      // Время
      display.setCursor(24, 0);
      if (gps.time.isValid()) {
        display.print("Time: ");
        if (gps.time.hour() < 10) display.print("0");
        display.print(gps.time.hour());
        display.print(":");
        if (gps.time.minute() < 10) display.print("0");
        display.print(gps.time.minute());
        display.print(":");
        if (gps.time.second() < 10) display.print("0");
        display.print(gps.time.second());
      } else {
        display.print("Time: No fix");
      }
    }
    
    display.setCursor(0, 48);
    display.println("=====================");
    display.println("  Hold UP to return");
    
    display.display();
  }
  
  // Меню WiFi
  void drawWiFiMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("WiFi Settings");
    display.println("=====================");
    
    if (!wifiConnected) {
      display.println(wifiPointer == 0 ? "> Connect to WiFi" : "  Connect to WiFi");
      display.println("");
      display.println("Status: Disconnected");
      display.println("");
      display.println("=====================");
      display.println("  Hold UP to return");
    } else {
      display.println(wifiPointer == 0 ? "> Connected" : "  Connected");
      display.println(wifiPointer == 1 ? "> Authentication" : "  Authentication");
      display.println(wifiPointer == 2 ? "> Send Track" : "  Send Track");
      display.println(wifiPointer == 3 ? "> Send All Tracks" : "  Send All Tracks");
      display.println("=====================");
      display.println(WiFi.SSID());
      display.print("IP: ");
      display.println(WiFi.localIP());
    }
    display.display();
  }
  
  // Меню Track
  void drawTrackMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Track: ");
    display.println("=====================");
    
    if (!flag_track) {
      display.setCursor(50, 0);
      display.println("STOPPED");
      display.setCursor(0, 16);
      display.println("Press OK to START");
      display.println("");
            if (gps.location.isValid()) {
          display.println("|  Lat    |    Lon  |");
          display.print("| ");
          display.print(gps.location.lat(), 4);
          display.print(" | ");
          display.print(gps.location.lng(), 4);
          display.print(" |");
        } else {
        display.println("GPS: No fix");
      }
    } else {
      display.setCursor(50, 0);
      display.println("RECORDING");
      display.setCursor(0, 16);
      display.println("Press OK to STOP");
      display.print("Points recorded: ");
      display.println(trackPoints);
        if (gps.location.isValid()) {
          display.println("|  Lat    |    Lon  |");
          display.print("| ");
          display.print(gps.location.lat(), 4);
          display.print(" | ");
          display.print(gps.location.lng(), 4);
          display.print(" |");
        } else {
        display.println("GPS: No fix");
      }
    }
    
    display.setCursor(0, 48);
    display.println("=====================");
    display.println("  Hold UP to return");
    display.display();
  }
  
  // Меню SD Card
  void drawSDCardMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("SD Card Management");
    display.println("=====================");
    display.println(sdPointer == 0 ? "> View Files" : "  View Files");
    display.println(sdPointer == 1 ? "> Format Card" : "  Format Card");
    display.println(sdPointer == 2 ? "> Test Card" : "  Test Card");
    display.println("");
    display.println("=====================");
    display.println("  Hold UP to return");
    display.display();
  }