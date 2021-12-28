#include <hidef.h>      /* common defines and macros */
#include <mc9s12dg256.h>     /* derivative information */
#pragma LINK_INFO DERIVATIVE "mc9s12dg256b"

#include "main_asm.h" /* interface to the assembly module */

char* enjoy;                   //to display when the counter is done
char* userPrompt;              //prompt user to enter input
char* userPromptTwo;           //to display user entry
char* invalidPrompt;           //invalid prompt
char* seconds;                 //to display seconds during count
char* degrees;                 //to display with temperature
char locked[15] = {            //locked message for terminal
  'D', 'o', 'o', 'r', ' ', 'N', 'o', 'w', ' ', 'L', 'o', 'c', 'k', 'e', 'd'    
};
char cooking[19] = {           //cooking message for terminal
  'C', 'o', 'o', 'k', 'i', 'n', 'g', ' ', 'I', 'n', ' ', 'P', 'r', 'o', 'g', 'r', 'e', 's', 's'
};
char unlocked[17] = {          //unlocked message for terminal
  'D', 'o', 'o', 'r', ' ', 'N', 'o', 'w', ' ', 'U', 'n', 'l', 'o', 'c', 'k', 'e', 'd'  
};
char cookDone[12] = {          //done cooking message for terminal
  'C', 'o', 'o', 'k', 'i','n','g', ' ', 'D', 'o', 'n', 'e'  
};
char aborted[16] = {           //aborted message for terminal
  'C', 'o', 'o', 'k', 'i','n','g', ' ', 'A', 'b', 'o', 'r', 't', 'e', 'd', '!'  
};
char enterKey[20] = {          //enter time message for terminal
  'E', 'n', 't', 'e', 'r', ' ', 'T', 'i', 'm', 'e', ' ', 'O', 'n', ' ', 'K', 'e', 'y', 'p', 'a', 'd'  
};
char invalid[13] = {           //invalid message for terminal
  'I', 'n', 'v', 'a', 'l', 'i', 'd', ' ', 'E', 'n', 't', 'r', 'y'  
};
int ledPulse;                  //for blinking LED at end
int blinkCount;                //for blinking LED at End
int pitch;                     //pitch to be assigned to pitch a, 440 Hz
int userIn;                    //to be assigned to keypad input
int temperature;               //to assign to value of measured temperature
int rotateCount;               //used in the for loop within rotatePlaten
int sw5;                       //variable for switch 5
int abortFlag;                 //to determine if endDisplay is activated
int width;                     //for servo
int wordCount;                 //for outputting to terminal

void userInput();              //gets user input from keypad
void latchDoor();              //latching door
void tempCheck();              //to check temperature
void rotatePlaten();           //to rotate platen
void unlatchDoor();            //unlatching door
void endDisplay();             //display when food is ready
void abortCount();             //to stop mid count
void outSCI(int select);       //to output to terminal. select determines which message to use

void interrupt 7 handler(){      //RTI Interrupt Service Routine

  if(userIn >= 1){
    set_lcd_addr(0x00);          //setting address of LCD
    write_int_lcd(userIn);       //displaying count during countdown
    type_lcd(seconds);           //output with count
    tempCheck();                 //to display temperature
    rotatePlaten();              //rotating platen during countdown
    tempCheck();                 //to display temperature
    abortCount();                //checking if count should abort
  }
  clear_RTI_flag();              //clear RTI flag
  userIn--;                      //decrementing count
  if(!userIn){                   //continuously check if countdown is done
    PORTB = 0;                   //turning off LEDs
    set_lcd_addr(0x00);          //setting address of LCD
    write_int_lcd(userIn);       //displaying count during countdown
    type_lcd(seconds);           //output with count
    tempCheck();                 //to display temperature
    ms_delay(1000);              //delay 1 s
    RTI_disable();               //disabling RTI
  }
  
}

void interrupt 13 handlerA(){  //for using buzzer

  tone(pitch);                 //set tone with value assigned to pitch
  
} 

void main(void) {
  
  PLL_init();         // set system clock frequency to 24 MHz 
  DDRB  = 0xff;       // Port B is output
  DDRJ  = 0xff;       // Port J is output
  DDRP  = 0xff;       // Port P is output
  PTJ = 0x00;         // enable LED
  PTP = 0x00;         // enable all 7-segment displays
  lcd_init();         //enable lcd
  keypad_enable();    //enable the keypad
  ad0_enable();       //enable a/d converter
  SW_enable();        //enable switches
  servo76_init();     //enable pwm1 for servo
  SCI0_init(9600);    //initialize SCI0 at 9600 baud

  while(1){           //loop until powered off
    
    userInput();
    latchDoor();
    outSCI(2);        //outputting message to terminal
    RTI_init();
    ms_delay(500);    //delay 500 ms
    
  
    if(abortFlag){    //checking if endDisplay deactivated from abortCount
      endDisplay(); 
    }
    
    unlatchDoor();
    
  }//end while forever
  
}//end main

void userInput(){

  userIn = -1;                          //initialize to -1 for use with loop
  userPrompt = "Enter Time:";            //prompt message
  userPromptTwo = "You Entered:";       //another message for showing entry
  invalidPrompt = "Invalid Entry:";     //outputting invalid prompt
  seconds = " seconds";                 //assigning to " seconds" for count output
  set_lcd_addr(0x00);                   //setting address of LCD
  type_lcd(userPrompt);                 //outputting prompt
  outSCI(6);                            //outputting message to terminal
  
  while(userIn == -1){                  //looping until key pressed
    userIn = getkey();
    if(((userIn < 1)||(userIn > 9))
                   &&(userIn != -1)){   //invalid entry detection..should I do just >9?
      set_lcd_addr(0x00);               //setting address of LCD
      type_lcd(invalidPrompt);          //outputting prompt
      set_lcd_addr(0x49);               //setting address of LCD
      write_int_lcd(userIn);            //outputting user input
      outSCI(7);                        //outputting message to terminal
      userIn = -1;                      //reinitiliaze to -1
      ms_delay(2000);                   //delay 3 s
      clear_lcd();                      //clearing LCD
      set_lcd_addr(0x00);               //setting address of LCD
      type_lcd(userPrompt);             //outputting prompt
      outSCI(6);                        //outputting message to terminal  
    }
  }
  clear_lcd();                          //clearing LCD
  ms_delay(200);                        //delay 200 ms
  set_lcd_addr(0x00);                   //setting address of LCD
  type_lcd(userPromptTwo);              //outputting prompt
  set_lcd_addr(0x49);                   //setting address of LCD
  write_int_lcd(userIn);                //outputting user input
  abortFlag = 1;                        //to allow endDisplay
  ms_delay(2000);                       //delay 3 s
  clear_lcd();                          //clearing LCD
  
}//end promptUser

void latchDoor(){

    for(width = 4500; width <= 6000; width = width + 5){   //to left
      set_servo76(width);                                  //move servo from 4500 to 6000
      ms_delay(5);                                         //quick delay
    }
    outSCI(1);                                             //outputting message to terminal
  
}//end latchDoor

void tempCheck(){                            //using thermistor

  degrees = " deg F";                        //to diplay next to temperature
  temperature = -(ad0conv(2))-64937;         //read temp sensor on channel 5,flip reading,then offset to match room temp
  set_lcd_addr(0x40);                        //display on second row of LCD
  write_int_lcd(temperature);                //output temperature to LCD
  type_lcd(degrees);                         //write deg F next to temperature
  
}//end tempCheck

void rotatePlaten(){                  

  int i;                                //variable in for loop
  PORTB = 3;                            //set PORTB to 3
  ms_delay(2);                          //quick delay
  for(rotateCount = 0; i < 300; i++){   //this loop will cycle PORTB values to spin motor 
    PORTB ^= 5;                         //xor PORTB with 5
    ms_delay(2);                        //quick delay
    PORTB ^= 10;                        //xor PORTB with 10
    ms_delay(2);                        //quick delay
  }
  PORTB = 9;                            //PORTB to 9 to finish off cycle
  ms_delay(1);                          //quick delay
  
}//end rotatePlaten

void unlatchDoor(){

  for(width = 6000; width >= 4500; width = width - 5){    //to right
    set_servo76(width);                                   //move servo from 6000 to 4500
    ms_delay(5);                                          //quick delay
  }
  outSCI(3);                                              //outputting message to terminal
  
}//end unlatchDoor

void endDisplay(){          

  enjoy = "Enjoy";          //to output to LCD
  ledPulse = 128;           //to blink last LED
  blinkCount = 2;           //assigning 2 makes 3 beeps
  clear_lcd();              //clearing LCD screen
  set_lcd_addr(0x00);       //setting address of LCD
  type_lcd(enjoy);          //outputting to LCD
  outSCI(4);                //outputting message to terminal
  pitch = 1705;             //this is 440Hz

  while(blinkCount >= 0){   //for pulsing
    PORTB ^= ledPulse;      //exoring to blink leds
    sound_init();           //init pulse train interrupts
    sound_on();             //enables timer and interrupts to turn sound
    ms_delay(500);          //delay 200 ms
    sound_off();            //disables...turns off sound
    ms_delay(300);          //delay 300 ms
    blinkCount--;           //decrement blinkCount
  }
  PORTB = 0;                //shutting off when done blinking
  ms_delay(500);            //delay half second before clearing LCD
  clear_lcd();              //clearing LCD when done
  
}//end endDisplay

void abortCount(){

  sw5 = 1;                  //assigning to value 1 which corresponds to switch 5 on port H
  
  if(!(PTH&sw5)){           //checking if switch 5 is pressed
    clear_lcd();            //clearing LCD screen
    set_lcd_addr(0x00);     //setting address of LCD
    type_lcd("Abort!");     //displaying on LCD
    outSCI(5);              //outputting message to terminal
    rotateCount = 0;        //to stop rotation
    abortFlag = 0;          //to stop endDisplay
    PORTB = 0;              //turning off LEDs
    ms_delay(1500);         //delay 1.5 seconds
    RTI_disable();          //disabling RTI
  } 
  else{
    rotatePlaten();         //rotating platen during countdown
  }
  
}//end abortCount

void outSCI(int select){

  wordCount = 0;                        //initialize wordCount to 0

  switch(select){
    case 1:                             //if select is 1, prints locked array to terminal
      while(wordCount < 15){            //loop while less than 15
        outchar0(locked[wordCount]);    //prints out each character of array to terminal
        wordCount++;                    //increment wordCount
      }
      wordCount = 0;                    //reinitialize wordCount to 0
      outchar0(10);                     //outputting carriage return to terminal
      break;                            //case done
    case 2:                             //if select is 2, prints cooking array
      while(wordCount < 19){            //loops while less than 19
        outchar0(cooking[wordCount]);   //prints out each character of array to terminal
        wordCount++;                    //increment wordCount  
      }
      wordCount = 0;                    //reinitialize wordCount to 0
      outchar0(10);                     //outputting carriage return to terminal
      break;                            //case done
    case 3:                             //if select is 3, prints unlocked array to terminal
      while(wordCount < 17){            //loops while less than 17
        outchar0(unlocked[wordCount]);  //prints out each character of array to terminal
        wordCount++;                    //increment wordCount  
      }
      wordCount = 0;                    //reinitialize wordCount to 0
      outchar0(10);                     //outputting carriage return to terminal
      break;                            //case done
    case 4:                             //if select is 4, prints cookDone array to terminal
      while(wordCount < 12){            //loop while less than 12
        outchar0(cookDone[wordCount]);  //prints out each character of array to terminal
        wordCount++;                    //increment wordCount  
      }
      wordCount = 0;                    //reinitialize wordCount to 0
      outchar0(10);                     //outputting carriage return to terminal
      break;                            //case done
    case 5:                             //if select is 5, prints aborted array to terminal
      while(wordCount < 16){            //loop while less than 16
        outchar0(aborted[wordCount]);   //prints out each character of array to terminal
        wordCount++;                    //increment wordCount  
      }
      wordCount = 0;                    //reinitialize wordCount to 0
      outchar0(10);                     //outputting carriage return to terminal
      break;
    case 6:                             //if select is 6, prints enterKey array to terminal
      while(wordCount < 20){            //loop while less than 20
        outchar0(enterKey[wordCount]);  //prints out each character of array to terminal
        wordCount++;                    //increment wordCount  
      }
      wordCount = 0;                    //reinitialize wordCount to 0
      outchar0(10);                     //outputting carriage return to terminal
      break;
    case 7:                             //if select is 7, prints invalid array to terminal
      while(wordCount < 13){            //loop while less than 13
        outchar0(invalid[wordCount]);   //prints out each character of array to terminal
        wordCount++;                    //increment wordCount  
      }
      wordCount = 0;                    //reinitialize wordCount to 0
      outchar0(10);                     //outputting carriage return to terminal
      break;
  }
  
}//end outSCI
