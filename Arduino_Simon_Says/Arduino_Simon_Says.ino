
/* Some Simon says style electronics game based on a Arduino. The special trick is that there is the main Arduino controller (with LCD) and 3 Boxes where buttons have to be pressed.
Those boxes are connected through ethernet cable (no ethernet protocol!) with up to 20m distance.

*/


/* Hardware test:

LED 1 (pin 13)  OK
LED 2 (pin 9) OK
LED 3 (pin A3) OK

LCD: ok

*/
// include the library code:
#include <LiquidCrystal.h>

// SmartMaker special
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// global operation mode
const String disabled = String("disabled");
const String enabled = String("enabled");
//const String broken = String("broken");
//const String testing = String("testing");
String operationMode = enabled;

int taskCounter = 0;
int brokenCounter = 0;
boolean gameFinished = false;

const unsigned char Box1_Button_gruen = 12; //gelb: Musik
const unsigned char Box1_Button_blau = 11;  // Vakuumeinlass
const unsigned char Box1_Schloss = 10;  // Radon
const unsigned char Box1_LED = 13;
const unsigned char Box2_Button_gruen = 8;  // Schrott abwurf
const unsigned char Box2_Regler = A5;  // SitzHeizung L-M-H   // 15-136
const unsigned char Box2_Schloss = A4;  // Schott
const unsigned char Box2_LED = 9;
const unsigned char Box3_Button_gruen = 1;  // Evak
const unsigned char Box3_Button_blau = A1; // gelb  Radium
const unsigned char Box3_Schloss = A0;  // Gefahr
const unsigned char Box3_LED = A3;

struct SystemState{
  unsigned char B1_Musik;
  unsigned char B1_Vakuum;
  unsigned char B1_Radon;
  unsigned char B2_Schrott;
  unsigned char B2_SitzHeizung;
  unsigned char B2_Schott;  
  unsigned char B3_Evak;
  unsigned char B3_Radium;
  unsigned char B3_Gefahr;
};

const String wrong_entry_string = String("Falscheingabe");
const String timeout_string = String("Zeit abgelaufen");
const String bugphalanx = String("Bugphalanx Rekonfigurieren:");
const String Musik_string = String("Musik aus um Strom zu sparen");
const String Vakuum_string = String("Vakuum in Dunkelkammer einlassen");
const String Radon_string = String("Radon magnetisieren");
const String Schrott_string = String("Schrott abwerfen");
const String Schott_string = String("Schott schliessen");
const String Evak_string = String("Evakuierung vorbereiten");
const String Radium_string = String("Radium inhalator vorbereiten");
const String Gefahr_string = String("Gefahren - Anzeige aus um Panik zu vermeiden");

const String SH_low_string = String("Sitzheizung auf Low");
const String SH_medium_string = String("Sitzheizung auf Medium");
const String SH_high_string = String("Sitzheizung auf High");

String current_task = String();

long successes = 0; // Number of successful tasks in a row

// States:
boolean game_running = true;  // Set to true while a game is running
boolean task_open = false;     // There is a task for the players open
long calm_phase = 10; // counts down the calm phase between games
long game_started_at = 0; // Millis when the game started
long games_won = 0;       // Number of games won
long games_failed = 0;    // Number of games failed

// Scrolling
char scroll_direction = 0;
char scroll_pos = 0;
const char scroll_steps = 30;
unsigned long last_scroll_at=0;      // When the last scroll step happened
unsigned long millis_per_scroll=500;  // Milliseconde till the next scroll step
const int scroll_countdown_start = 10; // Setting init for scroll countdown
int scroll_countdown = scroll_countdown_start; // Iterations till scrolling starts

// Game settings
// Use these to tune the difficulty
int failed_break = 10000; // Millis gamer has to wait after fail
int debug_delay = 0;   // Delay for debugging
int tick_delay = 100;    // Delay in every game tick
long min_successes = 10;  // Succeesses till the game iteration is won
long millis_for_game = 80000;  //Milliseconds to finish game


/*
 Extra function, we will need debouncing
*/
unsigned char read_button(unsigned char pin) 
{
  
  return digitalRead(pin);
}

/*
 Extra function, we will need debouncing
*/
int read_slide(unsigned char pin) 
{
  return analogRead(pin);
}



/** Send state over Serial (debug !)
*
*
*
**/
void print_state(struct SystemState state)
{
  Serial.print("Debug: BTN Musik:  ");
  Serial.println (state.B1_Musik);
  Serial.print("Debug: BTN Vakuum:  ");
  Serial.println (state.B1_Vakuum);
  Serial.print("Debug: BTN Radon:  ");
  Serial.println (state.B1_Radon);
  Serial.print("Debug: BTN Schrott:  ");
  Serial.println (state.B2_Schrott);
  Serial.print("Debug: BTN Schott:  ");
  Serial.println (state.B2_Schott);
  Serial.print("Debug: BTN Evak:  ");
  Serial.println (state.B3_Evak);
  Serial.print("Debug: BTN Radium:  ");
  Serial.println (state.B3_Radium);
  Serial.print("Debug: BTN Gefahr:  ");
  Serial.println (state.B3_Gefahr);
  Serial.print("Debug: BTN Sitzheizung:  ");
  Serial.println (state.B2_SitzHeizung);
}

/** Announce a task to do
*
**/
void announce(String message)
{
  print_lcd(message);
}

long old_changeme = -1;

/* Generate a new state with only a small difference
*
*/

struct SystemState randomize_next_state(struct SystemState current)
{
  struct SystemState next;
  long changeme;
  
  // Copy current
  next.B1_Musik = current.B1_Musik;
  next.B1_Vakuum = current.B1_Vakuum;
  next.B1_Radon = current.B1_Radon;
  next.B2_Schrott = current.B2_Schrott;
  next.B2_Schott = current.B2_Schott;
  next.B3_Evak = current.B3_Evak;
  next.B3_Radium = current.B3_Radium;
  next.B3_Gefahr = current.B3_Gefahr;
  next.B2_SitzHeizung = current.B2_SitzHeizung;
  
  // Identify button to change
  
  while (changeme == old_changeme)
  {
    changeme = random(0,9);  
  }
  old_changeme = changeme;

  Serial.print("Debug: Randomizing....");
  Serial.println(changeme);
  
  // modify and announce
  switch (changeme){
    case 0:
      next.B1_Musik = ! next.B1_Musik;
      announce(Musik_string);
      current_task = Musik_string;
      break;
    case 1:
      next.B1_Vakuum = ! next.B1_Vakuum;
      announce(Vakuum_string);
      current_task = Vakuum_string;
      break;
    case 2:
      next.B1_Radon = ! next.B1_Radon;
      announce(Radon_string);
      current_task = Radon_string;
      break;
    case 3:
      next.B2_Schrott = ! next.B2_Schrott;
      announce(Schrott_string);
      current_task = Schrott_string;
      break;
    case 4:
      next.B2_Schott = ! next.B2_Schott;
      announce(Schott_string);
      current_task = Schott_string;
      break;
    case 5:
      next.B3_Evak = ! next.B3_Evak;
      announce(Evak_string);
      current_task = Evak_string;      
      break;
    case 6:
      next.B3_Radium = ! next.B3_Radium;
      announce(Radium_string);
      current_task = Radium_string;      
      break;
    case 7:
      next.B3_Gefahr = ! next.B3_Gefahr;
      announce(Gefahr_string);
      current_task = Gefahr_string;      
      break;
    case 8:
      if (next.B2_SitzHeizung == 1)
      {     
        next.B2_SitzHeizung = 2;
        announce(SH_medium_string);
        current_task = SH_medium_string;        
      }
      else if (next.B2_SitzHeizung == 3)
      {
        next.B2_SitzHeizung = 2;
        announce(SH_medium_string);
        current_task = SH_medium_string;
      }
      else if (random(0,2)==0)
      {
        next.B2_SitzHeizung = 1;
        announce(SH_low_string);
        current_task = SH_low_string;        
      }
      else
      {
        next.B2_SitzHeizung = 3;
        announce(SH_high_string);
        current_task = SH_high_string;
      }      
      break;      
    default:
      break;
  }  
  
  return next;

}




/*
Read the current state
*/
struct SystemState read_state() {
  struct SystemState state;
  int regler=0;
  
  state.B1_Musik = read_button(Box1_Button_gruen);
  state.B1_Vakuum = read_button(Box1_Button_blau);
  state.B1_Radon = read_button(Box1_Schloss);
  state.B2_Schrott = read_button(Box2_Button_gruen);
  state.B2_Schott = read_button(Box2_Schloss);
  state.B3_Evak = read_button(Box3_Button_gruen);
  state.B3_Radium = read_button(Box3_Button_blau);
  state.B3_Gefahr = read_button(Box3_Schloss);
  
  regler = read_slide(Box2_Regler);
  if  (regler < 55)
    state.B2_SitzHeizung = 1;
  else if  (regler < 90)
    state.B2_SitzHeizung = 2;
  else
    state.B2_SitzHeizung = 3;
  
  return state;
}


/** Takes 2 states, if both are equal (with small flex. in analog) returns True
*
* return: True if states match, False else
*/
boolean state_matches(struct SystemState s1, struct SystemState s2)
{
  if (s1.B1_Musik != s2.B1_Musik)
  {
    return false;
  }
  if (s1.B1_Vakuum != s2.B1_Vakuum)
  {
    return false;
  }
  if (s1.B1_Radon != s2.B1_Radon)
  {
    return false;
  }
  if (s1.B2_Schrott != s2.B2_Schrott)
  {
    return false;
  }
  if (s1.B2_Schott != s2.B2_Schott)
  {
    return false;
  }
  if (s1.B3_Evak != s2.B3_Evak)
  {
    return false;
  }
  if (s1.B3_Radium != s2.B3_Radium)
  {
    return false;
  }
  if (s1.B3_Gefahr != s2.B3_Gefahr)
  {
    return false;
  }
  if (s1.B2_SitzHeizung != s2.B2_SitzHeizung)
  {
    return false;
  }

  return true;
  
}

/* Read the debounced state
*/
struct SystemState read_state_debounced()
{
  struct SystemState statea;
  struct SystemState stateb;
  
  statea = read_state();
  delay(10);
  stateb=read_state();
  
  while (!state_matches(statea,stateb))
  {
    statea = stateb;
    delay(10);
    stateb=read_state();
  }
  return statea;

}

/** Compare the current state to 2 possible states
*
* return: 0: first state, 1: second state, 2: Wrong state
**/
unsigned char compare_state(struct SystemState current, struct SystemState prev, struct SystemState next)
{  
  if (state_matches(current, next))
  {
    return 1;
  }
  if (state_matches(current, prev))
    return 0;

  return 2;
}

long hash_state()
{
  struct SystemState state;
  long hash = 0;
  
  state = read_state_debounced();
  hash = 
  state.B1_Musik + 
  state.B1_Vakuum * 2 +
  state.B1_Radon * 4 +
  state.B2_Schrott * 8 +
  state.B2_Schott * 16 +
  state.B3_Evak * 32 +
  state.B3_Radium * 64 +
  state.B3_Gefahr * 128 +
  state.B2_SitzHeizung * 256;
  
  return hash;
}

String inputString = "";         // a string to hold incoming data


void print_lcd(String data)
{
  lcd.clear();
  lcd.print(bugphalanx);
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  lcd.print(data);
  scroll_countdown = scroll_countdown_start;
}

void leds_off(){
  digitalWrite(Box1_LED, LOW);
  digitalWrite(Box2_LED, LOW);
  digitalWrite(Box3_LED, LOW);
}

/** Switch a random led on
*
**/
void random_led_on()
{
  char id = random (1,4);
  switch (id)
  {
    case 1:
        digitalWrite(Box1_LED, HIGH);
        break;
    case 2:
        digitalWrite(Box2_LED, HIGH);
        break;
    case 3:
        digitalWrite(Box3_LED, HIGH);
        break;
  }
}

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

  // Print a message to the LCD.
  
  
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
  randomSeed(hash_state());
  Serial.println ("event_booted");
  Serial.println ("event_gamestart");  
}


void loop() {
  struct SystemState current_state;
  struct SystemState old_state;
  struct SystemState next_state;
  
  unsigned char res; // result of the player jobs

  // Scrolling
  if (last_scroll_at + millis_per_scroll < millis())
  {
    last_scroll_at = millis();
    scroll_countdown -= 1;
    if (scroll_countdown <= 0)
    {
      if (scroll_direction)
      {
        for (int i=0;i<scroll_steps; i++)
          lcd.scrollDisplayRight();
        scroll_direction = 1;
      }
      else
      {
        lcd.scrollDisplayLeft();
      }
      scroll_pos += 1;
      if (scroll_pos > scroll_steps)
      {
        scroll_pos = 0;
        scroll_direction != scroll_direction;
      }
    }
  }
  
  
  // Timed status updates
  taskCounter++;
  
  if (taskCounter == 2000)  // wrap around every 10 Seconds
  {
    taskCounter = 0;
  }
  
  if ((taskCounter % 1000) == 0) // 5000ms Tick
  {
    Serial.print("status_");
    Serial.println (operationMode);
    
    // Debugging, printing state to serial
    /*current_state = read_state_debounced();
    Serial.println("Debug: Current state");
    print_state(current_state);
    Serial.println("Debug: Target state");
    print_state(next_state);*/
  }

  if (operationMode == enabled)
  {
    if (game_running)
    {
      if (game_started_at + millis_for_game < millis())
      {
        task_open = false;
        game_running = false;
        print_lcd(timeout_string);
        //operationMode = broken;
        Serial.println("Debug: Task timeouted");
        games_failed += 1;
        delay(failed_break);
      }
      
      delay(debug_delay); // Debug delay
      Serial.println("Debug: Game running");
      if (task_open)
      {
        Serial.print("Debug: Task open  ");
        Serial.println(current_task);
        // We already have a task. Waiting for answer or fail
        // 3) Check for expected change. If wrong change: broken        

        current_state = read_state_debounced();

        res = compare_state(current_state, old_state, next_state);
        Serial.print("Debug: State comparison: ");
        Serial.println(res);

        if (res == 0)
        { // Old state, nothing new
          Serial.println("Debug: old state, nothing changed");
          Serial.println("Debug: Current state");
          print_state(current_state);
          Serial.println("Debug: Target state");
          print_state(next_state);
        }
        else if (res == 1)
        {
          // New state, success
          task_open = false;
          successes += 1;
          Serial.println("Debug: Task success");
          Serial.print("Debug: Task success count");
          Serial.println(successes);
        }
        else if (res == 2)
        {
          // Error state
          task_open = false;
          game_running = false;
          print_lcd(wrong_entry_string);
          games_failed += 1;
          Serial.println("Debug: Task failed");
          delay(failed_break);
        }
        
        if (successes > min_successes)
        { // Game won, so far
          game_running = false;
          successes = 0;
          games_won += 1;
          calm_phase = random(10, 30);
          Serial.println("Debug: Game round won");
        }
      }
      else
      {
        Serial.println("Debug: Randomizing new task");
        // For debouncing add delay
        //delay(50);
        leds_off();
        random_led_on();
        
        // Create a new task
        // 1) Read the current state
        old_state = read_state_debounced();
    
        // 2) Randomize a job
        next_state = randomize_next_state(old_state);
                
        task_open = true;
        
        Serial.println("-------------");
        //current_state = read_state();
        Serial.println("Debug: Current state");
        print_state(old_state);
        Serial.println("Debug: Target state");
        print_state(next_state);
        Serial.println("-------------");        
      }
    }
    else
    {
      print_lcd("System OK ");
      Serial.print("Debug: calm phase ");
      Serial.println (calm_phase);
      calm_phase = calm_phase - 1;
      
      if (calm_phase <= 0)
      { // Starting game
        game_running = true;
        task_open = false;
        game_started_at = millis();
      }
    }
  }
  
  // Serial control for external controller
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
      }
      Serial.print("status_");
      Serial.println (operationMode);
    } 
    // add it to the inputString:
    inputString += inChar;
  }
  // reset input buffer
  inputString = String("");

  delay (tick_delay);
}


