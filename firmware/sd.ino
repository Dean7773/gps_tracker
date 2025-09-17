  // Просмотр файлов на SD с удалением
  void viewSDFiles() {
    display.clearDisplay();
    display.setCursor(0, 0);
    if (!SD.begin(SD_CS_PIN)) {
      display.println("SD not found!");
      display.display();
      delay(2000);
      return;
    }
    
    File root = SD.open("/");
    if (!root) {
      display.println("SD open error!");
      display.display();
      delay(2000);
      return;
    }
    
    // Считываем имена файлов
    char fileNames[16][32];
    uint32_t fileSizes[16];
    int fileCount = 0;
    
    File entry = root.openNextFile();
    while (entry && fileCount < 16) {
      if (!entry.isDirectory()) {
        strncpy(fileNames[fileCount], entry.name(), sizeof(fileNames[fileCount]));
        fileSizes[fileCount] = entry.size();
        fileCount++;
      }
      entry.close();
      entry = root.openNextFile();
    }
    root.close();
    
    int pointer = 0, scroll = 0;
    const int visibleRows = 4; // Увеличено для экрана 64x128
    
    if (fileCount == 0) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("No files!");
      display.display();
      delay(1500);
      return;
    }
    
    while (fileCount > 0) {
      int analog = analogRead(ANALOG_PIN);
      bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
      bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
      bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
      
      up.tick(upPressed);
      ok.tick(okPressed);
      down.tick(downPressed);
  
      if (up.click()) pointer = (pointer + fileCount - 1) % fileCount;
      if (down.click()) pointer = (pointer + 1) % fileCount;
      if (pointer < scroll) scroll = pointer;
      if (pointer >= scroll + visibleRows) scroll = pointer - visibleRows + 1;
      
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Files: 2xOK to delete");
      display.println("=====================");
      for (int i = 0; i < visibleRows; i++) {
        int idx = scroll + i;
        if (idx >= fileCount) break;
        display.setCursor(0, 16 + i * 8);
        if (idx == pointer) display.print("> ");
        else display.print("  ");
        // Обрезаем длинные имена
        char shortName[20];
        strncpy(shortName, fileNames[idx], 18);
        shortName[18] = '\0';
        if (strlen(fileNames[idx]) > 18) strcat(shortName, "…");
        display.print(shortName);
      }
      display.setCursor(0, 48);
      display.println("=====================");
      display.println("  Hold UP to return");
      display.display();
      if (ok.hasClicks(2)) {
        // Удаляем выбранный файл
        if (SD.remove("/" + String(fileNames[pointer]))) {
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("   FILE DELETED:");
          display.println("=====================");
          display.println("");
          display.print(fileNames[pointer]);
          display.println("");
          display.println("");
          display.println("");
          display.println("=====================");
          display.display();
          delay(1000);
          // Обновляем список файлов
          viewSDFiles();
          return;
        } else {
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("   DELETE ERROR:");
          display.println("=====================");
          display.println("");
          display.print(fileNames[pointer]);
          display.println("");
          display.println("");
          display.println("");
          display.println("=====================");
          display.display();
          delay(1000);
        }
      }
      if (up.holding()) break;
      delay(1);
    }
  }
  
  // Форматирование SD
  void formatSDCard() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Format SD?");
    display.println("Double push = YES");
    display.println("Hold = NO");
    display.display();
    bool confirmed = false;
    while (true) {
      int analog = analogRead(ANALOG_PIN);
      bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
      bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
      bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
      
      up.tick(upPressed);
      ok.tick(okPressed);
      down.tick(downPressed);
  
      if (ok.hasClicks(2)) { confirmed = true; break; }
      if (up.holding()) break;
      delay(1);
    }
    if (confirmed) {
      if (!SD.begin(SD_CS_PIN)) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("SD not found!");
        display.display();
        delay(2000);
        return;
      }
      
      // Для ESP32 форматирование не поддерживается стандартной библиотекой
      // Вместо этого удаляем все файлы
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("ESP32 SD library");
      display.println("doesn't support");
      display.println("formatting.");
      display.println("Use PC to format.");
      display.display();
      delay(3000);
    } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Format cancelled");
      display.display();
      delay(1000);
    }
  }
  
  // SD Card Test
  void testSDCard() {
    if (!SD.begin(SD_CS_PIN)) {
      display.println("SD not initialized!");
      display.display();
      delay(1000);
      return;
    }
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Testing SD Card...");
    display.println("Step 1: Write test");
    display.display();
    delay(1000);
    
    // Проверяем свободное место
    size_t totalBytes = SD.totalBytes();
    size_t usedBytes = SD.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SD Card Info:");
    display.print("Total: ");
    display.print(totalBytes / 1024);
    display.println(" KB");
    display.print("Used: ");
    display.print(usedBytes / 1024);
    display.println(" KB");
    display.print("Free: ");
    display.print(freeBytes / 1024);
    display.println(" KB");
    display.display();
    delay(2000);
    
    // Тест записи
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Creating test file...");
    display.println("test.txt");
    display.display();
    delay(1000);
    
    // Пробуем разные варианты создания файла
    File testFile = SD.open("/test.txt", FILE_WRITE);
    
    if (testFile) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("File created!");
      display.println("Writing data...");
      display.display();
      delay(500);
      
      testFile.println("SD Card Test");
      testFile.println("Time: " + String(millis()));
      testFile.println("Status: OK");
      testFile.println("ESP32 Test");
      
      // Принудительная синхронизация данных
      testFile.flush();
      testFile.close();
      
      // Проверяем, что файл действительно создался
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Checking file...");
      display.display();
      delay(500);
      
      // Пробуем разные имена файла для чтения
      File readFile = SD.open("/test.txt", FILE_READ);
      
      if (readFile) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("File opened for read!");
        display.print("Name: ");
        display.println(readFile.name());
        display.print("Size: ");
        display.print(readFile.size());
        display.println(" bytes");
        display.println("Reading content...");
        display.display();
        delay(1000);
        
        String content = readFile.readString();
        readFile.close();
        
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Read completed!");
        display.print("Content: ");
        display.print(content);
        display.println("Step 3: Delete test");
        display.display();
        delay(2000);
        
        // Удаляем тестовый файл
        bool deleteSuccess = false;
        if (SD.remove("/test.txt")) {
          deleteSuccess = true;
        }
        
        if (deleteSuccess) {
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("SD Test PASSED!");
          display.println("Write/Read/Delete OK");
          display.display();
          delay(2000);
        } else {
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("ERROR: SD Delete failed!");
          display.display();
          delay(2000);
        }
      } else {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("ERROR: SD Read failed!");
        display.display();
        delay(2000);
      }
    } else {
      // Детальная диагностика ошибки записи
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("SD Write FAILED!");
      display.println("================");
      display.println("Possible reasons:");
      display.println("- Card is write-protected");
      display.println("- Not enough space");
      display.println("- File system error");
      display.println("- Hardware issue");
      display.println("");
      display.println("Press OK to continue");
      display.display();
      
      while (true) {
        int analog = analogRead(ANALOG_PIN);
        bool upPressed = (analog >= UP_MIN && analog <= UP_MAX);
        bool okPressed = (analog >= OK_MIN && analog <= OK_MAX);
        bool downPressed = (analog >= DOWN_MIN && analog <= DOWN_MAX);
        
        up.tick(upPressed);
        ok.tick(okPressed);
        down.tick(downPressed);
        
        if (ok.click()) break;
        delay(100);
      }
    }
  }