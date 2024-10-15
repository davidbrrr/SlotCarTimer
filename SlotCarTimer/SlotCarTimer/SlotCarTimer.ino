// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// pin layout
const int INPUT_PIN = 6;
const int BUZZER_PIN = 5;
const int SEGMENT_LATCH = 3;  //74HC595 pin 12 STCP
const int SEGMENT_CLOCK = 4; //74HC595 pin 11 SHCP
const int SEGMENT_DATA = 2;   //74HC595 pin 14 DS

// constants
const float TRACK_LENGTH = 10.0; //10 meters
const unsigned char table[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71,0x00,0x40,0x80}; //0-9 (0-9), A-F (10-15), off (16), hyphen (17), decimal point (18)
const int digitPins[4] = {A0, A1, A2, A3}; //analog out

// button state
int buttonState; // the current reading from the input pin
int lastButtonState = HIGH;  // the previous reading from the input pin - HIGH = unpressed
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers

// app state
bool first = true;
long start = 0L;
long bestLapTime = 60L * 60L * 1000L; //1 hour

// other
char tempTime[9];
char timeString[16];
char tempSpeed[5];
char speedString[16];
unsigned char tempCurrentTime[6]; //max 99999ms, and an extra for the null terminator

void setup() {
  //set up pins
  pinMode(LED_BUILTIN, OUTPUT); //internal test LED
  pinMode(INPUT_PIN, INPUT_PULLUP); //push button / slot car detection
  pinMode(BUZZER_PIN, OUTPUT); //buzzer
  pinMode(SEGMENT_LATCH, OUTPUT); //LED segment driver
  pinMode(SEGMENT_CLOCK, OUTPUT); //LED segment driver
  pinMode(SEGMENT_DATA, OUTPUT); //LED segment driver
  pinMode(A0, OUTPUT); //LED digit on/off
  pinMode(A1, OUTPUT); //LED digit on/off
  pinMode(A2, OUTPUT); //LED digit on/off
  pinMode(A3, OUTPUT); //LED digit on/off
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // print a message to the LCD.
  lcd.print(" Time: NO TIME");
  lcd.setCursor(0, 1);
  lcd.print("Speed: NO SPEED");

  //debugging
  //Serial.begin(9600);
}

void loop() {
  // turn off test LED
  digitalWrite(LED_BUILTIN, LOW);

  //lap timer
  if(first) DisplayDigits(17,17,17,17); //--.--
  else {
    long currentTime = millis()-start;
    if(currentTime < 10000) { //less than 10 seconds -> show leading 0, single second, then decimals -> 01.23
      DisplayDigits(0,currentTime/1000,currentTime/100%10,currentTime/10%10);
    } else { //more than 10 seconds -> show both digits of the seconds, then decimals -> 12.34
      DisplayDigits(currentTime/10000,currentTime/1000%10,currentTime/100%10,currentTime/10%10);
    }
  }

  // read the state of the switch
  int reading = digitalRead(INPUT_PIN);

  // if the switch changed, due to noise or pressing, reset the debouncing timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  //check debounce
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // if the button state has changed
    if (reading != buttonState) {
      buttonState = reading;

      // lap is finished if state is LOW i.e. pressed
      if (buttonState == LOW) {
        // ignore the first lap, just a warmup
        if(!first) {
          long lapTime = millis()-start;
          if(lapTime < bestLapTime) {
            bestLapTime = lapTime;
            
            //buzzer
            digitalWrite(BUZZER_PIN, HIGH);
            delay(100);
            digitalWrite(BUZZER_PIN, LOW);
          }
          float t = bestLapTime / 1000.0;
          dtostrf(t, 9, 2, tempTime); //9 digits min, 2 dp
          sprintf(timeString, "Time: %ss", tempTime);
          lcd.setCursor(0, 0);
          lcd.print(timeString); 
    
          //speed is distance/time (in m/s)
          float s = (TRACK_LENGTH / (bestLapTime / 1000.0)) * 3.6; //in km/h
          dtostrf(s, 5, 2, tempSpeed); //5 digits min, 2 dp
          sprintf(speedString, "Speed: %skm/h", tempSpeed);
          lcd.setCursor(0, 1);
          lcd.print(speedString);
        } else {
          first = false;
        }
    
        start = millis(); //reset timer
      }
    }
  }

  //save button state
  lastButtonState = reading;
}

/**
 * Provide the digits to display for the lap time.
 * First 2 digits are seconds, others are fractions of a second to 2dp.
 */
void DisplayDigits(unsigned char digit1, unsigned char digit2, unsigned char digit3, unsigned char digit4) {
  DisplayDigit(digit1, 0, false);
  delay(1);
  DisplayDigit(digit2, 1, true); //show the decimal point
  delay(1);
  DisplayDigit(digit3, 2, false);
  delay(1);
  DisplayDigit(digit4, 3, false);
  delay(1);
}

/**
 * Multiplexes the digits.
 */
void DisplayDigit(unsigned char num, int digitDisplay, boolean showDecimalPoint)
{
  //output data
  digitalWrite(SEGMENT_LATCH, LOW);
  shiftOut(SEGMENT_DATA, SEGMENT_CLOCK, MSBFIRST, showDecimalPoint ? table[num]|table[18] :  table[num]);
  digitalWrite(SEGMENT_LATCH, HIGH);

  //turn on digit
  digitalWrite(digitPins[digitDisplay], LOW);

  //turn off the others
  for (int k = 0; k < 4; k++) {
    if(k != digitDisplay) digitalWrite(digitPins[k], HIGH);
  }
}

//  AAA
// F   B
// F   B
// F   B
//  GGG
// E   C
// E   C
// E   C
//  DDD

// .GFEDCBA (convert binary -> hex into table)
