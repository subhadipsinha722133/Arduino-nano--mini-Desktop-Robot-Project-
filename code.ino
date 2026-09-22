#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pin Connections
const int touchPin   = 2;
const int buzzerPin  = 3;
const int AIN1       = 5;
const int AIN2       = 6;
const int BIN1       = 9;
const int BIN2       = 10;

// ==========================================
// *** SPEED SETTINGS ***
// ==========================================
int runSpeed  = 125; 
int turnSpeed = 135; 

// Mood Variables
int currentMood = 0;
const int totalMoods = 13;

unsigned long lastAutoMoodTime = 0;
const unsigned long autoMoodInterval = 6500; // Har 6.5s mein auto badlega

// Touch / Long Press Timing Variables
bool lastTouchState = LOW;
unsigned long touchStartTime = 0;
bool isLongPressTriggered = false;

// Sleep / Pause States
bool isSleeping = false;
unsigned long sleepStartTime = 0;
const unsigned long sleepDuration = 20000; // 20 Second (20000 ms)

bool isShortTouchPaused = false;
unsigned long shortTouchPauseTimer = 0;

// Movement State Machine
unsigned long moveTimer = 0;
unsigned long moveDuration = 400;

void setup() {
  pinMode(touchPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  stopMotors();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  }

  // Welcome Beep
  tone(buzzerPin, 1500, 80); delay(90);
  tone(buzzerPin, 2000, 100);

  playMoodAndSound(currentMood);
  lastAutoMoodTime = millis();
  pickNextMove();
}

void loop() {
  handleTouchInput();

  // 1. Agar Robot 20 second ke Sleep Mode mein hai
  if (isSleeping) {
    stopMotors();
    // 20 second poore hone par wake up
    if (millis() - sleepStartTime >= sleepDuration) {
      isSleeping = false;
      // Wake up tone
      tone(buzzerPin, 1200, 80); delay(90);
      tone(buzzerPin, 1800, 120);
      playMoodAndSound(0); // Happy face
      lastAutoMoodTime = millis();
      pickNextMove();
    }
    return; // Sleep mode mein baaki movement ya auto-change block rahega
  }

  // 2. Agar Short Press ke baad 1.5 second ka pause hai
  if (isShortTouchPaused) {
    stopMotors();
    if (millis() - shortTouchPauseTimer >= 1500) {
      isShortTouchPaused = false;
      pickNextMove();
    }
    return;
  }

  // 3. Automatic Mood Change (Har 6.5 second baad)
  if (millis() - lastAutoMoodTime >= autoMoodInterval) {
    currentMood = (currentMood + 1) % totalMoods;
    playMoodAndSound(currentMood);
    lastAutoMoodTime = millis();
  }

  // 4. Desktop Roaming
  if (millis() - moveTimer >= moveDuration) {
    pickNextMove();
  }
}

// ==========================================
// TOUCH & LONG PRESS DETECTION
// ==========================================
void handleTouchInput() {
  bool currentTouch = digitalRead(touchPin);

  // Ungli rakhi gayi
  if (currentTouch == HIGH && lastTouchState == LOW) {
    touchStartTime = millis();
    isLongPressTriggered = false;
    stopMotors();
  }

  // Ungli daba kar rakhi hui hai (Long Press Check)
  if (currentTouch == HIGH && !isLongPressTriggered) {
    // Agar 1.5 second (1500ms) se zyada dabakar rakha
    if (millis() - touchStartTime >= 1500) {
      isLongPressTriggered = true;
      isSleeping = true;
      sleepStartTime = millis();

      // Long press confirmation tune (Sleep tune)
      tone(buzzerPin, 800, 100); delay(120);
      tone(buzzerPin, 500, 120); delay(140);
      tone(buzzerPin, 300, 250);

      // Sleepy display show karo
      display.clearDisplay();
      animSleepyZzz();
    }
  }

  // Ungli hata li gayi (Release)
  if (currentTouch == LOW && lastTouchState == HIGH) {
    // Agar yeh long press nahi tha, toh isko normal short click maano
    if (!isLongPressTriggered) {
      stopMotors();
      currentMood = (currentMood + 1) % totalMoods;
      playMoodAndSound(currentMood);

      isShortTouchPaused = true;
      shortTouchPauseTimer = millis();
      lastAutoMoodTime = millis();
    }
  }

  lastTouchState = currentTouch;
}

// ==========================================
// MOVEMENT FUNCTIONS
// ==========================================
void pickNextMove() {
  moveTimer = millis();
  int r = random(0, 5);

  if (r == 0 || r == 1) {
    moveDuration = random(300, 450);
    moveForwardSmooth();
  } else if (r == 2) {
    moveDuration = random(220, 300);
    turnLeftSmooth();
  } else if (r == 3) {
    moveDuration = random(220, 300);
    turnRightSmooth();
  } else {
    moveDuration = random(400, 700);
    stopMotors();
  }
}

void moveForwardSmooth() {
  analogWrite(AIN1, 180); analogWrite(AIN2, 0);
  analogWrite(BIN1, 180); analogWrite(BIN2, 0);
  delay(20);
  analogWrite(AIN1, runSpeed);
  analogWrite(AIN2, 0);
  analogWrite(BIN1, runSpeed);
  analogWrite(BIN2, 0);
}

void turnLeftSmooth() {
  analogWrite(AIN1, 0);
  analogWrite(AIN2, turnSpeed);
  analogWrite(BIN1, turnSpeed);
  analogWrite(BIN2, 0);
}

void turnRightSmooth() {
  analogWrite(AIN1, turnSpeed);
  analogWrite(AIN2, 0);
  analogWrite(BIN1, 0);
  analogWrite(BIN2, turnSpeed);
}

void stopMotors() {
  analogWrite(AIN1, 0);
  analogWrite(AIN2, 0);
  analogWrite(BIN1, 0);
  analogWrite(BIN2, 0);
}

// ==========================================
// DISPLAY & SOUND CONTROLLER
// ==========================================
void playMoodAndSound(int mood) {
  display.clearDisplay();

  switch (mood) {
    case 0: // Happy
      tone(buzzerPin, 1300, 60); delay(70);
      tone(buzzerPin, 1800, 80);
      display.drawCircle(40, 36, 16, SSD1306_WHITE);
      display.fillRect(20, 36, 40, 20, SSD1306_BLACK);
      display.drawCircle(88, 36, 16, SSD1306_WHITE);
      display.fillRect(68, 36, 40, 20, SSD1306_BLACK);
      display.fillCircle(20, 42, 3, SSD1306_WHITE);
      display.fillCircle(108, 42, 3, SSD1306_WHITE);
      break;

    case 1: // Look Around
      tone(buzzerPin, 1600, 50); delay(60);
      tone(buzzerPin, 2100, 60);
      display.fillRoundRect(44, 18, 28, 32, 6, SSD1306_WHITE);
      display.fillRoundRect(88, 18, 28, 32, 6, SSD1306_WHITE);
      break;

    case 2: // Angry
      tone(buzzerPin, 350, 150);
      display.fillRoundRect(26, 20, 28, 28, 4, SSD1306_WHITE);
      display.fillRoundRect(74, 20, 28, 28, 4, SSD1306_WHITE);
      display.fillTriangle(20, 15, 60, 15, 60, 32, SSD1306_BLACK);
      display.fillTriangle(68, 15, 108, 15, 68, 32, SSD1306_BLACK);
      break;

    case 3: // Sad / Cry
      tone(buzzerPin, 800, 80); delay(90);
      tone(buzzerPin, 450, 140);
      display.fillRoundRect(26, 24, 26, 24, 4, SSD1306_WHITE);
      display.fillRoundRect(76, 24, 26, 24, 4, SSD1306_WHITE);
      display.fillTriangle(20, 20, 58, 20, 20, 34, SSD1306_BLACK);
      display.fillTriangle(70, 20, 108, 20, 108, 34, SSD1306_BLACK);
      display.fillCircle(38, 54, 3, SSD1306_WHITE);
      display.fillCircle(88, 54, 3, SSD1306_WHITE);
      break;

    case 4: // Sleepy
      tone(buzzerPin, 300, 120);
      animSleepyZzz();
      break;

    case 5: // Heart / Love
      tone(buzzerPin, 1400, 70); delay(80);
      tone(buzzerPin, 1750, 80); delay(90);
      tone(buzzerPin, 2100, 120);
      display.fillCircle(34, 26, 7, SSD1306_WHITE);
      display.fillCircle(46, 26, 7, SSD1306_WHITE);
      display.fillTriangle(27, 29, 53, 29, 40, 44, SSD1306_WHITE);
      display.fillCircle(82, 26, 7, SSD1306_WHITE);
      display.fillCircle(94, 26, 7, SSD1306_WHITE);
      display.fillTriangle(75, 29, 101, 29, 88, 44, SSD1306_WHITE);
      break;

    case 6: // Shock
      tone(buzzerPin, 2400, 80);
      display.drawCircle(38, 32, 22, SSD1306_WHITE);
      display.fillCircle(38, 32, 6, SSD1306_WHITE);
      display.drawCircle(90, 32, 22, SSD1306_WHITE);
      display.fillCircle(90, 32, 6, SSD1306_WHITE);
      break;

    case 7: // Dizzy
      tone(buzzerPin, 900, 40); delay(50);
      tone(buzzerPin, 600, 40); delay(50);
      tone(buzzerPin, 900, 40);
      display.drawLine(24, 20, 52, 44, SSD1306_WHITE);
      display.drawLine(52, 20, 24, 44, SSD1306_WHITE);
      display.drawLine(76, 20, 104, 44, SSD1306_WHITE);
      display.drawLine(104, 20, 76, 44, SSD1306_WHITE);
      break;

    case 8: // Puppy Cute
      tone(buzzerPin, 1800, 40); delay(50);
      tone(buzzerPin, 2200, 60);
      display.fillCircle(38, 32, 18, SSD1306_WHITE);
      display.fillCircle(90, 32, 18, SSD1306_WHITE);
      display.fillCircle(34, 26, 6, SSD1306_BLACK);
      display.fillCircle(42, 38, 3, SSD1306_BLACK);
      display.fillCircle(86, 26, 6, SSD1306_BLACK);
      display.fillCircle(94, 38, 3, SSD1306_BLACK);
      break;

    case 9: // Wink & Star
      tone(buzzerPin, 1200, 50); delay(60);
      tone(buzzerPin, 1600, 50); delay(60);
      tone(buzzerPin, 2000, 70);
      display.fillRoundRect(78, 16, 26, 32, 6, SSD1306_WHITE);
      display.drawFastHLine(24, 34, 26, SSD1306_WHITE);
      display.drawLine(20, 18, 28, 18, SSD1306_WHITE);
      display.drawLine(24, 14, 24, 22, SSD1306_WHITE);
      break;

    case 10: // Suspicious
      tone(buzzerPin, 500, 60); delay(70);
      tone(buzzerPin, 700, 90);
      display.fillRect(24, 28, 30, 12, SSD1306_WHITE);
      display.fillRect(74, 24, 30, 16, SSD1306_WHITE);
      display.drawFastHLine(72, 18, 34, SSD1306_WHITE);
      break;

    case 11: // Cyber Scan
      tone(buzzerPin, 2100, 30); delay(40);
      tone(buzzerPin, 1900, 30);
      display.drawRoundRect(20, 24, 88, 16, 4, SSD1306_WHITE);
      display.fillRect(56, 26, 16, 12, SSD1306_WHITE);
      break;

    case 12: // Dead KO
      tone(buzzerPin, 700, 60); delay(70);
      tone(buzzerPin, 500, 70); delay(80);
      tone(buzzerPin, 300, 140);
      display.drawLine(28, 24, 48, 40, SSD1306_WHITE);
      display.drawLine(48, 24, 28, 40, SSD1306_WHITE);
      display.drawLine(80, 24, 100, 40, SSD1306_WHITE);
      display.drawLine(100, 24, 80, 40, SSD1306_WHITE);
      break;
  }

  display.display();
}

void animSleepyZzz() {
  display.drawFastHLine(24, 34, 28, SSD1306_WHITE);
  display.drawFastHLine(76, 34, 28, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(95, 12); display.print("z");
  display.setCursor(105, 5); display.print("Z");
  display.display();
}
