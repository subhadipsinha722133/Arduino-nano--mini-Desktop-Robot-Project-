#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Reset pin (-1 sharing Arduino reset)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pin Definitions
const int touchPin = 2;
const int buzzerPin = 3;

// DRV8833 Motor Driver Pins
const int AIN1 = 5;  // Left Motor PWM
const int AIN2 = 6;
const int BIN1 = 9;  // Right Motor PWM
const int BIN2 = 10;

// Motor Speed Constants (0 - 255)
const int MOVE_SPEED = 215;      // Strong torque for table roaming
const int TURN_SPEED = 200;      // In-place turning speed
const int KICKSTART_SPEED = 255;  // Instant torque boost to overcome load/friction

// State Variables
int currentMood = 0;
const int TOTAL_MOODS = 14;

unsigned long lastMoveTime = 0;
int moveState = 0; 
unsigned long stateDuration = 0;

void setup() {
  pinMode(touchPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  stopMotors();

  // OLED initialize (Try standard 0x3C, fallback to 0x3D)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  }

  display.clearDisplay();
  playStartupSound();
  playMoodAnimation(0);
}

void loop() {
  // Touch Sensor Interaction
  if (digitalRead(touchPin) == HIGH) {
    stopMotors(); // Turant robot ko roko

    // Agla mood chunen
    currentMood = (currentMood + 1) % TOTAL_MOODS;
    
    // Mood-specific sound aur animated expression chalao
    playMoodSound(currentMood);
    playMoodAnimation(currentMood);

    // Touch debounce aur wait period
    unsigned long waitStart = millis();
    while (millis() - waitStart < 1400) {
      if (digitalRead(touchPin) == HIGH) {
        delay(120);
        currentMood = (currentMood + 1) % TOTAL_MOODS;
        playMoodSound(currentMood);
        playMoodAnimation(currentMood);
        waitStart = millis();
      }
      delay(20);
    }

    lastMoveTime = millis();
    moveState = 0;
  } else {
    // 80cm x 40cm table ke anusaar safe chhote steps mein roam karna
    desktopSafeRoam();
  }
}

// =========================================================================
// 1. MOTOR FUNCTIONS (WITH INSTANT TORQUE KICKSTART)
// =========================================================================

void stopMotors() {
  analogWrite(AIN1, 0);
  analogWrite(AIN2, 0);
  analogWrite(BIN1, 0);
  analogWrite(BIN2, 0);
}

// Static friction todne ke liye 60ms ka full power burst
void kickstartMotors(bool leftFwd, bool rightFwd) {
  analogWrite(AIN1, leftFwd ? KICKSTART_SPEED : 0);
  analogWrite(AIN2, leftFwd ? 0 : KICKSTART_SPEED);
  analogWrite(BIN1, rightFwd ? KICKSTART_SPEED : 0);
  analogWrite(BIN2, rightFwd ? 0 : KICKSTART_SPEED);
  delay(60);
}

void moveForward() {
  kickstartMotors(true, true);
  analogWrite(AIN1, MOVE_SPEED);
  analogWrite(AIN2, 0);
  analogWrite(BIN1, MOVE_SPEED);
  analogWrite(BIN2, 0);
}

void turnLeftInPlace() {
  kickstartMotors(false, true);
  analogWrite(AIN1, 0);
  analogWrite(AIN2, TURN_SPEED);
  analogWrite(BIN1, TURN_SPEED);
  analogWrite(BIN2, 0);
}

void turnRightInPlace() {
  kickstartMotors(true, false);
  analogWrite(AIN1, TURN_SPEED);
  analogWrite(AIN2, 0);
  analogWrite(BIN1, 0);
  analogWrite(BIN2, TURN_SPEED);
}

// =========================================================================
// 2. DESKTOP SAFE ROAMING (80cm x 40cm Safe Short Bursts)
// =========================================================================

void desktopSafeRoam() {
  unsigned long now = millis();

  if (now - lastMoveTime >= stateDuration) {
    lastMoveTime = now;
    moveState = random(0, 5);

    switch (moveState) {
      case 0:
      case 1:
        // Chhota aage ka kadam (sirf 350-500ms = ~6-8 cm)
        drawNormalEyes();
        moveForward();
        stateDuration = random(350, 500);
        break;

      case 2:
        // Left Turn on spot
        drawLookLeft();
        turnLeftInPlace();
        stateDuration = random(250, 420);
        break;

      case 3:
        // Right Turn on spot
        drawLookRight();
        turnRightInPlace();
        stateDuration = random(250, 420);
        break;

      case 4:
      default:
        // Pause and look around safely
        stopMotors();
        if (random(0, 2) == 0) drawBlink();
        else drawLookUp();
        stateDuration = random(900, 1500);
        break;
    }
  }
}

// =========================================================================
// 3. SOUND EFFECTS FOR EACH MOOD
// =========================================================================

void playStartupSound() {
  tone(buzzerPin, 900, 60);  delay(70);
  tone(buzzerPin, 1300, 80); delay(90);
  tone(buzzerPin, 2100, 120);
}

void playMoodSound(int mood) {
  switch (mood) {
    case 1: // Happy Laugh / Chirp
      tone(buzzerPin, 1500, 50); delay(60);
      tone(buzzerPin, 1900, 60); delay(70);
      tone(buzzerPin, 2400, 100);
      break;

    case 2: // Angry Low Growl
      tone(buzzerPin, 320, 140); delay(150);
      tone(buzzerPin, 240, 200);
      break;

    case 3: // Sad Whimper (Descending pitch)
      tone(buzzerPin, 1200, 90); delay(100);
      tone(buzzerPin, 900, 110); delay(120);
      tone(buzzerPin, 650, 180);
      break;

    case 4: // Sleepy Snore / Yawn
      tone(buzzerPin, 400, 150); delay(180);
      tone(buzzerPin, 300, 250);
      break;

    case 5: // Heart / Love melody
      tone(buzzerPin, 1300, 70); delay(80);
      tone(buzzerPin, 1750, 80); delay(90);
      tone(buzzerPin, 2200, 140);
      break;

    case 6: // Surprised / Shocked!
      tone(buzzerPin, 2600, 150); delay(80);
      tone(buzzerPin, 3000, 120);
      break;

    case 7: // Dizzy / Wobble
      for (int f = 600; f < 1400; f += 200) {
        tone(buzzerPin, f, 30);
        delay(35);
      }
      break;

    case 8: // Puppy Cute
      tone(buzzerPin, 1800, 80); delay(90);
      tone(buzzerPin, 2200, 100);
      break;

    case 9: // Wink Click
      tone(buzzerPin, 2000, 50); delay(60);
      tone(buzzerPin, 1600, 70);
      break;

    case 10: // Suspicious / Hmm
      tone(buzzerPin, 700, 120); delay(130);
      tone(buzzerPin, 750, 150);
      break;

    case 11: // Sci-Fi Radar / Scanning
      for (int i = 0; i < 3; i++) {
        tone(buzzerPin, 1800 + (i * 300), 40);
        delay(60);
      }
      break;

    case 12: // Bored Eye Roll
      tone(buzzerPin, 800, 80); delay(90);
      tone(buzzerPin, 600, 120);
      break;

    case 13: // Dead / Flatline
      tone(buzzerPin, 450, 350);
      break;

    default: // Normal Beep
      tone(buzzerPin, 1400, 60); delay(70);
      tone(buzzerPin, 1800, 80);
      break;
  }
}

// =========================================================================
// 4. ANIMATION CONTROLLER (14 MOODS WITH DYNAMIC FRAMES)
// =========================================================================

void playMoodAnimation(int mood) {
  switch (mood) {
    case 0:  animNormalIdle(); break;
    case 1:  animHappyBouncing(); break;
    case 2:  animAngryShaking(); break;
    case 3:  animSadCrying(); break;
    case 4:  animSleepyZzz(); break;
    case 5:  animPulsingHeart(); break;
    case 6:  animSurprisedShock(); break;
    case 7:  animDizzySpins(); break;
    case 8:  animPuppyCute(); break;
    case 9:  animWinkWithSparkle(); break;
    case 10: animSuspiciousSquint(); break;
    case 11: animSciFiScanner(); break;
    case 12: animBoredEyeRoll(); break;
    case 13: animDeadKnockout(); break;
    default: animNormalIdle(); break;
  }
}

// -------------------------------------------------------------------------
// MOOD 0: Normal Idle Eye
// -------------------------------------------------------------------------
void drawNormalEyes() {
  display.clearDisplay();
  display.fillRoundRect(26, 14, 30, 38, 8, SSD1306_WHITE);
  display.fillRoundRect(72, 14, 30, 38, 8, SSD1306_WHITE);
  display.display();
}

void drawBlink() {
  display.clearDisplay();
  display.fillRoundRect(26, 31, 30, 5, 2, SSD1306_WHITE);
  display.fillRoundRect(72, 31, 30, 5, 2, SSD1306_WHITE);
  display.display();
  delay(120);
  drawNormalEyes();
}

void drawLookLeft() {
  display.clearDisplay();
  display.fillRoundRect(14, 14, 30, 38, 8, SSD1306_WHITE);
  display.fillRoundRect(60, 14, 30, 38, 8, SSD1306_WHITE);
  display.display();
}

void drawLookRight() {
  display.clearDisplay();
  display.fillRoundRect(38, 14, 30, 38, 8, SSD1306_WHITE);
  display.fillRoundRect(84, 14, 30, 38, 8, SSD1306_WHITE);
  display.display();
}

void drawLookUp() {
  display.clearDisplay();
  display.fillRoundRect(26, 6, 30, 36, 8, SSD1306_WHITE);
  display.fillRoundRect(72, 6, 30, 36, 8, SSD1306_WHITE);
  display.display();
}

void animNormalIdle() {
  drawNormalEyes();
  delay(350);
  drawBlink();
  delay(200);
  drawNormalEyes();
}

// -------------------------------------------------------------------------
// MOOD 1: Happy Bouncing Laughing Eyes (^ ^) + Blushing Cheeks
// -------------------------------------------------------------------------
void animHappyBouncing() {
  for (int bounce = 0; bounce < 3; bounce++) {
    int yOffset = (bounce % 2 == 0) ? 0 : 4;

    display.clearDisplay();
    // Left eye curve
    display.fillCircle(40, 30 + yOffset, 18, SSD1306_WHITE);
    display.fillCircle(40, 36 + yOffset, 18, SSD1306_BLACK);

    // Right eye curve
    display.fillCircle(88, 30 + yOffset, 18, SSD1306_WHITE);
    display.fillCircle(88, 36 + yOffset, 18, SSD1306_BLACK);

    // Blushing cheeks (dots)
    display.fillCircle(20, 48 + yOffset, 5, SSD1306_WHITE);
    display.fillCircle(108, 48 + yOffset, 5, SSD1306_WHITE);

    display.display();
    delay(160);
  }
}

// -------------------------------------------------------------------------
// MOOD 2: Angry / Furrowed Eyebrows with Red-Hot Tremble
// -------------------------------------------------------------------------
void animAngryShaking() {
  for (int shake = 0; shake < 4; shake++) {
    int xOffset = (shake % 2 == 0) ? -2 : 2;

    display.clearDisplay();
    // Left eye
    display.fillRoundRect(26 + xOffset, 18, 30, 34, 6, SSD1306_WHITE);
    display.fillTriangle(20 + xOffset, 10, 58 + xOffset, 30, 20 + xOffset, 30, SSD1306_BLACK);

    // Right eye
    display.fillRoundRect(72 + xOffset, 18, 30, 34, 6, SSD1306_WHITE);
    display.fillTriangle(108 + xOffset, 10, 70 + xOffset, 30, 108 + xOffset, 30, SSD1306_BLACK);

    // Angry brow tick marks
    display.drawLine(60, 10, 68, 10, SSD1306_WHITE);
    display.drawLine(64, 6, 64, 14, SSD1306_WHITE);

    display.display();
    delay(120);
  }
}

// -------------------------------------------------------------------------
// MOOD 3: Sad / Droopy Eyes with Animated Falling Tears
// -------------------------------------------------------------------------
void animSadCrying() {
  for (int tearY = 38; tearY < 60; tearY += 4) {
    display.clearDisplay();

    // Droopy Left Eye
    display.fillRoundRect(26, 18, 30, 34, 6, SSD1306_WHITE);
    display.fillTriangle(24, 14, 58, 14, 24, 28, SSD1306_BLACK);

    // Droopy Right Eye
    display.fillRoundRect(72, 18, 30, 34, 6, SSD1306_WHITE);
    display.fillTriangle(104, 14, 70, 14, 104, 28, SSD1306_BLACK);

    // Tear drops falling down
    display.fillCircle(38, tearY, 3, SSD1306_WHITE);
    display.fillTriangle(38, tearY - 5, 35, tearY, 41, tearY, SSD1306_WHITE);

    display.fillCircle(90, tearY, 3, SSD1306_WHITE);
    display.fillTriangle(90, tearY - 5, 87, tearY, 93, tearY, SSD1306_WHITE);

    display.display();
    delay(100);
  }
}

// -------------------------------------------------------------------------
// MOOD 4: Sleepy / Floating "Zzz"
// -------------------------------------------------------------------------
void animSleepyZzz() {
  for (int frame = 0; frame < 3; frame++) {
    display.clearDisplay();

    // Closed sleepy slit eyes
    display.fillRoundRect(24, 35, 32, 6, 2, SSD1306_WHITE);
    display.fillRoundRect(72, 35, 32, 6, 2, SSD1306_WHITE);

    // Floating Z z z
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(95, 26 - (frame * 4));
    display.print("z");

    display.setTextSize(2);
    display.setCursor(104, 16 - (frame * 4));
    display.print("Z");

    display.display();
    delay(250);
  }
}

// -------------------------------------------------------------------------
// MOOD 5: Love / Beating Pulsing Heart Eyes
// -------------------------------------------------------------------------
void drawOneHeart(int cx, int cy, int size) {
  int r = size / 2;
  display.fillCircle(cx - r, cy, r, SSD1306_WHITE);
  display.fillCircle(cx + r, cy, r, SSD1306_WHITE);
  display.fillTriangle(cx - size, cy, cx + size, cy, cx, cy + size + 2, SSD1306_WHITE);
}

void animPulsingHeart() {
  for (int p = 0; p < 3; p++) {
    // Large Heart Frame
    display.clearDisplay();
    drawOneHeart(40, 26, 12);
    drawOneHeart(88, 26, 12);
    // Sparkle
    display.drawPixel(64, 18, SSD1306_WHITE);
    display.drawLine(62, 18, 66, 18, SSD1306_WHITE);
    display.drawLine(64, 16, 64, 20, SSD1306_WHITE);
    display.display();
    delay(200);

    // Small Heart Frame
    display.clearDisplay();
    drawOneHeart(40, 28, 8);
    drawOneHeart(88, 28, 8);
    display.display();
    delay(150);
  }
}

// -------------------------------------------------------------------------
// MOOD 6: Surprised / Wide Shock Eyes with Shrinking Pupils & Exclamation
// -------------------------------------------------------------------------
void animSurprisedShock() {
  for (int pupilR = 10; pupilR >= 5; pupilR -= 2) {
    display.clearDisplay();
    // Giant white sclera
    display.fillCircle(38, 32, 24, SSD1306_WHITE);
    display.fillCircle(90, 32, 24, SSD1306_WHITE);

    // Black shrinking iris
    display.fillCircle(38, 32, pupilR, SSD1306_BLACK);
    display.fillCircle(90, 32, pupilR, SSD1306_BLACK);

    // Exclamation mark in middle
    display.fillRect(63, 16, 3, 14, SSD1306_WHITE);
    display.fillRect(63, 34, 3, 3, SSD1306_WHITE);

    display.display();
    delay(120);
  }
  delay(400);
}

// -------------------------------------------------------------------------
// MOOD 7: Dizzy / Spinning Spiral Pupils
// -------------------------------------------------------------------------
void animDizzySpins() {
  for (int step = 0; step < 4; step++) {
    display.clearDisplay();
    // Left eye spinning cross
    int lx = 40, ly = 32;
    int rx = 88, ry = 32;
    int s = 14;

    if (step % 2 == 0) {
      display.drawLine(lx - s, ly - s, lx + s, ly + s, SSD1306_WHITE);
      display.drawLine(lx - s, ly + s, lx + s, ly - s, SSD1306_WHITE);
      display.drawLine(rx - s, ry - s, rx + s, ry + s, SSD1306_WHITE);
      display.drawLine(rx - s, ry + s, rx + s, ry - s, SSD1306_WHITE);
    } else {
      display.drawLine(lx, ly - s, lx, ly + s, SSD1306_WHITE);
      display.drawLine(lx - s, ly, lx + s, ly, SSD1306_WHITE);
      display.drawLine(rx, ry - s, rx, ry + s, SSD1306_WHITE);
      display.drawLine(rx - s, ry, rx + s, ry, SSD1306_WHITE);
    }

    display.display();
    delay(150);
  }
}

// -------------------------------------------------------------------------
// MOOD 8: Cute Puppy Eyes (Large glossy black with dual reflections)
// -------------------------------------------------------------------------
void animPuppyCute() {
  display.clearDisplay();
  // Big round bases
  display.fillRoundRect(22, 12, 38, 42, 14, SSD1306_WHITE);
  display.fillRoundRect(68, 12, 38, 42, 14, SSD1306_WHITE);

  // Big black inside
  display.fillCircle(41, 33, 15, SSD1306_BLACK);
  display.fillCircle(87, 33, 15, SSD1306_BLACK);

  // Big glossy light reflection
  display.fillCircle(36, 27, 6, SSD1306_WHITE);
  display.fillCircle(82, 27, 6, SSD1306_WHITE);

  // Small second light reflection
  display.fillCircle(46, 38, 2, SSD1306_WHITE);
  display.fillCircle(92, 38, 2, SSD1306_WHITE);

  display.display();
  delay(800);
}

// -------------------------------------------------------------------------
// MOOD 9: Winking Eye with 4-Point Star Sparkle
// -------------------------------------------------------------------------
void animWinkWithSparkle() {
  display.clearDisplay();
  // Left eye open
  display.fillRoundRect(26, 14, 30, 38, 8, SSD1306_WHITE);

  // Right eye winking curve
  display.fillCircle(88, 30, 16, SSD1306_WHITE);
  display.fillCircle(88, 35, 16, SSD1306_BLACK);

  // Sparkle next to wink
  int sx = 110, sy = 24;
  display.drawLine(sx - 7, sy, sx + 7, sy, SSD1306_WHITE);
  display.drawLine(sx, sy - 7, sx, sy + 7, SSD1306_WHITE);
  display.fillCircle(sx, sy, 2, SSD1306_WHITE);

  display.display();
  delay(700);
}

// -------------------------------------------------------------------------
// MOOD 10: Suspicious Squint (One eyebrow raised, narrow slit)
// -------------------------------------------------------------------------
void animSuspiciousSquint() {
  display.clearDisplay();
  // Left narrow squint
  display.fillRoundRect(26, 28, 30, 12, 3, SSD1306_WHITE);

  // Right raised inquisitive eye
  display.fillRoundRect(72, 18, 30, 32, 6, SSD1306_WHITE);

  // Raised eyebrow line over right eye
  display.drawLine(70, 10, 104, 15, SSD1306_WHITE);
  display.drawLine(70, 11, 104, 16, SSD1306_WHITE);

  display.display();
  delay(800);
}

// -------------------------------------------------------------------------
// MOOD 11: Sci-Fi Futuristic Scanning Visor
// -------------------------------------------------------------------------
void animSciFiScanner() {
  for (int scanX = 16; scanX <= 112; scanX += 12) {
    display.clearDisplay();

    // Robot eye outline boxes
    display.drawRoundRect(24, 16, 34, 32, 4, SSD1306_WHITE);
    display.drawRoundRect(70, 16, 34, 32, 4, SSD1306_WHITE);

    // Laser beam vertical sweep
    display.fillRect(scanX, 10, 4, 44, SSD1306_WHITE);

    display.display();
    delay(50);
  }
}

// -------------------------------------------------------------------------
// MOOD 12: Bored 360-Degree Eye Roll
// -------------------------------------------------------------------------
void animBoredEyeRoll() {
  int positions[5][2] = {
    {0, -8},  // Up
    {8, 0},   // Right
    {0, 8},   // Down
    {-8, 0},  // Left
    {0, 0}    // Center
  };

  for (int i = 0; i < 5; i++) {
    display.clearDisplay();
    // Sclera
    display.fillRoundRect(26, 16, 30, 34, 8, SSD1306_WHITE);
    display.fillRoundRect(72, 16, 30, 34, 8, SSD1306_WHITE);

    // Moving black pupils
    int px = positions[i][0];
    int py = positions[i][1];
    display.fillCircle(41 + px, 33 + py, 7, SSD1306_BLACK);
    display.fillCircle(87 + px, 33 + py, 7, SSD1306_BLACK);

    display.display();
    delay(130);
  }
}

// -------------------------------------------------------------------------
// MOOD 13: Dead / Knocked Out (X X Eyes)
// -------------------------------------------------------------------------
void animDeadKnockout() {
  display.clearDisplay();

  // Left X
  display.drawLine(28, 20, 50, 44, SSD1306_WHITE);
  display.drawLine(29, 20, 51, 44, SSD1306_WHITE);
  display.drawLine(28, 44, 50, 20, SSD1306_WHITE);
  display.drawLine(29, 44, 51, 20, SSD1306_WHITE);

  // Right X
  display.drawLine(78, 20, 100, 44, SSD1306_WHITE);
  display.drawLine(79, 20, 101, 44, SSD1306_WHITE);
  display.drawLine(78, 44, 100, 20, SSD1306_WHITE);
  display.drawLine(79, 44, 101, 20, SSD1306_WHITE);

  // Tongue/wavy mouth
  display.drawLine(56, 52, 72, 52, SSD1306_WHITE);

  display.display();
  delay(800);
}
