#include <stdint.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
  
#define LIGHT_TO_FREQ_F 2
#define LIGHT_TO_FREQ_OD 3 
#define BUTTON 8

#define LED_P0 9
#define LED_P1 10

#define LED_P2 11 // OD

#define F_LED_BRIGHT 255
#define OD_LED_BRIGHT 255

// volatile to be stored as close to the µC as possible
volatile unsigned long cntF = 0;
unsigned long oldcntF = 0;
unsigned long tF = 0;
unsigned long lastF;

volatile unsigned long cntOD = 0;
unsigned long oldcntOD = 0;
unsigned long tOD = 0;
unsigned long lastOD;

// this is the recorded frequency. The average window is used to get more stable results
unsigned long hz_refF; // blank frequency
unsigned long hz_ref_avgF;
unsigned long hz_refOD; // blank frequency
unsigned long hz_ref_avgOD;
#define AVERAGE_WINDOW 5

// this determines the output
#define DEBUG_MODE 1
#define OD_DIGITS 3

unsigned long hz_avgF[AVERAGE_WINDOW];
unsigned long hz_windowF = 0;
unsigned long iCountF = 0;
unsigned long hz_avgOD[AVERAGE_WINDOW];
unsigned long iCountOD = 0;
unsigned long hz_windowOD = 0;

// this is for selection either OD or F measurement
#define F_MODE 0
#define OD_MODE 1
#define ODF_PIN 5
bool odf_mode;

//interrupt for OD
void irqOD()
{
  cntOD++;
}

//interrupt for F
void irqF()
{
  cntF++;
}

void blank() {
  iCountOD = 0;
  
  uint8_t i = 0;
  for (i = 0; i < AVERAGE_WINDOW; ++i)
  {
    unsigned long start = millis();
    oldcntOD = cntOD;
    
    while( millis()-start<=1000 ){}
    
    unsigned long t = cntOD;
    hz_refOD = t - oldcntOD;
    
    hz_avgOD[iCountOD++ % AVERAGE_WINDOW] = hz_refOD;
    
    cntOD = 0;
    oldcntOD = 0;
  }
  hz_refOD = 0;
  for (i = 0; i < AVERAGE_WINDOW; ++i)
  {
    hz_refOD += hz_avgOD[i];
  }
  
  hz_refOD = hz_refOD / AVERAGE_WINDOW;
  
  cntOD = 0;
  oldcntOD = 0;
  lastOD = millis();
}

void setup() {
  Serial.begin(9600);
  
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH);
  delay(500);

  pinMode(LIGHT_TO_FREQ_F, INPUT);
  digitalWrite(LIGHT_TO_FREQ_F, HIGH);
  attachInterrupt(0, irqF, RISING); //interrupt 0 is on PIN 2
  
  pinMode(LIGHT_TO_FREQ_OD, INPUT);
  digitalWrite(LIGHT_TO_FREQ_OD, HIGH);
  attachInterrupt(0, irqOD, RISING); //interrupt 0 is on PIN 2
    
  lcd.init();
  
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Optical Density");
  lcd.setCursor(0,1);
  lcd.print("Fluorescence");
    
  analogWrite(LED_P0, F_LED_BRIGHT);
  analogWrite(LED_P1, F_LED_BRIGHT);
  analogWrite(LED_P2, OD_LED_BRIGHT);

  // this is needed for OD :)
  blank();
}

int getMeasurement(volatile unsigned long *cnt, unsigned long *oldcnt, unsigned long* hz_avg, unsigned long* hz, unsigned long* hz_win, unsigned long* iCount, unsigned long* last, int wait)
{
  if (millis() - *last < wait)
  return 0;
  
  unsigned long t = *cnt;
  *hz = t - *oldcnt;

  hz_avg[*iCount % AVERAGE_WINDOW] = *hz;
  *iCount = *iCount + 1;

  unsigned long hz_window = 0;
  for (uint8_t i = 0; i < AVERAGE_WINDOW; ++i)
  {
    hz_window += hz_avg[i];
  }
  
  *hz_win = hz_window / AVERAGE_WINDOW; // windows average frequency
  *oldcnt = t;
  
  *last = millis();
  *cnt = 0;
  *oldcnt = 0;
  
  return 1;
}

int button_in;

void loop() {
  uint64_t j;
  
  button_in = digitalRead(BUTTON);
  
  if( button_in == HIGH)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("OD:");
    lcd.setCursor(0, 1);
    lcd.print("setting blank");
    
    blank();
  }
  
  analogWrite(LED_P0, F_LED_BRIGHT);
  analogWrite(LED_P1, F_LED_BRIGHT);
  delay(100);

  unsigned long hzF, hzOD; //current frequency

  getMeasurement(&cntF, &oldcntF, hz_avgF,&hzF, &hz_windowF, &iCountF, &lastF, 4000);
  getMeasurement(&cntOD, &oldcntOD, hz_avgOD,&hzOD, &hz_windowOD, &iCountOD, &lastOD, 1000);
    
  analogWrite(LED_P0, F_LED_BRIGHT);
  analogWrite(LED_P1, F_LED_BRIGHT);
  delay(500);
  
  Serial.print(0);
  Serial.print("#;");
  Serial.print(0);
  Serial.print(";");
  Serial.print(0);
  Serial.print(";");
  Serial.print(0);
  Serial.print(";");
  Serial.print(hz_refOD);
  Serial.print(";");
  Serial.print(hzOD);
  Serial.print(";");
  Serial.print(hzF);
  Serial.print("\n");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("OD:");
  lcd.print(hz_windowOD);
    
  // since frequency is linear to transmission, this is transmittance      
  lcd.print(" T :");
  lcd.print(hz_windowF);
      
  if (DEBUG_MODE)
  {
    lcd.setCursor(0, 1);
    lcd.print("OD0: "); // blank value - interesting for OD
    lcd.print(hz_refOD);
    lcd.print(" Tc:"); // current value
    lcd.print(hzF);
  }
}
