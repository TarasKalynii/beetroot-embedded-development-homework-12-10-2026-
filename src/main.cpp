
#include <Arduino.h>
#include <esp_arduino_version.h>

// ==========================================
// Configuration
// ==========================================

constexpr uint8_t RELAY_PIN = 4;

// true  - 5 sec ON / 15 sec OFF
// false - 15 min ON / 45 min OFF
constexpr bool TEST_MODE = true;

constexpr uint32_t ON_TIME = TEST_MODE ? 5 : 900;
constexpr uint32_t PERIOD = TEST_MODE ? 20 : 3600;
constexpr uint32_t OFF_TIME = PERIOD - ON_TIME;

// NPN transistor: HIGH = relay ON
constexpr uint8_t RELAY_ON = HIGH;
constexpr uint8_t RELAY_OFF = LOW;

// ==========================================
// Timer state
// ==========================================

hw_timer_t* timer = nullptr;

volatile bool relayOn = false;
volatile uint32_t elapsedSeconds = 0;

// ==========================================
// Hardware timer interrupt (every 1 second)
// ==========================================

void IRAM_ATTR onTimer() {
    elapsedSeconds++;

    if (relayOn && elapsedSeconds >= ON_TIME) {
        relayOn = false;
        elapsedSeconds = 0;

        digitalWrite(RELAY_PIN, RELAY_OFF);
    }
    else if (!relayOn && elapsedSeconds >= OFF_TIME) {
        relayOn = true;
        elapsedSeconds = 0;

        digitalWrite(RELAY_PIN, RELAY_ON);
    }
}

// ==========================================
// Setup
// ==========================================

void setup() {
    Serial.begin(115200);
    delay(1000); // Serial Monitor initialization only

    // Start with relay OFF
    digitalWrite(RELAY_PIN, RELAY_OFF);
    pinMode(RELAY_PIN, OUTPUT);

    // Initialize hardware timer
#if ESP_ARDUINO_VERSION_MAJOR >= 3

    timer = timerBegin(1000000);

    if (timer == nullptr) {
        Serial.println("Timer initialization failed");
        return;
    }

    timerAttachInterrupt(timer, &onTimer);

#else

    timer = timerBegin(0, 80, true);

    if (timer == nullptr) {
        Serial.println("Timer initialization failed");
        return;
    }

    timerAttachInterrupt(timer, &onTimer, true);

#endif

    // Turn relay ON before starting the timer
    relayOn = true;
    digitalWrite(RELAY_PIN, RELAY_ON);

    Serial.println("Relay controller started");
    Serial.println("Relay ON");

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    timerAlarm(timer, 1000000, true, 0);
#else
    timerAlarmWrite(timer, 1000000, true);
    timerAlarmEnable(timer);
#endif
}

// ==========================================
// Main loop - logging only
// ==========================================

void loop() {
    static bool previousState = true;

    bool currentState = relayOn;

    if (currentState != previousState) {
        previousState = currentState;

        Serial.println(
            currentState ? "Relay ON" : "Relay OFF"
        );
    }
}
