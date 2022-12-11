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
volatile unsigned char* ddr_l = (unsigned char*) 0x10A;
volatile unsigned char* pin_l = (unsigned char*) 0x109;

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
}
//vars
unsigned int temperatureThreshold = 70; //edit once I actually look at how the temp sensor works 
unsigned int waterThreshold = 10; //same w/ water
unsigned int state = 0; 
unsigned int temperature = 0; //declaring here. May move once monitoring is added.
unsigned int water = 0; //again
void loop() {
 //Realtime clock output time of state changes needed.  
if (state == 0){ //disabled
    led_set(0);
    //fanset(false); //move later to only be on state change? - Done, vestigal
  }

  if (state == 1){ //idle
    led_set(1);
    //fanset(false);
    if (*pin_h & 0b00001000){ //'vent' movement. Happens everywhere but Disabled
    //set step left here (will return to after I acttually look at the library - Kyle)
    }
    if (*pin_h & 0b00010000){
    //step right here - Maybe could'a put this into a function. /shrug
    }
    if (*pin_h & 0b00100000){ //if off button is pressed
      state = 0;
      fanset(false);
    }
    if(water <= waterThreshold){
      state = 2;
      fanset(false);
    }
    //need water monitoring
    if(temperature >= temperatureThreshold){
      state = 3;
      fanset(true);
    }
  }

  if (state == 2){ //error
    led_set(2);
    //fanset(false);
    if (*pin_h & 0b00001000){ //'vent' movement. Happens everywhere but Disabled
    //set step left here (will return to after I acttually look at the library - Kyle)
    }
    if (*pin_h & 0b00010000){
    //step right here
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
    }
    if (*pin_h & 0b00010000){
    //step right here
    }
    if (*pin_h & 0b00100000){//if off button is pressed
      state = 0;
      fanset(false);
    }
    if(temperature <= temperatureThreshold){
      state = 1; //goto idle  - hmm, now I kinda want to use gotos, but I think that's disaprooved of in C?
      fanset(false);
    }
    if(water <= waterThreshold){
      state = 2;
      fanset(false);
    }
    //Need water monitoring
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
    //need Real time clock time output still
    *port_e |= 0b00001000;
  } else {
    U0putchar('o');
    U0putchar('f');
    U0putchar('f');
    *port_e &= 0b11110111;
  }
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
      *port_l &= 0b00000001; 
      *port_b |= 0b11111011; 
      *port_b &= 0b11111110;
      break;


  }
}
