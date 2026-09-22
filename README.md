# 🤖 DIY Interactive Desktop Companion Robot

Ek compact, expressive aur intelligent desktop robot jo Arduino Nano, 0.96" OLED display, DRV8833 dual motor driver, capacitive touch sensor aur buzzer ke sath kaam karta hai. Yeh robot table par dheere-dheere ghoomta hai, touch karne par alag-alag bhavnayen (emotions) dikhata hai aur sounds nikalta hai.

---

## ✨ Features

- **Expressive OLED Animations (13 Moods):** Happy, Curious Look-Around, Angry, Sad with Animated Falling Tears, Sleepy, Pulsing Heart Eyes, Shock, Dizzy, Cute Puppy Eyes, Wink & Star, Suspicious, Sci-Fi Cyber Scan, aur Dead (KO).
- **Custom Sound Effects:** Har mood aur action ke liye alag 8-bit retro sound frequencies.
- **Touch Interactive:**
  - **Short Tap:** Motor turant rukti hai, buzzer sound play hota hai aur expression agle mood par switch ho jata hai.
  - **Long Press (> 1.5 seconds):** Robot 20 second ke liye "Sleep Mode" mein chala jata hai (Zzz animation aur motors poori tarah band).
- **Automatic Mood Cycle:** Normal chalte waqt har 6.5 second baad naya emotion aur sound automatic aate hain.
- **Desktop Wandering Logic:** 80cm x 40cm table safety ke liye short step bursts aur non-blocking timing control.

---

## 🛠️ Hardware Requirements

| Component | Quantity | Specification / Description |
| :--- | :--- | :--- |
| **Microcontroller** | 1 | Arduino Nano (ATmega328P) |
| **Display** | 1 | 0.96" I2C OLED Display (SSD1306, 128x64) |
| **Motor Driver** | 1 | DRV8833 Dual H-Bridge Motor Driver Module |
| **Motors** | 2 | Yellow TT Geared DC Motors + Rubber Wheels |
| **Touch Sensor** | 1 | TTP223 Capacitive Touch Switch Module |
| **Buzzer** | 1 | 5V Active / Passive Piezo Buzzer |
| **Power Booster** | 1 | MT3608 DC-DC Step-Up Boost Converter Module |
| **Battery** | 1 | 3.7V 18650 Li-ion Cell |
| **Capacitor** | 1 | 100µF – 470µF 16V/25V Electrolytic Capacitor (Noise/Brownout Filter) |

---

## 🔌 Pin Connections & Wiring Diagram

### 1. DRV8833 Motor Driver
| DRV8833 Pin | Connection | Note |
| :--- | :--- | :--- |
| **VM** | MT3608 Booster **VOUT+** (7.0V) | Motor power input |
| **GND** | Booster **VOUT-** & Arduino **GND** | Common ground zaroori hai |
| **STBY** | Arduino Nano **5V** | Driver standby pull-high |
| **AIN1** | Arduino Nano **D5** | Left Motor Control (PWM) |
| **AIN2** | Arduino Nano **D6** | Left Motor Control (PWM) |
| **BIN1** | Arduino Nano **D9** | Right Motor Control (PWM) |
| **BIN2** | Arduino Nano **D10** | Right Motor Control (PWM) |
| **AO1 & AO2**| Left Gear Motor Wires | Motor terminals |
| **BO1 & BO2**| Right Gear Motor Wires | Motor terminals |

### 2. 0.96" I2C OLED Display
| OLED Pin | Arduino Nano Pin | Note |
| :--- | :--- | :--- |
| **VCC** | **5V** | Regulated logic power |
| **GND** | **GND** | Ground |
| **SDA** | **A4** | I2C Data Line |
| **SCL** | **A5** | I2C Clock Line |

### 3. TTP223 Touch Sensor
| Sensor Pin | Arduino Nano Pin | Note |
| :--- | :--- | :--- |
| **VCC** | **5V** | Power |
| **GND** | **GND** | Ground |
| **SIG / OUT** | **D2** | Digital Touch Trigger |

### 4. Buzzer
| Buzzer Pin | Arduino Nano Pin | Note |
| :--- | :--- | :--- |
| **Positive (+)** | **D3** | Tone PWM Pin |
| **Negative (-)** | **GND** | Ground |

### 5. Power Distribution & Voltage Step-Up
1. **Battery (3.7V)** $\rightarrow$ MT3608 Booster ke **VIN+** aur **VIN-** par connect karein.
2. Booster ke trimpot ko counter-clockwise ghuma kar output voltage **7.0V** set karein.
3. **Booster VOUT+ (7.0V):**
   - Arduino Nano ke **VIN** pin par.
   - DRV8833 ke **VM** pin par.
4. **Booster VOUT- (GND):**
   - Arduino Nano ke **GND** par.
   - DRV8833 ke **GND** par.
5. **Decoupling Capacitor:** 470µF capacitor ko DRV8833 ke **VM (+)** aur **GND (-)** ke parallel mein lagayein taaki brownout reset na ho.

> ⚠️ **Warning:** Booster ka 7.0V kabhi bhi Arduino ke **5V** pin par mat lagayein. Hamesha **VIN** pin use karein.

---

## 💻 Software Setup

1. **Arduino IDE** open karein.
2. **Library Manager** (`Ctrl + Shift + I` / `Cmd + Shift + I`) open karke yeh libraries install karein:
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
3. Board select karein: **Tools > Board > Arduino AVR Boards > Arduino Nano**.
4. Processor: **ATmega328P** (agar upload error aaye toh **ATmega328P (Old Bootloader)** chunein).
5. Code compile aur upload karein.

---

## ⚙️ Speed Tuning

Agar robot ka wazan kam ya zyada ho toh code ke top section se speed adjust kar sakte hain:

```cpp
int runSpeed  = 125; // Dheere seedha chalne ke liye (0 - 255)
int turnSpeed = 135; // Mudne ki speed
