
#define __iRows 256

#define PINFREQUP 13
#define PINFREQDN 12
#define PWMOUT 11
#define LCDRS 10
#define LCDEN 9
#define LCD4 4
#define LCD5 5
#define LCD6 6
#define LCD7 7

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
unsigned int iReading;   

// Others
float dTemp;

void setup() {

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Pendulum Driver");
  
  // Configure the look up table
  for( iT = 0; iT<256; ++iT){
    dTable[iT] = 50*sin(2*pi*(float)iT/256.0)+82;
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
  // Set CS02, CS01 and CS00 bits for 256 prescaler
  TCCR0B |= (1 << CS02 )|(1 << CS00);
  // Enable the timer compare interrupt
  TIMSK0 |= ( 1 << OCIE0A );
  
  // enable interrupts
  sei();

}

void loop() {


  // Get current state of up pin
  iReading = digitalRead(PINFREQUP);

  // Increase frequency (decrease register)
  if( iReading == LOW && iLastUp == HIGH && OCR0A > 10){
    iLastUp = LOW;
    OCR0A = OCR0A-1;
    Serial.println(OCR0A);
    lcd.begin(16, 2);
    lcd.print(dCalcFreq(OCR0A), 3);
    lcd.print(" Hz");
  }
  if( iReading == HIGH && iLastUp == LOW ){
    iLastUp = HIGH;
  }

  // Get current state of down pin
  iReading = digitalRead(PINFREQDN);

  // Decrease frequency (increase register)
  if(iReading == LOW && iLastDn == HIGH &&  OCR0A < 255){
    iLastDn = LOW;
    OCR0A = OCR0A+1;
    Serial.println(OCR0A);
    lcd.begin(16, 2);
    lcd.print(dCalcFreq(OCR0A), 3);
    lcd.print(" Hz");
  }
  if( iReading == HIGH && iLastDn == LOW ){
    iLastDn = HIGH;
  }


}

ISR(TIMER0_COMPA_vect){
  
  // tell servo to go to position
  myservo.write((int)dTable[bSync]);
  bSync++;

  //OCR0A = (  analogRead(A0) >>2 );

}

float dCalcFreq(int iRegVal){

  dTemp = 15625/((float)OCR0A + 1.0);
  return dTemp/((float)__iRows);

}

