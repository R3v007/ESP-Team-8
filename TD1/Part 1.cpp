#include "mbed.h"
#include "C12832.h"
#include "QEI.h"



class Potentiometer                                 //Begin Potentiometer class definition
{
private:                                            //Private data member declaration
    AnalogIn inputSignal;                           //Declaration of AnalogIn object
    float VDD, currentSampleNorm, currentSampleVolts; //Float variables to speficy the value of VDD and most recent samples

public:                                             // Public declarations
    Potentiometer(PinName pin, float v) : inputSignal(pin), VDD(v) {}   //Constructor - user provided pin name assigned to AnalogIn...
    //VDD is also provided to determine maximum measurable voltage
    float amplitudeVolts(void)                      //Public member function to measure the amplitude in volts
    {
        return (inputSignal.read()*VDD);            //Scales the 0.0-1.0 value by VDD to read the input in volts
    }

    float amplitudeNorm(void)                       //Public member function to measure the normalised amplitude
    {
        return inputSignal.read();                  //Returns the ADC value normalised to range 0.0 - 1.0
    }

    void sample(void)                               //Public member function to sample an analogue voltage
    {
        currentSampleNorm = inputSignal.read();       //Stores the current ADC value to the class's data member for normalised values (0.0 - 1.0)
        currentSampleVolts = currentSampleNorm * VDD; //Converts the normalised value to the equivalent voltage (0.0 - 3.3 V) and stores this information
    }

    float getCurrentSampleVolts(void)               //Public member function to return the most recent sample from the potentiometer (in volts)
    {
        return currentSampleVolts;                  //Return the contents of the data member currentSampleVolts
    }

    float getCurrentSampleNorm(void)                //Public member function to return the most recent sample from the potentiometer (normalised)
    {
        return currentSampleNorm;                   //Return the contents of the data member currentSampleNorm
    }

};








DigitalOut lm_bip (PB_2);                   //left motor bipolar pin
DigitalOut rm_bip (PB_13);                  //right motor bipolar pin
DigitalOut lm_dir (PB_15);                  //left motor direction pin
DigitalOut rm_dir (PB_12);                  //right motor direction pin
PwmOut lm_pwm (PB_1);                       //left motor PWM pin
PwmOut rm_pwm (PB_14);                      //right motor PWM pin

C12832 lcd(D11, D13, D12, D7, D10);


Potentiometer rm_pot(A1, 19);               //potentionmeter to be used for choosing speed 
Potentiometer lm_pot(A0, 19);               //potentionmeter range value chosen as double voltage range +1 (for off)

float lm_pot_hold, rm_pot_hold;             //variables for holding the value from the potentiometers 




QEI lm_enc (PA_0, PA_1, PA_4, 512);              //initialise left encoder
QEI rm_enc (PB_0, PC_1, PC_0, 512);              //a, b, index, pulses per revolution 
                                            
Ticker encoder_ticker;                      

int lm_enc_prev, lm_enc_cur, rm_enc_prev, rm_enc_cur;           //variables for storing current and previous encoder values


float lw_rpm, rw_rpm;                   //variables for storing rpm of the individual WHEELS (w not m in variable name)
float lw_ms, rw_ms;                     //variables for storing ms of the individual WHEELS (w not m in variable name)
float speed_trans, speed_rot;                //variables for translational velocity and rotational velocity

void EncoderISR(){                          //ISR for calculating wheel speeds
    
    lm_enc_cur = lm_enc.getPulses();        //store current tick count
    rm_enc_cur = rm_enc.getPulses(); 
    
    lw_rpm = ((60*(lm_enc_prev - lm_enc_cur))/(0.001*15*512));      //calculate rpm of left wheel in RPM using value from encoder       
    rw_rpm = ((60*(rm_enc_prev - rm_enc_cur))/(0.001*15*512));      //rpm = (60 (convert to mins) * ticks passed)/(0.001(time peiod) * 15(gear ratio) * 256(encoder ticks per rev)) 
    
    lw_ms = (0.08 * 3.1415926535 * lw_rpm) / 60;        //calculate the speed of the left wheel in ms, maybe use a pi function
    rw_ms = (0.08 * 3.1415926535 * rw_rpm) / 60;        //ms = (wheel circumfrence * rpm) / 60 (convert to seconds)
    
    lm_enc_prev = lm_enc_cur;               //store current value in previous
    rm_enc_prev = rm_enc_cur;        
    
    }






int main (){
    
    encoder_ticker.attach(&EncoderISR, 0.001);    // initialise the encoder, sets the sampling rate to 10 Hz

    
    lm_pwm.period(0.4f);                    //this sets the period of the pwm cycle, NEED TO CALCULATE A BETTER VALUE!!
    rm_pwm.period(0.4f);                    //period of right motor pwm cycle, value is as given in example
    
    lm_bip = 1;                             //put the motors into bipolar mode
    rm_bip = 1; 
    
    
     
    
    
    while (1){
        
        lm_pot_hold = lm_pot.getCurrentSampleVolts();         //samples the potentiometer and stores the value
        rm_pot_hold = rm_pot.getCurrentSampleVolts();
        
      
        
        
        
        if (lm_pot_hold < 9){                               //check left potentiometer for reverse position
            
            lm_dir = 0;                                     //set direction of left motor (0 is assumed to be reverse)
            
            lm_pwm.write((9.00 - lm_pot_hold)/9.00);            //set the duty ratio as the position of the potentiometer as a percentage of max position
            
            }
        else if (lm_pot_hold > 10){                          //check left potentiometer for forward position
            
            lm_dir = 1;                                     //set direction of left motor (1 is assumed to be forward)
            
            lm_pwm.write((lm_pot_hold-10.00)/9.00);             //set the duty ratio to the position of the potentiometer as a percentage of max position
        }
        else {                                            //hopefully this is a small range inbetween forward and reverse where the motor is off
            lm_pwm.write(0);                                //set the duty ratio to 0 i.e turn the motor off
            }
        
        
        //RIGHT MOTOR CONTROL, SAME AS LEFT BUT THE DIRECTION IS OPPOSITE
        
        
        
        if (rm_pot_hold < 9){                              
            
            rm_dir = 1;                                   
            
            rm_pwm.write((9.00 - rm_pot_hold)/9.00);           
            
            }
        else if (lm_pot_hold > 10.00){                          
            
            rm_dir = 0;                                
            
            rm_pwm.write((rm_pot_hold-10.00)/9.00);        
        }
        else {                                            
            rm_pwm.write(0);                               
            }
                    
        
        lcd.locate(0,0);
        lcd.printf("Left Wheel Speed: %.2f m/s\n", lw_ms);       //text displaying the speed                     
        lcd.printf("Right Wheel Speed: %.2f m/s\n", rw_ms);      

        
        }
    
    
}
