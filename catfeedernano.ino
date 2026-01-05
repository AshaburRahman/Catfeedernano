#include <Servo.h>

#define SERVO_PIN      9
#define TIME_POT_PIN   A0
#define VOL_POT_PIN    A1
#define BUTTON_PIN     2
#define BUZZER_PIN     3
#define LED_PIN        13

#define SERVO_STOP     1500
#define SERVO_FORWARD  1330
#define SERVO_REVERSE  1700

#define MIN_INTERVAL_HOURS  1
#define MAX_INTERVAL_HOURS  8

#define MIN_FEED_TIME_MS    500
#define MAX_FEED_TIME_MS    3000

Servo feeder;

unsigned long lastFeedTime = 0;
unsigned long feedIntervalMs = 3600000UL;
bool busy = false;

void playBuzzer() {
  tone(BUZZER_PIN, 2000);   // 2 kHz
  delay(300);
  noTone(BUZZER_PIN);
}

void feed() {
  busy = true;
  digitalWrite(LED_PIN, HIGH);

  int volumeValue = analogRead(VOL_POT_PIN);
  int feedTime = map(volumeValue, 0, 1023,
                     MIN_FEED_TIME_MS, MAX_FEED_TIME_MS);

  feeder.writeMicroseconds(SERVO_FORWARD);
  delay(feedTime);

  feeder.writeMicroseconds(SERVO_REVERSE);
  delay(300);

  feeder.writeMicroseconds(SERVO_FORWARD);
  delay(feedTime / 2);

  feeder.writeMicroseconds(SERVO_STOP);

  playBuzzer(); 

  digitalWrite(LED_PIN, LOW);
  busy = false;
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  feeder.attach(SERVO_PIN);
  feeder.writeMicroseconds(SERVO_STOP);

  lastFeedTime = millis();
}

void loop() {

  // READ TIME POT (1–8 HOURS) 
  int timeValue = analogRead(TIME_POT_PIN);
  int hours = map(timeValue, 0, 1023,
                  MIN_INTERVAL_HOURS, MAX_INTERVAL_HOURS);
  feedIntervalMs = (unsigned long)hours * 3600000UL;

  if (!busy && (millis() - lastFeedTime >= feedIntervalMs)) {
    feed();
    lastFeedTime = millis();
  }

  if (digitalRead(BUTTON_PIN) == LOW && !busy) {
    delay(50); 
    if (digitalRead(BUTTON_PIN) == LOW) {
      feed();
      while (digitalRead(BUTTON_PIN) == LOW); 
    }
  }

  delay(1000);
}