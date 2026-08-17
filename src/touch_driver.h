#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#include <Arduino.h>
#include <Wire.h>

#define PIN_TP_INT 21

struct TouchPoint {
  uint16_t x;
  uint16_t y;
  bool touched;
};

class TouchDriver {
public:
  uint8_t touchAddr = 0x63;

  bool begin() {
    Wire.begin(PIN_TP_SDA, PIN_TP_SCL);
    pinMode(PIN_TP_INT, INPUT_PULLUP);
    
    // Hardware reset touch chip
    pinMode(PIN_TP_RST, OUTPUT);
    digitalWrite(PIN_TP_RST, LOW);
    delay(30);
    digitalWrite(PIN_TP_RST, HIGH);
    delay(50);

    uint8_t addrs[] = {0x63, 0x3B, 0x15, 0x5D, 0x38};
    for (uint8_t a : addrs) {
      Wire.beginTransmission(a);
      if (Wire.endTransmission() == 0) {
        touchAddr = a;
        Serial.printf("[TOUCH] Found Touch Controller at 0x%02X\n", a);
        return true;
      }
    }
    Serial.println("[TOUCH] No touch device responded to ping; listening on 0x63");
    return false;
  }

  TouchPoint readTouch() {
    TouchPoint tp = {0, 0, false};

    Wire.beginTransmission(touchAddr);
    Wire.write(0x02); // Register 0x02: Touch point count
    if (Wire.endTransmission() == 0) {
      Wire.requestFrom((uint8_t)touchAddr, (uint8_t)5);
      if (Wire.available() >= 5) {
        uint8_t touchCount = Wire.read(); // Reg 0x02
        uint8_t x_high     = Wire.read(); // Reg 0x03
        uint8_t x_low      = Wire.read(); // Reg 0x04
        uint8_t y_high     = Wire.read(); // Reg 0x05
        uint8_t y_low      = Wire.read(); // Reg 0x06

        if ((touchCount & 0x0F) > 0 || (x_high & 0x0F) > 0 || x_low > 0) {
          tp.touched = true;
          tp.x = ((x_high & 0x0F) << 8) | x_low;
          tp.y = ((y_high & 0x0F) << 8) | y_low;
          Serial.printf("[TOUCH HW PARSE] Count=%d, X=%d, Y=%d\n", touchCount, tp.x, tp.y);
        }
      }
    }
    return tp;
  }
};

#endif // TOUCH_DRIVER_H
