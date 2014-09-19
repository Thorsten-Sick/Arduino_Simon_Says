
/* Some Simon says style electronics game based on a Arduino. The special trick is that there is the main Arduino controller (with LCD) and 3 Boxes where buttons have to be pressed.
Those boxes are connected through ethernet cable (no ethernet protocol!) with up to 20m distance.

*/

// include the library code:
#include <LiquidCrystal.h>
// SmartMaker special
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// global operation mode
const String disabled = String("disabled");
const String enabled = String("enabled");
const String broken = String("broken");
String operationMode = enabled;

int taskCounter = 0;
int brokenCounter = 0;
boolean gameFinished = false;

const unsigned char Box1_Button_gruen = 12; //gelb
const unsigned char Box1_Button_blau = 11;
const unsigned char Box1_Schloss = 10;
const unsigned char Box1_LED = 13;
const unsigned char Box2_Button_gruen = 8;
const unsigned char Box2_Regler = A5;
const unsigned char Box2_Schloss = A4;
const unsigned char Box2_LED = 9;
const unsigned char Box3_Button_gruen = 1;
const unsigned char Box3_Button_blau = A1; // gelb
const unsigned char Box3_Schloss = A0;
const unsigned char Box3_LED = A3;

String inputString = "";         // a string to hold incoming data

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Simon says:");
  
  pinMode(Box1_Button_gruen, INPUT_PULLUP);
  pinMode(Box1_Button_blau, INPUT_PULLUP);
  pinMode(Box1_Schloss, INPUT_PULLUP);
  pinMode(Box1_LED, OUTPUT);
  pinMode(Box2_Button_gruen, INPUT_PULLUP); 
  pinMode(Box2_Regler, INPUT_PULLUP);
  pinMode(Box2_Schloss, INPUT_PULLUP);
  pinMode(Box2_LED, OUTPUT);
  pinMode(Box3_Button_gruen, INPUT_PULLUP);
  pinMode(Box3_Button_blau, INPUT_PULLUP);
  pinMode(Box3_Schloss, INPUT_PULLUP);
  pinMode(Box3_LED, OUTPUT);
  
  Serial.begin(9600); 

  inputString.reserve(200);

  randomSeed(analogRead(0));

  Serial.println ("event_booted");
  Serial.println ("event_gamestart");

}

void loop() {
  
  taskCounter++;
  
  if (taskCounter == 2000)  // wrap around every 10 Seconds
  {
    taskCounter = 0;
  }
  
  if ((taskCounter % 1000) == 0) // 5000ms Tick
  {
    Serial.print("status_");
    Serial.println (operationMode);
  }

  if (operationMode == enabled)
  {

    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    char buffer[7];
    sprintf(buffer,"%4i",analogRead(Box2_Regler));
    lcd.print(buffer);
    
    digitalWrite(Box1_LED, digitalRead(Box3_Schloss));
    digitalWrite(Box2_LED, digitalRead(Box3_Button_gruen));
    digitalWrite(Box3_LED, HIGH ); //digitalRead(Box3_Schloss));
  }
  
  if (operationMode == broken)
  {
    if (brokenCounter == 0)
    {
      digitalWrite(Box1_LED, random (1));
      digitalWrite(Box2_LED, random (1));
      digitalWrite(Box3_LED, random (1));
      
      brokenCounter = random (30);
    }
    else
    {
      brokenCounter--;
    }
  }

  while (Serial.available())
  {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // if the incoming character is a newline, compute received command:
    if (inChar == '\n')
    {
      if (inputString == String("setstat_enabled"))
      {
        operationMode = enabled;
      }

      if (inputString == String("setstat_disabled"))
      {
        operationMode = disabled;
        digitalWrite(Box1_LED, LOW);
        digitalWrite(Box2_LED, LOW);
        digitalWrite(Box3_LED, LOW);
        gameFinished =true;
      }

      if (inputString == String("setstat_broken"))
      {
        operationMode = broken;
        gameFinished =true;
      }
      Serial.print("status_");
      Serial.println (operationMode);
    } 
    // add it to the inputString:
    inputString += inChar;
  }
  // reset input buffer
  inputString = String("");
  
  delay (5);
}


