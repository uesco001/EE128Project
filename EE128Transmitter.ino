//~~~~~~~~~~~~~~~ Switch statement variables ~~~~~~~~~~~~~~~
  char userInput;
  char menuCase = '0';
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



//------------------------------ searchSignal(a) begin ------------------------------
void searchSignal()
{ 
  while(digitalRead(10))
  {
    PORTB = 0x02;              
    //digitalWrite(9, HIGH);                      // Sets pin D9 on
    delayMicroseconds(3000);                    // Waits for 10ms - Creating signal w/ 100Hz frequency
    PORTB = 0x00;              
    //digitalWrite(9, LOW);                       // Sets pin D9 off
    delayMicroseconds(7000);                    // Waits for 10ms - Creating signal w/ 100Hz frequency
  }
}
//------------------------------ searchSignal() end ------------------------------


//------------------------------ repeater() begin ------------------------------
   
void repeater()
{
  TCCR1A = 0x40;                                       // Set to toggle mode 
  TCNT1 = 0;
  TIFR1 = (1<<ICF1);                                   // Clear ICF (Input Capture flag) flag
  
  while(digitalRead(10))
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
 DDRB = 0x02;                   // Sets pin D9 as output
 Serial.begin(9600);
 //pinMode(9, OUTPUT);          // Sets pin D9 as output
 //pinMode(10, INPUT);          // Sets pin D10 as input
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

      while(Serial.available() == 0);
      userInput = Serial.read();
      
      if (userInput == ('1'))
      {
        menuCase = '1';
        break;
      }
      else if (userInput == ('2'))
      {
        menuCase = '2';
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
