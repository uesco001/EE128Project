#define byte uint8_t
#include "ServoTimer2.h"
//~~~~~~~~~~~~~~~ Servo variables ~~~~~~~~~~~~~~~
ServoTimer2 servo1;                // Servo X
ServoTimer2 servo2;                // Servo Y
unsigned c = 0;
unsigned char servoValX = 87;      // Center for X axis
unsigned char servoValY = 85;      // Center for Y axis
//~~~~~~~~~~~~~~~ Signal interpretation variables ~~~~~~~~~~~~~~~
// This is our BST
unsigned char alphabet[] = {'0','T','E','M','N','A','I','O','G','K','D','W',
'R','U','S',' ','2','Q','Z','Y','C','X','B','J','P','3','L','4', 'F','V','H'};
// This is the BST
unsigned char Long = 2; // Long pulse
unsigned char Short = 1; // Short pulse
unsigned char pulse [5] = {0,0,0,0,0};
unsigned char UARTflag = 0;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//ISR(TIMER1_COMPA_vect)
//{
//  UARTflag = 1;
//  Serial.print('p');
//}
unsigned char MenuCase = 'M';
//------------------------------ search() begin ------------------------------  
unsigned char search(unsigned char cnt, unsigned char i, unsigned char value[])
{
  if(value[cnt] == Long)
  {
    i = i*2 + 1; // increment index for long value 
    cnt++;       // increment cnt
    search(cnt,i,value); // call funtion again
  }
  else if( value[cnt] == Short)
  {
    i = i*2 + 2;    // increments index for short value
    cnt++;          // incremetn cnt
    search(cnt,i,value); // call funtion again
  }
  else
  {
    return alphabet[i];  // base case 
  }
}
//------------------------------ search() end ------------------------------ 

//------------------------------ check() begin ----------------------------------
bool check(){
  unsigned i = 0;
  unsigned int times[7] = {0,0,0,0,0, 0};  // create array of long for time event
  TCCR1A = 0;                   // nothing for register
  TCNT1 = 0;                    // set time to zero

  TIMSK1 = 0;   // disable all interrupts
  TCCR1B = 0x05; // Rise time and a prescale of 1024
  TCNT1 = 0; 
  for(i = 0; i < 7; i++) // check signal only at 5 events 
  {
    TCCR1B ^= 1<<6 ;  // toggle first will be rising then  falling
    while ( (TIFR1&(1<<ICF1)) == 0){  // poll for the fallin edge 
      if(TCNT1 > 1600)                 // wait untill max three periods of calibrated signal 
       {
        TIFR1 = 0;        // clear the flags
        i = 10;           // get out of condition just incase
        return false;     // return false if over the three peridos
  
        } 
     
    }
   times[i] = ICR1;     // store the timed even
   TIFR1 = (1<<ICF1);   // clear the IC flaga
   Serial.print("in\n");
  }

  TCCR1A = 0;   // set all timeer registers to ntohing 
  TIMSK1 = 0;   // set all timeer registers to ntohing 
  TCCR1B = 0;    // set all timeer registers to ntohing 
  TIFR1 = 0;   // set all timeer registers to ntohing 

  /// insert the calculations
  long high1 = times[3] - times[2];
  long low1 = times[4] - times[3];
  long high2 = times[5] - times[4];
  long low2 = times[6] - times[5];
// signal will be pulsed at around 100Hz
  for(i = 0; i < 7; i++)
  {
    Serial.print("\ntimes = ");
    Serial.print(times[i]);
  }
  Serial.print("\nend");

  
  if(high1 > 75 || high1 < 45)
  {return false;}
  if(low1 > 120 || low1 < 90)
  {return false;}
  if(high2 > 75 || high2 < 45)
  {return false;}
  if(low2 > 120 || low2 < 90)
  {return false;}

  
  
  return true;
  
}
//------------------------------ check() end------------------------------

//------------------------------ calibrate() begin ------------------------------
// this funtion will go through all the angles of the servo and will check where the signal is the strongest
unsigned calibrate(){
  unsigned place = 0;   // initialize place
  while(place < 32401)  // 32401 is 180*180
  {
    servoValX = place % 180 ;  // gives value from 0-179
    servoValY = place / 180;  // gives value from 0-180
    
    
    if(servoValY%2)       // if Y is old then that means X is at 179
    {
    
      servo1.write(3000 - map(servoValX, 0,180,750,2250)); // so do 180 - x  as x increases (not changed bc of servo 2
    }
    else
    { 
      servo1.write( map(servoValX, 0,180,750,2250)); }   // if its not odd then do regulare increment 
      
   
    servo2.write(map(servoValY, 0,180,750,2250));  // keep incrementing y 
     if(digitalRead(8) == HIGH)                   // read if there is some signal
    {
      if(check()){          // check is its the correct signal 
        return place;       // if so turn in the x coordinate
      }
    }
    place++;              // if not increment place for new position
    delay(50);            // delay so servos 
   
  }
  return 32500;           // if no signal was found then return a value greater than 180*180
}
//------------------------------ calibrate() end ------------------------------

//------------------------------ getSignal() begin ------------------------------
unsigned char getSignal(){
  
  unsigned char value,x,i,b;
  long timedEvent[8] = {0};
  unsigned char  pulse[5] = {0};
  long pulse_width = 0;
  long upper_long = 9625;
  long lower_long = 4550;
  long upper_short = 4000;
  long lower_short = 900;
  unsigned int eventCount = 0;
  TCCR1B = 0x00;                    // so timer doesn't go off
  TIMSK1 = 0x00;            //Timer/Counter1 Interrupt Mask Register for OCR1A 
  TCCR1A = 0;
  eventCount = 0;
  UARTflag = 0;
  TCNT1 = 0;
  TIFR1 = (1<<ICF1);                             // Clear ICF (Input Capture flag) flag
  TCCR1B = 0x00;                               // Falling Edge, no prescaler
    //Serial.print("wait\n");
  while ( (TIFR1&(1<<ICF1)) == 0);              // Wait for timer to detect Falling Edge
  TIFR1 = (1<<ICF1); 
  TCNT1 = 0;
  TCCR1B = 0x05;                            // prescale to 64 
  //OCR1A = 65535;   
  //TIMSK1 = 0x02;                    
  eventCount = 1;
  x = 1;
  while((eventCount < 8) && (UARTflag == 0)){
      TCCR1B ^= 1<<6 ;
      //Serial.print('r');
      while ( !( (TIFR1&(1<<ICF1)))){if(TCNT1 - timedEvent[eventCount - 1] > 30000){break; } } // Wait for timer to detect Falling Edge
      TIFR1 = (1<<ICF1); 
      //Serial.print('t');
      if( (eventCount < 8 ) && !UARTflag){
        timedEvent[eventCount] = ICR1;
        TIFR1 = (1<<ICF1);                         // Clear ICF flag
        eventCount++;
       
        }
      else                                         //Can I optimize this
      {
        eventCount = 8;
      } 
  }//end while
   
       TIMSK1 = 0;
     
       TCCR1B = 0x00;   /// stop the timer
   
       /////////////////////////////// FINNA TRYNNA CONVERT TIME TO 1 AND 0
       
       for(i = 0; i < 4; i++){
          pulse_width = timedEvent[i*2 + 1] - timedEvent[i*2];
          if(/*pulse_width < upper_long && */pulse_width > lower_long )
          {
            value = 2;
          }
          else if( pulse_width < upper_short && pulse_width > lower_short)
          {
            value = 1;
          }
          else{
            value = 0;
          }
//          Serial.print(timedEvent[i*2 + 1],DEC);
//          Serial.print(" - ");
//          Serial.print(timedEvent[i*2],DEC);
//          Serial.print(" = ");
//          Serial.print(pulse_width),DEC;
//          Serial.print('\n');
          pulse[i] = value; 
       }
       b = search(0,0,pulse);
       return b;      
}
//------------------------------ getSignal() end ------------------------------


void setup() {
  Serial.begin(9600);
  servo1.attach(3);  // gets PWM from pin 3
  servo2.attach(5);  // gets PWM from pin 5
  pinMode(12,INPUT);
  pinMode(13,OUTPUT);
}
void loop() {
//  Serial.print("Calibrating... \n");
//  c = calibrate();
//  Serial.print(c);
//  while(1){
//  Serial.write(getSignal());
//  Serial.print('\n');
//  }
  
unsigned char letter;
  switch(MenuCase)
  {
    case 'M':
      digitalWrite(13,LOW);
      Serial.print("Main menu\n Search? enter S\nRead enter T\n");
      while(Serial.available() == 0);
      letter = Serial.read();
        if(letter == 'S'){MenuCase = letter; break;}
        else if(letter == 'T'){MenuCase = letter; break;}
        else { Serial.print("WRONG\n"); MenuCase = 'M'; break;}
     case 'T':
          digitalWrite(13,HIGH);
         Serial.print("\nYou can exit by pressing button and waving hand accross photodiode\nReading signal...\n");
         while(digitalRead(12)){
          Serial.write(getSignal());
          //Serial.print('\n');
         }
         MenuCase = 'M';
         break;
         

        
     case 'S':
        Serial.print("Searching for signal...\n");
        c = calibrate();
        if(c == 32500)
        {
         MenuCase = 'n';
         break; 
        }
        else{
          Serial.print("x angle is ");
          Serial.print(c%180);
          Serial.print("\ny angle is ");
          Serial.print(c/180);
          MenuCase = 'R';
          break; 
          
        }
     case 'n':

          Serial.print("Signal not found\n Search again? Y or N\n");
          while(Serial.available() == 0);
          letter = Serial.read();
          if(letter == 'Y')
          {
            MenuCase = 'S';
            break;
          }
          else{ 
            MenuCase = 'S';
            break;
          }
      case 'R':
          Serial.print("\nready to read?\n Y or N");
          while(Serial.available() == 0);
          letter = Serial.read();
          if(letter == 'Y')
          {
            MenuCase = 'T';
            
            break;
          }
          else{ 
            MenuCase = 'R';
            break;
          }
          
          
        
  
    
  }
  
}