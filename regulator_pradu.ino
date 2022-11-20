/*********************************************************************

   Current regulator v.0.1

   Current regulator in version pre-alpha. In the future i want 
   to make voltage and power regulator, charger Pb, Li-ion, limiter 
   chanrging, desulphation and regeneration Pb aku, tester any type
   of aku
   
**********************************************************************
 *
 *  This program is working witch electronic adapter based on opamp 
 *  LM358N. His job is amplify signal of electrical shunt wchich is
 *  built with 6x 0,03Ohm resistors connected parallel.
 *  I used both available amplifier in LM358N. Both has diffrent 
 *  degrees of amplification. One for low cuffent flow, second 
 *  for bigger one up to 20A.
 *  
 *  I don't make any scheme of this becouse "Well, i will remember".
 *  and half a year later I'm fully of hope that will works without 
 *  any eksplosion.
 *  
**********************************************************************
   
   To do list:
   -increase fq od PWM
   -increase precision current regulation 
   -work time limiter
   -ading kalibration function
   -adding settings:
      -reading precision
      -settings precision 
   -reading temperature of battery and radiator of regulator
   -logging data on SD card or something like this
   
*********************************************************************/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(13, 11, 6, 5, 4);

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 6 - Data/Command select (D/C)
// pin 5 - LCD chip select (CS)
// pin 4 - LCD reset (RST)
// Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!



const byte clk = 2;             // Don't rember what is it
const byte dt = 3;              // same, maybe i can remember later
const byte sw = 8;              // same as up
const int lpAnalPin = A0;       // Less precision analog Pin 
const int mpAnalPin = A1;       // More precision analog Pin
const byte key = 9;             

bool backlight = 0;             // Variable for backlight in display, probably it's connected permanently
bool bt = 1;                    

byte menuItem = 1;              // This variable allow change row in menu
byte page = 1;                  // Allow to change page of menu if it doesn't fit on one
byte contrast = 50;             // Change dosplay contrast. It doesn't work, but without display doesn't display

int counter = 1;                // It's probably counter for encoder
int impulse = 1;                // It's for correct service of encoder in menu
int setAmpDt = 0;               // It could be anything

unsigned long mTime = 0;        // Variable for timers and interrupts
unsigned long time = 0;         // Same up

void setup() {
  Serial.begin(9600);

  pinMode(clk, INPUT);
  pinMode(dt, INPUT);
  pinMode(lpAnalPin, INPUT);
  pinMode(mpAnalPin, INPUT);
  pinMode(sw, INPUT_PULLUP);
  pinMode(key, OUTPUT);
  analogWrite(key, 255);


  analogReference(INTERNAL);

  attachInterrupt(0, blinkA, LOW);
  attachInterrupt(1, blinkB, LOW);

  time = millis();

  Serial.println("Regulator pradu!");

  display.begin();
  // init done
  display.setRotation(2); // set 180* incwerted display

  // you can change the contrast around to adapt the display
  // for the best viewing!
  // but for some reason it doesn't work
  display.setContrast(contrast);
  display.clearDisplay();

//  text display tests
//  display.setTextSize(1);
//  display.setTextColor(BLACK);
//  display.setCursor(20, 15);
//  display.println("Regulator");
//  display.setCursor(20, 23);
//  display.println("pradu");
//  display.display();
//  delay(500);

  drawMenu();

}

// Funktions blinkA and blinkB are for debouncing and add and substract
// pulses from encoder 
void blinkA() {
  if ((millis() - time) > 5)
    counter++;
  time = millis();
}

void blinkB() {
  if ((millis() - time) > 5)
    counter--;
  time = millis();
}

// Something didn't work without this and this was something with encoder
// so i write it here and sometimes use it.
int isChange() {
  if (impulse != counter) {
    return true;
  }
  else {
    return false;
  }
}

// This funktion distinguishes directions rotary encoder
int upDown(int var) {
  if (impulse > counter) {
    impulse = counter;
    var--;
  }
  else if (impulse < counter) {
    impulse = counter;
    var++;
  }
  Serial.print("Impulse: ");
  Serial.println(impulse);
  Serial.print("Counter: ");
  Serial.println(counter);
  Serial.print("Var: ");
  Serial.println(var);
  return var;
}

int checkIfRelased() {
  while (true) {
    if (digitalRead(sw) == 1) {
      break;
    }
  }
}

int measureTime() {
  if (millis() >= mTime) {
    mTime = mTime + 1000UL;
    return true;
  }
  else {
    return false;
  }
}

int confirm(bool var = 0) {
  bool c = 0;

  while (true) {
    if (isChange() == true) {
      c = upDown(c);
    }
    display.setTextSize(1);
    display.fillRect(5, 12, 70, 28, WHITE);
    display.drawRect(7, 12, 70, 28, BLACK);
    display.setCursor(15, 15);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    display.print("Na pewno?");
    display.drawFastHLine(10, 24, 64, BLACK);
    display.setCursor(20, 27);
    if (c == 0) {
      display.setTextColor(WHITE, BLACK);
    }
    else {
      display.setTextColor(BLACK, WHITE);
    }
    display.print("Nie");
    display.setCursor(50, 27);
    if (c == 1) {
      display.setTextColor(WHITE, BLACK);
    }
    else {
      display.setTextColor(BLACK, WHITE);
    }
    display.print("Tak");
    display.display();

    if (c == 0 and digitalRead(sw) == 0) {
      checkIfRelased();
      return var = true;
    }
    else if (c != 0 and digitalRead(sw) == 0) {
      checkIfRelased();
      c = 0;
      return var = false;
    }
  }
}

void loop() {
  static bool chec = 0;
  chec = isChange();
  if (chec == true) {
    menuItem = upDown(menuItem);
    if (menuItem == 0 or menuItem >= 3) {
      menuItem = 1;
    }
    drawMenu();
    chec = 0;
  }

  if (digitalRead(sw) == 0) {
    chooseItem();
    drawMenu();
  }
}

void chooseItem() {
  Serial.println("Chose item");
  //  static bool lastbt = 1;
  checkIfRelased();
  switch (menuItem) {
    case 1:
      charge();
      break;
    case 2:
      settings();
      break;
  }
}

void settings() {
  display.clearDisplay();
  display.print("Nie wiem co robie");
  display.display();
  delay(5000);
}

void charge() {
  bool a = 1;
  byte sTime = 10;
  byte s = 0, m = 0, h = 0;
  byte valKey = 255;
  byte setAmpAn = 0;
  int readAnal = 0;                 // analogRead(lpAnalPin);
  float rlAmp  = 0;                 //readAnal * (10 / 1023);

  setAmpAn = setCharge(setAmpAn);
//  Serial.println("setAmpDt i setAmpAn ");
//  Serial.print(setAmpDt);
//  Serial.print(", ");
//  Serial.println(setAmpAn);
  mTime = millis();
//  Serial.println(mTime);

  while (a) {
    if (measureTime() == true) {
      s++;
      if (s == 60) {
        m++;
        s = 0;
      }
      if (m == 60) {
        h++;
        m = 0;
      }
      drawCharge(rlAmp, setAmpAn, s, m, h);
    }
//    Serial.println(micros());
    readAnal = analogRead(lpAnalPin);
//    Serial.println(readAnal);
    if (readAnal > setAmpDt){
      valKey++;
      analogWrite(key, valKey);
      if(valKey == 255){
        valKey--;
      }
    }
    else{
      valKey--;
      analogWrite(key, valKey);
      if(valKey == 0){
        valKey++;
      }
    }
//    Serial.println(millis());
//    Serial.println(valKey);
    
    if (readAnal <= 200) {
      readAnal = analogRead(mpAnalPin);
      rlAmp = readAnal * (2.0 / 1023.0);
      }
    else{
      rlAmp = readAnal * (10.0 / 1023.0);
    }
//    Serial.println(micros());
//    Serial.println(rlAmp);
    

    if (digitalRead(sw) == 0) {
      checkIfRelased();
      a = confirm(a);
    }
//    Serial.println(micros());
  }

//  Serial.print("Set value[A]: ");
//  Serial.println(setAmpAn);
//  Serial.print("Real value: ");
//  Serial.println( readAnal);
//  Serial.print("Real value[A]: ");
//  Serial.println(rlAmp);
//  Serial.println(millis());

}

void drawCharge(float rlAmp, byte setAmpAn, byte s, byte m, byte h) {
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(7, 0);
  display.print("Ladowarka Pb");
  display.drawFastHLine(0, 8, 83, BLACK);
  display.setCursor(0, 10);
  display.print("Czas ladowania");
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.print(h);
  display.print(":");
  display.print(m);
  display.print(":");
  display.print(s);
  display.print("(10h)");
  display.setCursor(0, 29);
  display.setTextSize(2);
  display.print(rlAmp);
  display.setTextSize(1);
  display.print("A");
  display.drawFastHLine(63, 37, 20, BLACK);
  display.drawFastVLine(63, 37, 10, BLACK);
  display.setCursor(65, 39);
  display.print(setAmpAn);
  display.print("A");

  display.display();
}

int setCharge(byte setAmpAn) {
  bool a = 1;
  bool c = 0;
//float setAmpDt = 0;
  float coef = 0.00977;

  while (a == 1) {
    drawSetAmp(setAmpAn);
    setAmpAn = upDown(setAmpAn);
    if (setAmpAn > 10) {
      setAmpAn = 10;
    }
    if (setAmpAn < 0) {
      setAmpAn = 0;
    }
    if (digitalRead(sw) == 0) {
      checkIfRelased();
      setAmpDt = float(setAmpDt);
      setAmpDt = setAmpAn / coef;
      setAmpDt = int(setAmpDt);
      a = confirm(a);
    }
  }
  
  Serial.print(setAmpAn);
  Serial.print(" ");
  Serial.println(setAmpDt);
  
  return setAmpAn;
}

void drawSetAmp(byte setAmpAn) {
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(7, 0);
  display.print("Ladowarka Pb");
  display.drawFastHLine(0, 10, 83, BLACK);
  display.setCursor(5, 15);
  display.print("Nastaw prad");
  display.setTextSize(2);
  display.setCursor(5, 25);
  display.print(setAmpAn);
  display.setTextSize(1);
  display.print("(max 10A)");

  display.setTextSize(2);
  display.display();
}

void drawMenu() {

  switch (page) {
    case 1:
      display.setTextSize(1);
      display.clearDisplay();
      display.setTextColor(BLACK, WHITE);
      display.setCursor(30, 0);
      display.print("Menu");
      display.drawFastHLine(0, 10, 83, BLACK);
      display.setCursor(0, 15);

      if (menuItem == 1) {
        display.setTextColor(WHITE, BLACK);
      }
      else {
        display.setTextColor(BLACK, WHITE);
      }

      display.print(">Ladowarka Pb");
      display.setCursor(0, 25);

      if (menuItem == 2) {
        display.setTextColor(WHITE, BLACK);
      }
      else {
        display.setTextColor(BLACK, WHITE);
      }

      display.print(">Ustawienia");

      display.display();
      break;
  }
}
