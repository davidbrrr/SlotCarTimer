// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// pin layout
int INPUT_PIN = 6;

// constants
float TRACK_LENGTH = 10.0; //10 meters

// vars
bool pressed = false;
bool first = true;
char tempTime[9];
char timeString[16];
char tempSpeed[5];
char speedString[16];

long start = 0L;
long bestLapTime = 60L * 60L * 1000L; //1 hour

void setup() {
  //set up pins
  pinMode(LED_BUILTIN, OUTPUT); //internal test LED
  pinMode(INPUT_PIN, INPUT_PULLUP); //push button / slot car detection
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // print a message to the LCD.
  lcd.print(" Time: NO TIME");
  lcd.setCursor(0, 1);
  lcd.print("Speed: NO SPEED");
}

void loop() {
  // turn off test LED
  digitalWrite(LED_BUILTIN, LOW);

  //button press
  if (digitalRead(INPUT_PIN) == LOW && !pressed)
  {
    pressed = true;
    if(!first) {
      long lapTime = millis()-start;
      if(lapTime < bestLapTime) bestLapTime = lapTime;
      float t = bestLapTime / 1000.0;
      dtostrf(t, 9, 3, tempTime); //9 digits min, 2 dp
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

  //button release
  if (digitalRead(INPUT_PIN) == HIGH && pressed)
  {
    pressed = false;
  }
}
