//~~~~~~~~~~~~~~~ Switch statement variables ~~~~~~~~~~~~~~~
  char userInput;
  char menuCase = '0';                          // Starts switches at selectionMenu
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



//------------------------------ searchSignal() begin ------------------------------
// Description: Sends a unique signal to the receiver during calibration mode.
// We chose to send a 100 Hz signal with a 30% duty cycle.
//----------------------------------------------------------------------------------
void searchSignal()
{ 
  while(digitalRead(10))
  {            
    digitalWrite(9, HIGH);                      // Sets pin D9 on - Rising edge
    delayMicroseconds(3000);                    // Waits for 3000us - Creating signal w/ 30% duty cycle
             
    digitalWrite(9, LOW);                       // Sets pin D9 off - Falling edge
    delayMicroseconds(7000);                    // Waits for 7000us
  }
}
//------------------------------ searchSignal() end ------------------------------


//------------------------------ repeater() begin ------------------------------
// Description: Repeater using input capture and output compare.
// Takes in button presses from user using input capture.
// Uses output compare in toggle mode to send square signal (laser) to the photodiode.
//------------------------------------------------------------------------------
   
void repeater()
{
  TCCR1A = 0x40;                                       // Set to toggle mode 
  TCNT1 = 0;
  TIFR1 = (1<<ICF1);                                   // Clear ICF (Input Capture flag) flag
  
  while(digitalRead(10))                               // When menu button is not pressed
  {
    //FIRST RISING EDGE (1)
    TCCR1B = 0x40;                                     // Rising edge, no prescaler
    while ((TIFR1&(1<<ICF1)) == 0);
    TCCR1C = 0x80;                                     // Toggle
    TIFR1 = (1<<ICF1);                                 // Clear ICF flag
  
    //FIRST FALLING EDGE (0)
    TCCR1B = 0x00;                                     // Falling edge, no prescaler
    while ((TIFR1&(1<<ICF1)) == 0);
    TCCR1C = 0x80;                                     // Toggle
    TIFR1 = (1<<ICF1);                                 // Clear ICF flag
  }
}
  
//------------------------------ repeater() end ------------------------------


//============================== setup and loop ==============================

void setup()
{
 Serial.begin(9600);
 pinMode(9, OUTPUT);          // Sets pin D9 as output - 
 pinMode(10, INPUT);          // Sets pin D10 as input
}
  
 void loop()
 {
  
 //--------------- menuCase Switch begin ---------------
 
  switch(menuCase){
    //----- (0) selectionMenu case begin -----
    case '0':
      Serial.print('\n');
      Serial.print("--------------------Menu-------------------- \n");
      Serial.print("Please make a selection: \n");
      Serial.print("1. Calibration \n");
      Serial.print("2. Begin transmission \n");
      Serial.print("-------------------------------------------- \n \n");

      while(Serial.available() == 0);     // Waits for user input
      userInput = Serial.read();
      
      if (userInput == ('1'))
      {
        menuCase = '1';                   // searchSignal case (Calibration)
        break;
      }
      else if (userInput == ('2'))
      {
        menuCase = '2';                   // repeater case (Morse Code Transmission)
        break;
      }
      else
      {
        Serial.print("Please enter a valid number from the menu \n");
        menuCase = '0';
        break;
      }
    //----- (0) selectionMenu case end -----

    //----- (1) searchSignal case begin -----
    case '1':
      Serial.print("Outputting search signal \n");
      searchSignal();
      Serial.print("Ending search transmission \n");
      menuCase = '0';
      break;
    //----- (1) searchSignal case begin -----

    //----- (2) repeater case begin -----
    case '2':
      Serial.print("Ready to send morse code \n");
      repeater();
      Serial.print("Morse code transmission sent \n");
      menuCase = '0';
      break;
    //----- (2) repeater case begin -----

  }
 //--------------- menuCase Switch end ---------------
 
}
