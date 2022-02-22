#include "mbed.h"
#include "C12832.h"
#include "QEI.h"





DigitalOut lm_bip (PB_2);                   //left motor bipolar pin
DigitalOut rm_bip (PB_13);                  //right motor bipolar pin
DigitalOut lm_dir (PB_15);                  //left motor direction pin
DigitalOut rm_dir (PB_12);                  //right motor direction pin
PwmOut lm_pwm (PB_1);                       //left motor PWM pin
PwmOut rm_pwm (PB_14);                      //right motor PWM pin






QEI lm_enc (PA_0, PA_1, PA_4, 512);              //initialise left encoder
QEI rm_enc (PB_0, PC_1, PC_0, 512);              //a, b, index, pulses per revolution 

Ticker encoder_ticker;

int lm_enc_cur, rm_enc_cur;                       //variables for storing revolutions completed


void EncoderISR(){                                //ISR for monitoring index value
    
    lm_enc_cur = lm_enc.getPulses();                   //store value of revolutions completed
    rm_enc_cur = rm_enc.getPulses();        
    
    }




int edges=0;              //variable for storing the number of edges travelled
int squares=0;            //variable for storing number of squares drawn




int main (){
    
    encoder_ticker.attach(&EncoderISR, 0.001);
  
  
    lm_pwm.period(0.4f);                    //this sets the period of the pwm cycle, NEED TO CALCULATE A BETTER VALUE!!
    rm_pwm.period(0.4f);                    //period of right motor pwm cycle, value is as given in example
    
    lm_bip = 1;                             //put the motors into bipolar mode
    rm_bip = 1; 
    
    char BoxState = 's';                       //initiate start-up state    
    
    int direction = 1;                          //variable used for all direction in switch statement (initialised for clockwise box and lm is standard)
    
    while(1) {
        
        switch(BoxState) {                     

            case('s'):                                //straight line state

                lm_dir = 1;                        //set direction of motors to forward
                rm_dir = 0;                       
                
                lm_pwm.write(0.75);                //set motor duty cycle to 75%    
                rm_pwm.write(0.75);                //adjust if the buggy doesn't travel straight

                if  (lm_enc_cur >= 15279 && rm_enc_cur >= 15279){                 //checks if buggy has travelled 50cm         (15279 is encoder pulses for ~2revs/50cm) 
                    
                    lm_pwm.write(0);               //switch motors off
                    rm_pwm.write(0);
                    
                    edges++;                       //increment edges travelled
                    
                    lm_enc.reset();                  //reset the encoder count
                    rm_enc.reset();                  
                    
                    if (edges == 4){                //checks for full sqaure completed
                        if (squares ==0 ){           //checks if its the second square completed
                            BoxState = 'c';    //move to change direction state if only first square completed 
                            }
                        else {BoxState = 'd';}           //move to done if the second square has been completed 
                        }
                    else {
                        BoxState = 't';               //nove to turn state
                        }
                    }

                break;

            case('t'):                          //90 degree turn state

                lm_dir = direction;                        //set direction of motors to turn right
                rm_dir = direction;                
                
                lm_pwm.write(0.75);                //set motor duty cycle to 75%    
                rm_pwm.write(0.75);                //adjust if the buggy doesn't turn 90 degrees       

                if  (lm_enc_cur >= 1920 && rm_enc_cur >= 1920){                 //checks if buggy has turn 90 degrees         (1920 is encoder pulses for 0.25revs/90 degrees) 
                    
                    lm_pwm.write(0);               //switch motors off
                    rm_pwm.write(0);
                    
                    lm_enc.reset();                  //reset the encoder count
                    rm_enc.reset();                  
                    
                    BoxState = 's';              //move to striaght state
                    
                    }

                break;

            case('c'):                    //Change direction state

                lm_dir = direction;                        //set direction of motors to turn
                rm_dir = direction;
                
                lm_pwm.write(0.75);                //set motor duty cycle to 75%    
                rm_pwm.write(0.75);                //adjust if the buggy doesn't turn 180 degrees   

                if  (lm_enc_cur >= 3840 && rm_enc_cur >= 3840){                 //checks if buggy has turned 180 degrees         (3840 is encoder pulses for 0.25revs/90 degrees) 
                    
                    lm_pwm.write(0);               //switch motors off
                    rm_pwm.write(0);
                    
                    lm_enc.reset();                  //reset the encoder count
                    rm_enc.reset();                  
                    
                    squares++;                     //increment the number of squares drawn
                    edges = 0;                     //reset the number of edges drawn
                    
                    direction = !direction;         //change turn direction
                    
                    BoxState = 's';           //return to straight to start new square                   
                     
                }

                break;

            case('d'):                //done state

                lm_pwm.write(0);               //switch motors off
                rm_pwm.write(0);

                break;

            default:
                BoxState = 'd';
        }
    }    
}
