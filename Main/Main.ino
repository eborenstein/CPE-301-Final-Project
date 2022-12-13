#include <DHT.h>
#include <DHT_U.h>

#include <RTClib.h>

 #define RDA 0x80
 #define TBE 0x20  


volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
 

volatile unsigned char* port_b = (unsigned char*) 0x25; 
volatile unsigned char* ddr_b  = (unsigned char*) 0x24; 
volatile unsigned char* pin_b  = (unsigned char*) 0x23;
// port e 3
volatile unsigned char* port_e = (unsigned char*) 0x2E; 
volatile unsigned char* ddr_e  = (unsigned char*) 0x2D; 
volatile unsigned char* pin_e  = (unsigned char*) 0x2C;


volatile unsigned char* port_h = (unsigned char*) 0x102;
volatile unsigned char* ddr_h  = (unsigned char*) 0x101; 
volatile unsigned char* pin_h  = (unsigned char*) 0x100;
// d6 step left in - PH3
// d7 step right in - PH4
// d8 fan stop (disable) - PH5

volatile unsigned char* port_l = (unsigned char*) 0x10B;
volatile unsigned char* ddr_l  = (unsigned char*) 0x10A;
volatile unsigned char* pin_l  = (unsigned char*) 0x109;

//Analog to Digital for Water sensor
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

RTC_DS1307 rtc;
DHT dht(A0, DHT11);
void setup() {
  
  U0init(9600);
  *ddr_e |= 0b00001000; //fan motor
  *ddr_h &= 0b11110111; //step right input button
  *ddr_h &= 0b11101111; //step left input button
  *ddr_h &= 0b11011111; //fan stop button (disable)
  *port_h |= 0b00001000; //step right input button set pull high
  *port_h |= 0b00010000; //step left input button set pull high
  *port_h |= 0b00100000; //fan stop button pull high
  //I am aware that by default these pins should be input and pull high, but I figure it's best to actually declare it. 
  //Should also help when looking back at the code. I know right, me, writing code and comments for the future? Ridiculous! -Kyle
  *ddr_l |= 0b00000100; //Yellow LED
  *ddr_l |= 0b00000001; //Blue LED
  *ddr_b |= 0b00000100; //Red LED
  *ddr_b |= 0b00000001; //Green LED

  RTC_init();
  adc_init();
}
//vars
unsigned int temperatureThreshold = 23; //Note - is in C
unsigned int waterThreshold = 10; //same w/ water
unsigned int state = 0; 
//unsigned int temperature = 0; //declaring here. May move once monitoring is added.
//unsigned int water = adc_read(0); //inital water reading. Maybe should just be set high initally? Actually, I'll just call adc_read(0) in the ifs
DateTime now = rtc.now();
void loop() {
  
  
  

  if (state == 0){ //disabled
    led_set(0);
    //fanset(false); //move later to only be on state change? - Done, vestigal
  }

  if (state == 1){ //idle
    led_set(1);
    //fanset(false);
    if (*pin_h & 0b00001000){ //'vent' movement. Happens everywhere but Disabled
    //set step left here (will return to after I acttually look at the library - Kyle)
    Vent_moved();
    }
    if (*pin_h & 0b00010000){
    //step right here - Maybe could'a put this into a function. /shrug
    Vent_moved();
    }
    if (*pin_h & 0b00100000){ //if off button is pressed
      state = 0;
      fanset(false);
    }
    if(adc_read(0) <= waterThreshold){
      state = 2;
      fanset(false);
    }
    
    if(dht.readTemperature() >= temperatureThreshold){
      state = 3;
      fanset(true);
    }
  }

  if (state == 2){ //error
    led_set(2);
    //fanset(false);
    if (*pin_h & 0b00001000){ //'vent' movement. Happens everywhere but Disabled
    //set step left here (will return to after I acttually look at the library - Kyle)
    Vent_moved();
    }
    if (*pin_h & 0b00010000){
    //step right here
    Vent_moved();
    }
    if (*pin_h & 0b00100000){//if off button is pressed
      state = 0;
      fanset(false);
    }
    //if (water >= water) - The doc says "a reset button should trigger a change to the IDLE stage if water level is above threshold"
    //I *assume* this means the actual resent button. Which as I understand, reloads the entire program. Which means this check should be unnecesairy, as everything is reinitalized - Kyle

    //LCD output error
  }

  if (state == 3){ //running
    led_set(3);
    //fanset(true); //may need to edit this later to only hapen on state change, lest the log get spammed
    if (*pin_h & 0b00001000){ //'vent' movement. Happens everywhere but Disabled
    //set step left here (will return to after I acttually look at the library - Kyle)
    Vent_moved();
    }
    if (*pin_h & 0b00010000){
    //step right here
    Vent_moved();
    }
    if (*pin_h & 0b00100000){//if off button is pressed
      state = 0;
      fanset(false);
    }
    if(dht.readTemperature() <= temperatureThreshold){
      state = 1; //goto idle  - hmm, now I kinda want to use gotos, but I think that's disaprooved of in C?
      fanset(false);
    }
    if(adc_read(0) <= waterThreshold){
      state = 2;
      fanset(false);
    }
    
  }


}


void print_int(unsigned int out_num)
{
  // clear a flag (for printing 0's in the middle of numbers)
  unsigned char print_flag = 0;
  // if its greater than 1000
  if(out_num >= 1000)
  {
    // get the 1000's digit, add to '0' 
    U0putchar(out_num / 1000 + '0');
    // set the print flag
    print_flag = 1;
    // mod the out num by 1000
    out_num = out_num % 1000;
  }
  // if its greater than 100 or we've already printed the 1000's
  if(out_num >= 100 || print_flag)
  {
    // get the 100's digit, add to '0'
    U0putchar(out_num / 100 + '0');
    // set the print flag
    print_flag = 1;
    // mod the output num by 100
    out_num = out_num % 100;
  } 
  // if its greater than 10, or we've already printed the 10's
  if(out_num >= 10 || print_flag)
  {
    U0putchar(out_num / 10 + '0');
    print_flag = 1;
    out_num = out_num % 10;
  } 
  // always print the last digit (in case it's 0)
  U0putchar(out_num + '0');
  // print a newline
  U0putchar('\n');
}
void U0init(int U0baud)
{
 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}
unsigned char U0kbhit()
{
  return *myUCSR0A & RDA;
}
unsigned char U0getchar()
{
  return *myUDR0;
}
void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}

void fanset(bool a)
{
  
  if (a)
  {
    U0putchar('o');
    U0putchar('n');
    
    *port_e |= 0b00001000;
  } else {
    U0putchar('o');
    U0putchar('f');
    U0putchar('f');
    *port_e &= 0b11110111;
  }
  put_time();
}

void led_set(int led){
  switch (led){
    case 0:  //led 0 set for Disabled state

      *port_l |= 0b00000100; //D47 |= 1; //Digital 47 = Yellow   PL2
      *port_l &= 0b11111110; //D49 &= 0; //Digtal 49 = Blue      PL0
      *port_b &= 0b11111011; //D51 &= 0; //Digital 51 = Red      PB2
      *port_b &= 0b11111110; //D53 &= 0; //Digital 53 = Green    PB0
      break;
    
    case 1: //idle
      *port_l &= 0b11111011; 
      *port_l &= 0b11111110; 
      *port_b &= 0b11111011; 
      *port_b |= 0b00000001;
      break;
    
    case 2: //error
      *port_l &= 0b11111011; 
      *port_l &= 0b11111110; 
      *port_b |= 0b00000100; 
      *port_b &= 0b11111110;
      break;
    
    case 3: //running
      *port_l &= 0b11111011; 
      *port_l |= 0b00000001; 
      *port_b &= 0b11111011; 
      *port_b &= 0b11111110;
      break;
    
  }
}

void RTC_init(){//checks if RTC connected, sets time
  if(!rtc.begin()){//as far as I can tell, rtc.begin automagically finds what pin the RTC is connected to. Returns false if can't find RTC - Kyle
    U0putchar('n');
    U0putchar('o');
    U0putchar('R');
    U0putchar('T');
    U0putchar('C');
  }
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));//sets date & time from sketch compile. Ripped from arduino RTC tutorial - Kyle
}

void put_time(){//split into it's own function when I considered that it's needed for both the fan motor and step motor
  now = rtc.now();
  U0putchar(' ');
  U0putchar('-');
  U0putchar(' ');
  print_int(now.year());
  U0putchar('/');
  print_int(now.month());
  U0putchar('/');
  print_int(now.day());
  U0putchar(' ');
  print_int(now.hour());
  U0putchar(':');
  print_int(now.minute());
  U0putchar(':');
  print_int(now.second());

}

void Vent_moved(){
  U0putchar('V');
  U0putchar('e');
  U0putchar('n');
  U0putchar('t');
  U0putchar(' ');
  U0putchar('m');
  U0putchar('o');
  U0putchar('v');
  U0putchar('e');
  U0putchar('d');
  put_time();
  U0putchar('\n');
}

void adc_init() //grabbed from lab 8. Because we have Analog 0 being the water sensor, we actually want all the MUX channel selection bits as 0 anyway
{ //(ADCB bit 3 and ADMUX bits 4-0 should be 0)
  // setup the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11101111; // clear bit 4 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11111000; // clear bit 2-0 to 0 to set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  //*my_ADMUX  &=  // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}

unsigned int adc_read(unsigned char adc_channel_num) // also ripped from lab 8. Thinking about it, I'm pretty certain that I could make it only use channel 0 anyway, but... Eh.
{ //It's not like anyone else in the group is going to read this, anyway. - Kyle
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
}
