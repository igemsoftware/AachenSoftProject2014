#include <stdint.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
  
#define LIGHT_TO_FREQ 2

#define BUTTON 8

#define LED_P0 9
#define LED_P1 10

#define F_LED_BRIGHT 255

// volatile to be stored as close to the µC as possible
volatile unsigned long cnt = 0;
unsigned long oldcnt = 0;
unsigned long t = 0;
unsigned long last;

// this is the recorded frequency. The average window is used to get more stable results
unsigned long hz_ref; // blank frequency
unsigned long hz_ref_avg;
#define AVERAGE_WINDOW 5

// this determines the output
#define DEBUG_MODE 1
#define OD_DIGITS 3

unsigned long hz_avg[AVERAGE_WINDOW];
unsigned long iCount = 0;

// this is for selection either OD or F measurement
#define F_MODE 0
#define OD_MODE 1
#define ODF_PIN 5
bool odf_mode;

//interrupt counter
void irq1()
{
  cnt++;
}

void blank() {
  iCount = 0;
  
  uint8_t i = 0;
  for (i = 0; i < AVERAGE_WINDOW; ++i)
  {
    unsigned long start = millis();
    oldcnt = cnt;
    
    while( millis()-start<=1000 ){}
    
    t = cnt;
    hz_ref = t - oldcnt;
    
    hz_avg[iCount++ % AVERAGE_WINDOW] = hz_ref;
    
    cnt = 0;
    oldcnt = 0;
  }
  hz_ref = 0;
  for (i = 0; i < AVERAGE_WINDOW; ++i)
  {
    hz_ref += hz_avg[i];
  }
  
  hz_ref = hz_ref / AVERAGE_WINDOW;
  
  cnt = 0;
  oldcnt = 0;
  last = millis();
}

void setup() {
  Serial.begin(9600);
  
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH);
  delay(500);

  pinMode(LIGHT_TO_FREQ, INPUT);
  digitalWrite(LIGHT_TO_FREQ, HIGH);
  attachInterrupt(0, irq1, RISING); //interrupt 0 is on PIN 2
  
  //determine the device mode
  odf_mode = digitalRead(ODF_PIN); // 1 = OD; 0 = F
  
  Serial.print("odf_mode ");
  Serial.println(odf_mode);
  
  lcd.init();
  
  lcd.backlight();
  lcd.setCursor(0,0);
  if( odf_mode == OD_MODE ){
    lcd.print("Optical Density");
  } else {
    lcd.print("Fluorescence");
    
    analogWrite(LED_P0, F_LED_BRIGHT);
    analogWrite(LED_P1, F_LED_BRIGHT);
  }
  lcd.setCursor(0,1);
  lcd.print("Measurement");

  if( odf_mode == OD_MODE ){
    analogWrite(LED_P0, 255); // switches LED on constantly for
    blank();
  } else {
    delay(1000); 
  }
}

uint64_t sum = 0;
int count = 0;
unsigned long lastOutput = -10000; 

int button_in;

void loop() {
  uint64_t j;
  
  button_in = digitalRead(BUTTON);
 
  //set blank if the button is pressed in OD mode 
  if( (odf_mode == OD_MODE) && (button_in == HIGH) ){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("OD:");
    lcd.setCursor(0, 1);
    lcd.print("setting blank");
    
    blank();
  }
  
  if (odf_mode != OD_MODE)
  {
    analogWrite(LED_P0, F_LED_BRIGHT);
    analogWrite(LED_P1, F_LED_BRIGHT);
    delay(100);
  }

  unsigned long hz; //current frequency
  unsigned long wait = 0;
  
  if (odf_mode == OD_MODE)
  {
    wait = 1000;
  }
  
  if (odf_mode != OD_MODE)
  {
    wait = 4000;
  }

  if (millis() - last >= wait)
  {
    t = cnt;
    hz = t - oldcnt;

    hz_avg[iCount++ % AVERAGE_WINDOW] = hz;

    unsigned long hz_window = 0;
    for (uint8_t i = 0; i < AVERAGE_WINDOW; ++i)
    {
      hz_window += hz_avg[i];
    }
    
    hz_window = hz_window / AVERAGE_WINDOW; // windows average frequency
    
    if (odf_mode != OD_MODE)
    {
      analogWrite(LED_P0, F_LED_BRIGHT);
      analogWrite(LED_P1, F_LED_BRIGHT);
      delay(100);
    }
    
    Serial.print(0);
    Serial.print("#;");
    Serial.print(0);
    Serial.print(";");
    Serial.print(0);
    Serial.print(";");
    Serial.print(0);
    Serial.print(";");
    Serial.print(hz_ref);
    Serial.print(";");
    Serial.print(hz);
    Serial.print(";");
    Serial.print(hz_window);
    Serial.print("\n");

    oldcnt = t;

    lcd.clear();
    lcd.setCursor(0, 0);
    if( odf_mode == OD_MODE ){
      
      lcd.print("OD:");
      lcd.print("_____");
      
      // since frequency is linear to transmission, this is transmittance      
      lcd.print(" T :");
      lcd.print(hz_window);
      
      if (DEBUG_MODE)
      {
        lcd.setCursor(0, 1);
        lcd.print("T0: "); // blank value - interesting for OD
        lcd.print(hz_ref);
        lcd.print(" Tc:"); // current value
        lcd.print(hz);
      }
    }
    else {
      lcd.print("Fluorescence: ");
      lcd.setCursor(0, 1);
      lcd.print(hz_window);
      lcd.setCursor(8, 1);
      lcd.print(hz);
      
      
      analogWrite(LED_P0, 0);
      analogWrite(LED_P1, 0);
      
      delay(500);
    }

    cnt = 0;
    oldcnt = 0;
    last = millis();
  }
}
