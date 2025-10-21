// Include BLE Keyboard library
#include <BleKeyboard.h>
// Include ezButton library
#include <ezButton.h>

// RBG led pin definition
#define G D0
#define R D2
#define B D4

// definition for button connected to XIAO C3
#define U_BUT D1
#define L_BUT D3
#define C_BUT D5
#define R_BUT D7
#define D_BUT D9

// BLEMOUSE global declaration
BleKeyboard bleKeyboard;

bool srl = false; // Show output on the Serial
bool status = true;
unsigned long last_activity; // determine if it's time to go to deep sleep

#define DEEP_SLEEP_IDLE_TIME (10 * 60 * 1000)
#define DEFAULT_WAKEUP_PIN 3
#define DEFAULT_WAKEUP_LEVEL  ESP_GPIO_WAKEUP_GPIO_LOW

#define DEBOUNCE_TIME 50 // the debounce time in millisecond, increase this time if it still chatters

ezButton u_but(U_BUT); // create ezButton object that attach to pin GPIO3;
ezButton l_but(L_BUT); // create ezButton object that attach to pin GPIO5;
ezButton c_but(C_BUT); // GPIO 7
ezButton r_but(R_BUT); // GPIO 20
ezButton d_but(D_BUT); // GPIO 9


void setup() {
  if (srl) {
    Serial.begin(115200);
    delay(500);
  }

  print_wakeup_reason();

  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);

  bleKeyboard.begin();

  u_but.setDebounceTime(DEBOUNCE_TIME);
  l_but.setDebounceTime(DEBOUNCE_TIME);
  c_but.setDebounceTime(DEBOUNCE_TIME);
  r_but.setDebounceTime(DEBOUNCE_TIME);
  d_but.setDebounceTime(DEBOUNCE_TIME);


  last_activity = millis();
}

void enable_deep_sleep_on_u_but() {

  const gpio_config_t config = {
      .pin_bit_mask = BIT(DEFAULT_WAKEUP_PIN),
      .mode         = GPIO_MODE_INPUT,
  };
  ESP_ERROR_CHECK(gpio_config(&config));
  ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(BIT(DEFAULT_WAKEUP_PIN), DEFAULT_WAKEUP_LEVEL));
  ESP_LOGI("TAG", "Enabling GPIO wakeup on pins GPIO%d\n", DEFAULT_WAKEUP_PIN);

}


/*
  Method to print the reason by which ESP32
  has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     if (srl) Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     if (srl) Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    if (srl) Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: if (srl) Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      if (srl) Serial.println("Wakeup caused by ULP program"); break;
    case ESP_SLEEP_WAKEUP_GPIO:     if (srl) Serial.println("Wakeup caused by GPIO"); break;
    case ESP_SLEEP_WAKEUP_UNDEFINED:if (srl) Serial.println("Wakeup cause unknown"); break;
    default:                        if (srl) Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

// when bluetooth is not connected
void not_connected()
{
  digitalWrite(R, HIGH);
  delay(500);
  digitalWrite(R, LOW);
  delay(500);

  digitalWrite(G, LOW);
  digitalWrite(B, LOW);
  status = true;
}

// when bluetooth is connected and but is presses led
void but_pressed()
{
  digitalWrite(B, HIGH);
  last_activity = millis();
}

// to read button value and transmit data
void keyboardconnected() {

  u_but.loop(); // MUST call the loop() function first
  l_but.loop(); // MUST call the loop() function first
  c_but.loop(); // MUST call the loop() function first
  r_but.loop(); // MUST call the loop() function first
  d_but.loop(); // MUST call the loop() function first
  
  if (u_but.isPressed()) {
    bleKeyboard.write(KEY_UP_ARROW);
    but_pressed();
    if (srl) Serial.println("UP KEY PRESSED");
  }
  if (d_but.isPressed()) {
    bleKeyboard.write(KEY_DOWN_ARROW);
    but_pressed();
    if (srl) Serial.println("DOWN KEY PRESSED");
  }
  if (l_but.isPressed()) {
    bleKeyboard.write(KEY_LEFT_ARROW);
    but_pressed();
    if (srl) Serial.println("LEFT KEY PRESSED");
  }
  if (r_but.isPressed()) {
    bleKeyboard.write(KEY_RIGHT_ARROW);
    but_pressed();
    if (srl) Serial.println("RIGHT KEY PRESSED");
  }
  if (c_but.isPressed()) {
    bleKeyboard.write(KEY_PAGE_UP);
    but_pressed();
    if (srl) Serial.println("CENTER KEY PRESSED");
  } else {
    digitalWrite(B, LOW);
  }
}


void loop() {
  if (bleKeyboard.isConnected()) {
    if (status == true) {
      digitalWrite(G,HIGH);
      delay(3000);
      status = false;
    }
    digitalWrite(G,LOW);
    keyboardconnected();
  } else {
    not_connected();
  }

  // If there's no activity for DEEP_SLEEP_IDLE_TIME ms, go to sleep
  if (millis() > (last_activity + DEEP_SLEEP_IDLE_TIME)) {
    if (srl) Serial.println("Going to sleep ...");
    enable_deep_sleep_on_u_but();
    esp_deep_sleep_start();
  }
}
