#define byte uint8_t  // if you are using the Servotimer 2 lib then include this and change boolean to bool in the .h file that you had to downnload form zip
#include "ServoTimer2.h"  /// using the servo2 library bc we are using Timer1 for input capture. Compiler does not like that so we use servotimer2 which uses timer 2
//~~~~~~~~~~~~~~~ Servo variables ~~~~~~~~~~~~~~~
ServoTimer2 servo1;                // Servo X axis 
ServoTimer2 servo2;                // Servo Y axis
unsigned c = 0;			   //  retrun vlaue for calibratoin 
unsigned char servoValX = 87;      // Center for X axis
unsigned char servoValY = 85;      // Center for Y axis
//*****************************************************
//~~~~~~~~~~~~~~~ Signal interpretation variables ~~~~~~~~~~~~~~~
// This is our BST
unsigned char alphabet[] = {'0','T','E','M','N','A','I','O','G','K','D','W',
'R','U','S',' ','2','Q','Z','Y','C','X','B','J','P','3','L','4', 'F','V','H'};
// This is the BST     
unsigned char Long = 2; // Long pulse
unsigned char Short = 1; // Short pulse
unsigned char pulse [5] = {0,0,0,0,0};  // pulse is array that stores 0 1 or 2 which corresponds to the pulsesread
//unsigned char UARTflag = 0; // uart flag for interrup didn't end up using it because of certain troubles
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//ISR(TIMER1_COMPA_vect) tried using it but didn't work well :p
//{
//  UARTflag = 1;
//  Serial.print('p');
//}
unsigned char MenuCase = 'M'; // initialize menu for switch statement
//------------------------------ search() begin ------------------------------  
// the purpose of the funtion is to iterate through the array recursively for a BST
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
// This funtion check is the signal is valid. We send a 100Hz squarewave with 30% duty cycle
// if its not that signal then check shall return false`

bool check(){
  unsigned i = 0;
  unsigned int times[7] = {0,0,0,0,0, 0};  // create array of long for time event
  // we chose an array of 7 because for some reason the first two times it reads and input its wrong. the first element of the array is the same as the last from the last time the funtion was called 
// the second element is some low value from 2 - 40; 
  TCCR1A = 0;                   // make sure nothing is set from other funtions
  TIMSK1 = 0;   // disable all interruptls incase they were set
  TCCR1B = 0x05; // falling edge time and a prescale of 1024
  TCNT1 = 0; 
  for(i = 0; i < 7; i++) // check signal only at 7 events 
  {
    TCCR1B ^= 1<<6 ;  // toggle first will be rising then  falling
    while ( (TIFR1&(1<<ICF1)) == 0){  // poll for the fallin edge 
      if(TCNT1 > 600)                 // wait untill max of 3 periods of calibrated signal 
       {
        TIFR1 = 0;        // clear the flags
        return false;     // return false if over the three peridos
  
        } 
     
    }
   times[i] = ICR1;     // store the timed even
   TIFR1 = (1<<ICF1);   // clear the IC flaga
 // debugging  Serial.print("in\n");
  }

  TCCR1B = 0;    // stop the timer  
  TIFR1 = 0;   // clear any flags 

  /// calculating the high time and the low time twice to ensure valid signal 
// as mentioned before times[0] and times[1] were consistently inaccurate :/
  long high1 = times[3] - times[2];   // calculate the time of a the high signal
  long low1 = times[4] - times[3];    // calcualte the time of the low signal
  long high2 = times[5] - times[4];  // time of high 
  long low2 = times[6] - times[5];   // time of low
// you can print out the value of each timed event
/*  for(i = 0; i < 7; i++)
  {
    Serial.print("\ntimes = ");
    Serial.print(times[i]);
  }
  Serial.print("\nend");
*/
  // check if signal is valid
// theoretically the high time should be 1/100Hz * .3 = .003sec
// .003/(1024/16MHz) = 46.875
// theoretically the low time should be .007
// .007/(1024/16MHz) = 109.475
// we set larger threshold you can make it more stirct 
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
// the idea is that you have 180 degrees you can go through for each servo motor so if you would make that
// into coordinates it will be a matrix of 180x180
// instead of making an 2d array or a 1d arary you can simply have a counter varibale that can go up to 
// 180*180 = 32400. Since we want our algorithm to go through all the angles, we iterate through all the angles 
// of motor for the horizontal rotation, then we increment by 1 degree for the vertical rotation.
// to do this we must mod our counter variable by 181 to give us the degree and divide our counter variable
// by 180 to get that degree   

// the reason we are using map is because originally we were using the servo1 lib but it conflicted with our
// funtions since they use the same timer. So we use servotimer2 lib. The write funtion is different and goes 
// from 750 to 2250 instead of 0 to 180. Thus we use the map funtion because we didn't want to do the work.

unsigned calibrate(){
  unsigned place = 0;   // initialize place
  while(place < 32401)  // 32401 is 180*180
  {
    servoValX = place % 180 ;  // gives value from 0-179
    servoValY = place / 180;  // gives value from 0-180
    
    //once X hits 179 for its angle we want it to decrement to 0 
    // each time X hits 179 then Y is odd
    if(servoValY%2)       // if Y is old then that means X is at 179
    {
    
      servo1.write(3000 - map(servoValX, 0,180,750,2250)); // decrements
    }
    else
    { 
      servo1.write( map(servoValX, 0,180,750,2250)); }   // if its not odd then do regulare increment 
      
   
    servo2.write(map(servoValY, 0,180,750,2250));  // keep incrementing y 
     if(digitalRead(8) == HIGH)                   // read if there is some signal
    {
      if(check()){          // check is its the correct signal 
        return place;       // if so turn in the coordinate
      }
    }
    place++;              // if not increment place for new position
    delay(50);            // delay so servos 
   
  }
  return 32500;           // if no signal was found then return a value greater than 180*180
}
//------------------------------ calibrate() end ------------------------------

//------------------------------ getSignal() begin ------------------------------
// The purpose of this funtion is to read a morse code signal. You must send pulses of correct length
// A short from our testing was around .1 seconds to .35
// A long form our testing was around . 

unsigned char getSignal(){
  
  unsigned char value,x,i,b;   // declaring vars
  long timedEvent[8] = {0};   // array to store the timed events 4 pulses so 4 rising 4 falling thus max of 8 events
  unsigned char  pulsed[5] = {0}; // array to store the type of pulse long short or nothing 
  long pulse_width = 0;  // 
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
          pulsed[i] = value; 
       }
       b = search(0,0,pulsed);
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
