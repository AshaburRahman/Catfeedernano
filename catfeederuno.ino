#include <Servo.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DS1302.h>

#define SERVO_PIN     9
#define POT_PIN       A0
#define BUTTON_PIN    2
#define BUZZER_PIN    3

#define RTC_CLK  6
#define RTC_DAT  7
#define RTC_RST  8

#define SERVO_STOP     1500
#define SERVO_FORWARD  1300
#define SERVO_REVERSE  1700

Servo feeder;
DS1302 rtc(RTC_RST, RTC_DAT, RTC_CLK);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0);

int feedHours[3] = {8, 14, 20};
bool fedToday[3] = {false, false, false};

void buzz() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(300);
  digitalWrite(BUZZER_PIN, LOW);
}

void feedNow() {
  int pot = analogRead(POT_PIN);
  int feedTime = map(pot, 0, 1023, 500, 3000);

  feeder.writeMicroseconds(SERVO_FORWARD);
  delay(feedTime);

  feeder.writeMicroseconds(SERVO_REVERSE);
  delay(300);

  feeder.writeMicroseconds(SERVO_FORWARD);
  delay(feedTime / 2);

  feeder.writeMicroseconds(SERVO_STOP);
  buzz();
}

void showDisplay(Time t) {
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);

  oled.setCursor(0, 12);
  oled.print("Time: ");
  oled.print(t.hour);
  oled.print(":");
  oled.print(t.min);

  oled.setCursor(0, 30);
  oled.print("Next feeds:");

  for (int i = 0; i < 3; i++) {
    oled.setCursor(0, 45 + i * 10);
    oled.print(feedHours[i]);
    oled.print(":00 ");
    oled.print(fedToday[i] ? "OK" : "WAIT");
  }

  oled.sendBuffer();
}


void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  feeder.attach(SERVO_PIN);
  feeder.writeMicroseconds(SERVO_STOP);

  rtc.halt(false);
  rtc.writeProtect(false);

  oled.begin();
}


void loop() {
  Time now = rtc.time();

  // Reset daily flags at midnight
  if (now.hour == 0 && now.min == 0) {
    for (int i = 0; i < 3; i++) fedToday[i] = false;
  }

  // Auto feeding
  for (int i = 0; i < 3; i++) {
    if (now.hour == feedHours[i] && now.min == 0 && !fedToday[i]) {
      feedNow();
      fedToday[i] = true;
    }
  }

  // Manual feed
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_PIN) == LOW) {
      feedNow();
      while (digitalRead(BUTTON_PIN) == LOW);
    }
  }

  showDisplay(now);
  delay(1000);
}
