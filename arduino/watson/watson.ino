/*
From http://2014.igem.org/Team:Aachen
*/

//includes
#include <DHT.h>
#include <LedControl.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

//pins
#define DHTPIN 9   // aluminium block temperature
#define DHT2PIN 10 // room temperature

#define LED_450_PIN 5
#define LED_480_PIN 8
#define LED_450_CHAR 53
#define LED_480_CHAR 56

//pins for the temperature display
#define LCD_DI 13
#define LCD_CLK 12
#define LCD_LD 11

//pin for the mosfet that controls the Peltier heater
#define PEL_MOSFET 4

#define LEDS 3

//target temperature
#define T_TARGET 38

#define DHTTYPE DHT22   // DHT 22  (AM2302)


DHT dht_int(DHT2PIN, DHTTYPE);
DHT dht_ext(DHTPIN, DHTTYPE);

LedControl lc=LedControl(LCD_DI, LCD_CLK, LCD_LD, 1);

//Peltier
int power = 0;                                  //Power level from 0 to 99%
int peltier_level = map(power, 0, 99, 0, 255);  //This is a value from 0 to 255 that actually controls the MOSFET


/*
CONTROLS
bTemp switches Temp sensor on
bPeltier switches Peltier on
bDisplay switches Display on
*/

bool bTemp = true;
bool bPeltier = true;
bool bDisplay = true;

unsigned int delaytime = 100;

int iHeatingLevel = 0;

void setup() {  
  Serial.begin(9600); 
  Serial.println("Serial Communication 9600 baud started");
  
  pinMode(LED_450_PIN,OUTPUT);
  pinMode(LED_480_PIN,OUTPUT);
  pinMode(LEDS,OUTPUT);

  
  if (bTemp)
  {
    dht_int.begin();
    dht_ext.begin();
  }
  

  if (bDisplay)
  {
    lc.shutdown(0,false);
    /* Set the brightness to a medium values */
    lc.setIntensity(0,8);
    /* and clear the display */
    lc.clearDisplay(0);
  }
}

void loop() {
  char option;

  if(Serial.available()>0){
    option = Serial.read();
      
    // turn 450nm LED
    if( option == LED_450_CHAR ){
      digitalWrite(LED_450_PIN,HIGH);
    }
    
    // turn 480nm LED
    if( option == LED_480_CHAR ){
      digitalWrite(LED_480_PIN,HIGH);
    }
    
    // turn off activated LED
    if( option == 'o' ){
      digitalWrite(3,LOW);
      
      digitalWrite(LED_450_PIN, LOW);
      digitalWrite(LED_480_PIN, LOW);
    }   
  }

  if (bTemp)
  {
      
    float t_pel = dht_ext.readTemperature();  
    float t_room = dht_int.readTemperature();
  
    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t_pel) || isnan(t_room)) {

    } else {
      
      // control the display
      if( bDisplay ){
    
        double t = t_room;
        
        //set display text
        lc.setRow(0,0,B01001110);
        lc.setRow(0,1,B01100011);
        
        if( iHeatingLevel>0 ){
          lc.setRow(0,7,B00110111);
        }

        int digit;
        bool display_digit;
        for( int i=3; i<8; i++ ){
          display_digit = true;
          if( i>3 ){
            digit = t/(pow(10,i-4));
            if( pow(10,i-4)>t ) display_digit=false;
          }
          else{
            digit = t*pow(10,4-i);  
          }
          if( display_digit==false ){
            lc.setRow(0,i,B00000000);
          }
          else{
            digit = abs(digit%10);

            if( i==4 ){
              lc.setDigit(0,i,digit,true);
            }
            else{  
              lc.setDigit(0,i,digit,false);
            }
          }
          delay(delaytime);
        }
      }
    }
  }
  
  // control the Peltier
  if (bPeltier)
  {
    int pel_level_manual;
        
    // temperature control    
    if ((t_pel > 50) && ( t_pel < 60 ))
    {
      iHeatingLevel = 70;
    }
    
    if (t_pel < 50)
    {
      iHeatingLevel = 99;
    }
    
    if (t_room < T_TARGET - 5)
    {
      iHeatingLevel = MIN(iHeatingLevel, 99);
    }
    if (t_room > T_TARGET - 2)
    {
      iHeatingLevel = MIN(iHeatingLevel, 85);
    }
    if (t_room > T_TARGET)
    {
      iHeatingLevel = 0;
    }
        
    // set power of the Mosfet
    power = map(iHeatingLevel,0,99,0,255); 
    analogWrite(PEL_MOSFET, power); 
  }
}
