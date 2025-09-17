// ============================================================================
// КОНФИГУРАЦИЯ ПИНОВ И НАСТРОЕК
// ============================================================================

// Пин для аналогового чтения кнопок
#define ANALOG_PIN 0  // GPIO0 (ADC1_CH1)

// Диапазоны напряжения для каждой кнопки
#define UP_MIN 1307    // Минимальное значение для кнопки UP
#define UP_MAX 1507    // Максимальное значение для кнопки UP
#define OK_MIN 2750    // Минимальное значение для кнопки OK
#define OK_MAX 2950    // Максимальное значение для кнопки OK
#define DOWN_MIN 4080  // Минимальное значение для кнопки DOWN
#define DOWN_MAX 4180  // Максимальное значение для кнопки DOWN

// GPS подключение
#define GPS_RX 20  // ESP32 RX (подключается к TX GPS)
#define GPS_TX 21  // ESP32 TX (подключается к RX GPS)

// OLED подключение
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C

// WiFi credentials
#define WIFI_CRED_MAXLEN 32
#define WIFI_CRED_COUNT 3
#define EEPROM_SIZE (WIFI_CRED_COUNT * WIFI_CRED_MAXLEN * 2)

// SD карта подключение
#define SD_CS_PIN 7

// Константы для API
#define SERVER_URL "http://192.168.0.112:8000"
#define API_AUTH_ENDPOINT "/api/v1/auth/login"
#define API_TRACKS_ENDPOINT "/api/v1/tracks/upload"
#define API_TRACKS_CHUNK_ENDPOINT "/api/v1/tracks/load_from_tracker"

// Отправка трека чанками
#define CHUNK_SIZE 50          // Оптимальный размер ~5.6KB для ESP32-C3

// Радиус Земли
#define EARTH_RADIUS_M 6371000.0