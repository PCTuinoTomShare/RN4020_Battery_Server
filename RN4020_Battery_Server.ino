
// Connect to Microchip RN4020 bluetooth LE module, presetting :
// - buadrate 9600pbs.
// - peripheral.
// - No auto advertisement.
// - Not support MLDP.
// - Hard wake connect to VDD directly. 
// - Soft wake connect to VDD directly.
// - MLDP/CMD connect to GND directly.
#define MLDP     13  // RN4020 MLDP control pin.
#define HW_WAKE  12  // RN4020 hardware wake control pin.
#define SW_WAKE  11  // RN4020 software wake control pin.
#define CONN_LED 8   // RN4020 connect LED.
#define BTN1_IN 4    // Button input #1.
#define BTN2_IN 3    // Button input #2. 
#define ANALOG_IN 0  // Analog input.

// RN4020 command, start advertisement.
const char cmd_advertisement[2] = {'A','\r'};
// RN4020 command, write local device UUID.
//const char cmd_local_uuid[9] = {'S','U','W',',','2','A','1','9',','};

char battery_level_text[12];       // Battery level character.
unsigned char temp1;              // Temporary #1.
unsigned char btn_cur;            // Current button input.
unsigned char btn_pre;            // Previous button input.
unsigned char btn_val;            // Button input.
unsigned char battery_delta;      // Button adjust flag.
unsigned char temp2;              // Temporary #2. 
unsigned char batttery_level;     // Battery level. 
unsigned long ms_cur;             // Minisecond time counter current.
unsigned long ms_pre;             // Minisecond time counter previous.
unsigned long ms_10ms_past;       // Minisecond time 100ms past.
unsigned long ms_1000ms_past;     // 1 second past.
unsigned long ms_30ms_past;       // 30ms past.
unsigned long analog_in;          // Analog input value.

void setup() {
  // put your setup code here, to run once:
  // IO port initialize.
  pinMode( MLDP, OUTPUT );
  pinMode( HW_WAKE, OUTPUT );
  pinMode( SW_WAKE, OUTPUT );  
  pinMode( CONN_LED, INPUT );
  pinMode( BTN1_IN, INPUT );
  pinMode( BTN2_IN, INPUT );  
  
  //analogReference( AR_DEFAULT );
    
  Serial5.begin( 9600 ); // Arduino UART interface.  
  Serial.begin( 9600 );

  // RN4020 control.
  digitalWrite( MLDP, LOW );        // Command mode.
  digitalWrite( HW_WAKE, HIGH );    // Hardware wake.
  digitalWrite( SW_WAKE, HIGH );    // Software wake.

  // Power on delay.
  delay( 100 );
  // Not connect.  
  if( digitalRead( CONN_LED ) == LOW ){
    Serial5.write( cmd_advertisement );
  }
  // Waiting for connect.
  while( digitalRead( CONN_LED ) == LOW ){}

  delay(100);
  // Variable reset.
  batttery_level = 50;
  
  battery_level_text[0] = 'S';  
  battery_level_text[1] = 'U';  
  battery_level_text[2] = 'W';  
  battery_level_text[3] = ',';    
  battery_level_text[4] = '2';  
  battery_level_text[5] = 'A';  
  battery_level_text[6] = '1';  
  battery_level_text[7] = '9';  
  battery_level_text[8] = ',';   
  battery_level_text[11] = '\r';
}

void loop() {
  // put your main code here, to run repeatedly:
  ms_cur = millis();  
  ms_10ms_past = ms_cur;
  ms_10ms_past -= ms_pre;
  if( ms_10ms_past > 10 ){
    // About 10ms past.
    // Increase 30ms past counter.
    ++ms_30ms_past;
    if( ms_30ms_past == 3 ){     
      // About 30ms past.
      ms_30ms_past = 0;     
      // Button #1.
      btn_cur = 0x00;
      if( digitalRead( BTN1_IN ) == LOW ){ // Pressed.
        btn_cur = 0x01;
      }
      // Button #2.
      btn_cur &= 0xfd;
      if( digitalRead( BTN2_IN ) == LOW ){ // Pressed.
        btn_cur |= 0x02;
      }
      // Combinet with previous.
      btn_val = btn_cur;
      btn_val &= btn_pre; 
      // Keep as previous.
      btn_pre = btn_cur;
      
      if( btn_val == 0x01 ){ // Adjust up.          
          battery_delta = 1;
      } 
      else if( btn_val == 0x02 ){ // Adjust down.
          battery_delta = 2;
      }
      else if( btn_val == 0x00 ){ // No press or release.      
        if( battery_delta == 1 ){
           if( batttery_level < 100 ){
               ++batttery_level;         
           }                
        }
        else if( battery_delta == 2 ){
           if( batttery_level  > 0 ){
               --batttery_level;         
           }                
        }        
        battery_delta = 0;
        
        // ADC input.
        analog_in = analogRead( ANALOG_IN );
        analog_in *= 100;
        analog_in /= 1023;        
        if( analog_in > 100 ){
          analog_in = 100;
        }
        
      }      
    }
    
    // Increase 1s past counter.
    ++ms_1000ms_past;
    if( ms_1000ms_past == 100 ){
       // About 1 second past.
       // To character,
       // 16^1 digit.
       ms_1000ms_past = 0;       
       temp2 = batttery_level;
       temp2 >>= 4;
       temp1 = (unsigned char)temp2;
       ToCharacter();
       battery_level_text[9] = temp1;
       // 16^0 digit.
       temp2 = batttery_level;
       temp2 &= 0x0f;
       temp1 = (unsigned char)temp2;
       ToCharacter();
       battery_level_text[10] = temp1;              
       // Command and data output.      
       if( digitalRead( CONN_LED ) == HIGH ){       
           Serial5.write( battery_level_text );           
           Serial.write( battery_level_text );
       }
    }
    // Keep current value as previous.
    ms_pre = ms_cur;
  }  
}

// to character.
void ToCharacter( void ){ 
 if( temp1 < 10 ){
    temp1 += 0x30;  
    return;
 }  
 temp1 -= 10;
 temp1 += 0x41;  
}

