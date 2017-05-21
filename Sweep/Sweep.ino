
#define __iRows 256

#define PINFREQUP 13
#define PINFREQDN 12
#define PWMOUT 3
#define LCDRS 8
#define LCDEN 9
#define LCD4 4
#define LCD5 5
#define LCD6 6
#define LCD7 7

// define some values used by the panel and buttons
int lcd_key = 0;
int adc_key_in = 0;
#define btnRIGHT 0
#define btnUP 1
#define btnDOWN 2
#define btnLEFT 3
#define btnSELECT 4
#define btnNONE 5

// Servo control code and object
#include <Servo.h>
Servo myservo;  

// include the library code for the LCD
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(LCDRS, LCDEN, LCD4, LCD5, LCD6, LCD7);

// Look up table
const float pi = 3.141592653589793;
int iT = 0;
int iStep = -5;
byte bSync = 0;
double dTable[__iRows];

// Variables related to the buttons
unsigned int iLastUp = HIGH;
unsigned int iLastDn = HIGH;
unsigned int iLast = btnNONE;
unsigned int iReading;   
bool bRun = true;

// Others
float dTemp;

void setup() {

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Pendulum Driver");
  lcd.print("Version 1.1");
  
  // Configure the look up table
  for( iT = 0; iT<256; ++iT){
    dTable[iT] = 0*sin(2*pi*(float)iT/256.0)+90;
  }

  Serial.begin(115200);
  delay(50);
  Serial.println(dTable[0]);
  Serial.println(dTable[63]);
  Serial.println(dTable[127]);
  Serial.println(dTable[191]);
  Serial.println("<-------------------------->");
  
  // configure the ADC input pin
  pinMode(A0, INPUT);

  // configure the button pins
  pinMode(PINFREQUP, INPUT_PULLUP);   
  pinMode(PINFREQDN, INPUT_PULLUP);   

  // attaches the servo on pin 9 to the servo object
  myservo.attach(PWMOUT);  

  // Set timer0 interrupt.
  TCCR0A = 0;   // Set entire TCCR0A register to 0
  TCCR0B = 0;   // Same for TCCR0B

  // See "ISR Frequency Ranges.xlsx" for details
  OCR0A = 75;  
  
  // Turn on the CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS02, CS01 and CS00 bits for 1024 prescaler
  TCCR0B |= (1 << CS02 )|(1 << CS00);
  // Enable the timer compare interrupt
  TIMSK0 |= ( 1 << OCIE0A );
  
  // enable interrupts
  sei();

}

void loop() {


  // Get current state of up pin
  iReading = read_LCD_buttons();

  if( iReading == btnNONE ){
    iLast = btnNONE;
  }

  // Increase frequency (decrease register)
  if( iReading == btnUP && iLast == btnNONE && OCR0A > 10){
    //iLastUp = LOW;
    iLast = btnUP;
    OCR0A = OCR0A-1;
    vUpdateDisp();
  }

  // Decrease frequency (increase register)
  if(iReading == btnDOWN && iLast == btnNONE &&  OCR0A < 255){
    iLast = btnDOWN;
    OCR0A = OCR0A+1;
    vUpdateDisp();
  }

  if( iReading == btnSELECT && iLast == btnNONE ) {
    iLast = btnSELECT;
    bRun = !bRun;
    Serial.println(bRun);
    if( !bRun ) {
      lcd.begin(16, 2);
      lcd.print("Stopped");
    }
    else
    { 
      vUpdateDisp();
    }
  }


}

ISR(TIMER0_COMPA_vect){
  
  if (bRun) {
  // tell servo to go to position
  myservo.write((int)dTable[bSync]);
  bSync++;
  }

  //OCR0A = (  analogRead(A0) >>2 );

}


void vUpdateDisp(){
  Serial.println(OCR0A);
  lcd.clear();
  lcd.print(dCalcFreq(OCR0A), 3);
  lcd.print(" Hz");  
}

float dCalcFreq(int iRegVal){

  dTemp = 15625/((float)OCR0A + 1.0);
  return dTemp/((float)__iRows);

}

// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0); // read the value from the sensor
  
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in > 1000) return btnNONE; 
  if (adc_key_in < 50) return btnRIGHT;
  if (adc_key_in < 195) return btnUP;
  if (adc_key_in < 380) return btnDOWN;
  if (adc_key_in < 555) return btnLEFT;
  if (adc_key_in < 790) return btnSELECT;
  return btnNONE; // when all others fail, return this...
}

