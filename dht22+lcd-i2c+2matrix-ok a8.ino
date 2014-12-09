
#define Version "0.994a14"


// This #include statement was automatically added by the Spark IDE.
#include "SparkTime/SparkTime.h"

// This #include statement was automatically added by the Spark IDE.
#include "pulse.h"

// // This #include statement was automatically added by the Spark IDE.
//#include <stdint.h>
#include <application.h>
#include "i2c.h"
#include "dht22.h"

#include "ledcontrol.h"

// This #include statement was automatically added by the Spark IDE.
//#include "dht22.cpp"
// This #include statement was automatically added by the Spark IDE.

// This #include statement was automatically added by the Spark IDE.
//#include "nouse.h"

// Use LCD I2C backpack
// SDA --> D0
// SCL --> D1
// VCC --> external 5V
//GND --> GND
// id = 0x27, declared in setup();

// VCC - 3V3
// GND - GND
// DIN - A0
// CS - A1
// CLK - A2

int led=7;
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))


#define XIVELY_API_KEY "4KLae7f8WIhUdUJvF0JxVZ42sWuBWE8VJHt9UJua9PuZP8uS"
#define FEED_ID "19367269"


#define CHECK_DHT_PERIOD    10*60   
#define CHECK_PIR_PERIOD    5       

#define pirLEDPin D6
#define pirPin D3

// SRF-05
const int numOfReadings = 3;                   // number of readings to take/ items in the array
int readings[numOfReadings];                    // stores the distance readings in an array
int arrayIndex = 0;                             // arrayIndex of the current item in the array
int total = 0;                                  // stores the cumlative total
int averageDistance = 0;                        // stores the average value

// setup pins and variables for SRF05 sonar device

// #define echoPin  D3                                // SRF05 echo pin (digital 2)
// #define initPin  D2                                // SRF05 trigger pin (digital 3)

#define breathLED D5

unsigned long pulseTime = 0;                    // stores the pulse in Micro Seconds
unsigned long distance = 0;                     // variable for storing the distance (cm)
#define SRFThreshold 38
#define SRFMAX  50

// setup pins/values for LED

#define redLEDPin  D6  //A4                              // Red LED, connected to digital PWM pin 9
int redLEDValue = 0;                            // stores the value of brightness for the LED (0 = fully off, 255 = fully on)
// int distanceCount=0;
// int distanceAcc=0;

//setup

int ledIntensity = 15;                          


void print(String str) {
    Serial.print(str);
}
void println(String str) {
    Serial.print(str);
    Serial.print("\n");
}


TCPServer server = TCPServer(8081);

//--------------------------------------
//TCPClient client;

TCPClient client;




void sendToXivelyWithLed(int led, float temperature, float humidity, int pirMove,int distance){
    int r = sendToXively(temperature, humidity, pirMove,distance);
    if (r==0){
        ledStatus(led, 1, 100);    
    } else {
        ledStatus (led, 2,1000);
    }
}

int sendToXively(float temperature, float humidity, int pirMove,int distance){
    //TCPClient client = server.available();
    int result=0;
    statusPrint("ConnToX...");
    
    client.flush();
    //temperature = getTemperature();
    if (client.connect("api.xively.com", 8081)) 
    {
        statusPrint("Start send");
        

        delay(500);
        client.print("{");
        client.print("  \"method\" : \"put\",");
        client.print("  \"resource\" : \"/feeds/");
        client.print(FEED_ID);
        client.print("\",");
        client.print("  \"params\" : {},");
        client.print("  \"headers\" : {\"X-ApiKey\":\"");
        client.print(XIVELY_API_KEY);
        delay(500);
        client.print("\"},");
        client.print("  \"body\" :");
        client.print("    {");
        client.print("      \"version\" : \"1.0.0\",");
        client.print("      \"datastreams\" : [");
        
        client.print("        {");
        client.print("          \"id\" : \"Humidity\",");
        client.print("          \"current_value\" : \"");
        client.print(humidity);
        delay(500);
        client.print("\"");
        client.print("        },");
        
        client.print("        {");
        client.print("          \"id\" : \"temperature\",");
        client.print("          \"current_value\" : \"");
        client.print(temperature);
        delay(500);
        client.print("\"");
        client.print("        },");
        
        //client.print("    {");
        //client.print("          \"id\" : \"SRF\",");
        //client.print("          \"current_value\" : \"");
        //client.print(distance);
        //delay(500);
        //client.print("\"");
        //client.print("        },");
        
        client.print("        {");
        client.print("          \"id\" : \"PIRsensor\",");
        client.print("          \"current_value\" : \"");
        client.print(pirMove);
        delay(500);
        client.print("\"");
        client.print("        }");
       
        client.print("      ]");
        client.print("    },");
        client.print("  \"token\" : \"0x12345\"");
        delay(500);
        client.print("}");
        client.println();
      
        statusPrint("SendDone");
        println("Connect Close");
        result=0;
    } 
    else 
    {
        statusPrint("Fail");
        // Connection failed
        //Serial.println("connection failed");
        result=1;
    }


    if (client.available()) 
    {
        // Read response
        
        char c= client.read();
        String str = "Client:" +c;
        statusPrint("Response from client");
        
        Serial.print(c,HEX);
    }

    if (!client.connected()) 
    {
        Serial.println();
        statusPrint("disc X Stop");
        client.stop();
    }

  
    client.stop();
    return result;
   // delay (2000);  // delete 2014 10 04
}

//============================================
void ledStatus(int ledPin, int x, int t)
{
    //println("led pulse");
    for (int j = 1; j <= x; j++)
    {
        digitalWrite(ledPin, 1);
        delay(t);
        digitalWrite(ledPin, 0);
        delay(t); 
   }
}


/*
 pin 12 is connected to the DataIn
 pin 11 is connected to the CLK
 pin 10 is connected to LOAD
//  */
// VCC - 3V3
// GND - GND
// DIN - A0
// CS - A1
// CLK - A2
//data load clock num
//                   DIN, CLK, LOAD, #chips 

#define LedMatrics 6
LedControl *lc;

LiquidCrystal_I2C *lcd;


DHT dht(DHTPIN, DHTTYPE);


float h;    // humidity
float t;    // temperature
int f = 0;  // failed?


unsigned long lastUp;
unsigned long lastMove;
unsigned long lastMoveUp;
int pirMove=0;
char loopStr[]="|\\-/!@#$%^&*";
int loopIdx = 0;
int dataSent=0;
char buf[65];

unsigned long lcdBacklightDelay=15;   // trun off after 15 sec.
unsigned long lcdBacklightLastUp=millis();

//sparkTime rtc;
UDP UDPClient;
SparkTime rtc;

unsigned long currentTime;
unsigned long lastTime = 0UL;
String timeStr;
String newTimeStr;

#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
unsigned long lastSync = millis();

void setup() {
    
   //rtc.begin(&UDPClient, "3.tw.pool.ntp.org");
   //rtc.setTimeZone(+8); // gmt offset
//   Serial.begin(9600);
  
  lcd = new LiquidCrystal_I2C(0x27, 20,4);
  
    lcd->init();                      // initialize the lcd
    lcd->backlight();
    lcd->clear();
  
  
  lc = new LedControl(A0,A2,A1,LedMatrics);
  for (int i=0; i<LedMatrics; i++){
    lc->shutdown(i,false);
    lc->setIntensity(i,8);
    lc->clearDisplay(i);
    }
  
    // PIR
    pinMode(pirPin, INPUT);
    pinMode(pirLEDPin, OUTPUT);
    digitalWrite(pirLEDPin, HIGH);
    delay(500);
    digitalWrite(pirLEDPin, LOW);
    
    for (int i=0; i<strlen(buf); i++) {
        buf[i] = ' ';
    }
    
    //SRF-05
    // pinMode(redLEDPin, OUTPUT);
    // pinMode(initPin,OUTPUT);
    // pinMode(echoPin, INPUT);
    
    //   for (int thisReading = 0; thisReading < numOfReadings; thisReading++) {
    // readings[thisReading] = 0;
//   }
    
  
  // set up the LCD's number of columns and rows: 
 // lcd->begin(20, 4);
//  lcd->clear();
  // Print a message to the LCD.
  lcd->setCursor(3,0);
  lcd->print("Weather monitor");
  lcd->setCursor(6,1);
  lcd->print("by cnwang");
  lcd->setCursor(0,2);
  lcd->print("Version :");
  lcd->print(Version);
         
    // initial DHT 
    
    // Serial.begin(115200);
    // println("Start DHT22 monitor");
    // print("Version : ");
    // Serial.println (Version);
    // //client.flush();
    
    scrollMessage("Weather Monitor. by cnwang ");
    //scrollMessage("012345678901234567890");
    Time.zone(+8);
    Spark.syncTime();
    lastSync = millis();
    
    TimeString();

    dht.begin();
    pinMode(D7, OUTPUT);
    //pinMode(A7, INPUT);
    ledStatus(D7, 3,1000);
    ledStatus(D7, 2,100);
    lastUp=millis()-3*1000*60;
    lastMoveUp = millis();
    lcdBacklightLastUp=millis();
    loopIdx=0;
    
    ledStatus(D5, 1,100);   
    
   
}

int phase = 0;
//char message[64];
int bufPos=0;
int bufLen=0;
char buf2[]="                                                      ";
char timebuf[60];
char Accbuf[40];



void loop() {

    //if ((millis()-lastUp>(tInterval()*1000*60)) || (millis()<lastUp)){
       // h = dht.readHumidity();
    //    t = dht.readTemperature();
        
        
    if (millis() - lastSync > ONE_DAY_MILLIS) {
    // Request time synchronization from the Spark Cloud
        Spark.syncTime();
        lastSync = millis();
    }
    if ((millis()-lastUp>(CHECK_DHT_PERIOD*1000)) || (millis()<lastUp)){  
          
        ledStatus(led, 1,300);
        //lcd->backlight();
        lcdBacklightOn();
        
        lastUp=millis();
        
        f = 0;
        h = dht.readHumidity();
        t = dht.readTemperature();
        //delay(2000); //delete 2014 10 04
        //if (distanceAcc>0){
        if (h<100) {
            sendToXivelyWithLed(led, t,h,pirMove,0);
        }
            //} //else {
            // if (h<100) {
            //     sendToXivelyWithLed(led, t,h,pirMove,-1);
            // }
        //}
        pirMove=0;
            // distanceAcc=0;
            // distanceCount=0;
        //sendToXively2(t,h); 
        //======
       
        delay(2000);
        lcd->setCursor(0,1);
        lcd->print("Temp = 00.00 C   ");
        lcd->setCursor(7,1);
        lcd->print(t,2);
        lcd->setCursor(0,2);
        lcd->print("RH.% = 99.00 %   ");
        lcd->setCursor(7,2);
        lcd->print(h,2);
        lcd->setCursor(0,3);
        lcd->print( "U/L: ");
        lcd->print(++dataSent);
        
    //    TimeString();
        // check hour 
    //    setAllLedIntensity(ledIntensity);
        
        //sprintf(timebuf, "%s", timeStr.toCharArray);
         //sprintf(buf, "T=%hi.%01hi%cC, H=%i.%01i%%, %s ", int(t), abs(int(int(t*10)%10)), 0x7f, int(h), int(int(h*10)%10),timebuf);
     //    sprintf(buf, "%hi.%01hi%cC  %i.%01i%%  %s  ", int(t), abs(int(int(t*10)%10)), 0x7f, int(h), int(int(h*10)%10),timebuf);
     //    bufPos=0;
     //    bufLen = strlen(buf);

        lcdBacklightOff();
    } else { //not 2 min
        
        lcd->setCursor(19,3);
        lcd->print(loopStr[loopIdx++]);
        if (loopIdx>13) loopIdx=0;
        TimeString();
        lcd->setCursor(8,3);
        //lcd->print(timebuf);
        

    }
    
    TimeString();
    sprintf(buf, "%hi.%01hi%cC  %i.%01i%%  %s  ", int(t), abs(int(int(t*10)%10)), 0x7f, int(h), int(int(h*10)%10),timebuf);
    //bufPos=0;
    bufLen = strlen(buf);
    lcd->setCursor(0,3);
    lcd->print (buf);
    lcd->setCursor(0,2);
    lcd->print (bufLen);
  
    // cancel split message check
    for (int i=0; i<10; i++) {
        *(buf2+i)=0;
    }
    
    
    for (int i=0; i<8 && bufPos<bufLen; i++, bufPos++) {
        *(buf2+i) = *(buf+bufPos);
        
    }
    *(buf2+bufPos)=0;
    if (bufPos>=bufLen) bufPos=0;
    
    // //sprintf(buf2, "1234567890ABC");
    scrollMessage(buf2);
    // scrollMessage(buf);
    
    
    if ((millis()-lastMoveUp>CHECK_PIR_PERIOD*1000) || (millis()<lastMoveUp)) {
        int k = digitalRead(pirPin);
        lastMoveUp = millis();
     
        if (k==0 ) { // no move
            if (millis()-lastMove>CHECK_PIR_PERIOD*1000) {
                digitalWrite(pirLEDPin, LOW); // turn off the light
                lcdBacklightOff();
            }
        } else {
            digitalWrite(pirLEDPin, HIGH);   // trun on the light
            lastMove=millis();
            lcdBacklightOn();
            pirMove+=1;
        }
        sprintf(Accbuf,"K=%d A=%d",k,pirMove);
        lcd->setCursor(8,3);
        lcd->print (Accbuf);
     
        
        
    } // check PIR every 5 seconds

}

void lcdBacklightOn() {
    lcd->backlight();
    lcdBacklightLastUp=millis();
}
void lcdBacklightOff(){
    if (millis()-lcdBacklightLastUp > lcdBacklightDelay*1000 || millis()<lcdBacklightLastUp) {
        lcd->noBacklight();
    }
}




const int numDevices = 1;
const long scrollDelay = 16;
unsigned long bufferLong [14] = {0}; 
unsigned long bufferLong2[14]={0};

//const static byte cp437_font [256] [8] = {


void scrollFont() {
  for (int counter=0x20;counter<0x80;counter++){
    loadBufferLong(counter);
    delay(500);
  }
}


// Scroll Message
void scrollMessage(char *messageString) {
  int counter = 0;
  int myChar=0;
  do {
    // read back a char 
    myChar =  *(messageString + counter); 
    if (myChar != 0){
      loadBufferLong(myChar);
    }
    counter++;
  } 
  while (myChar != 0);
}
// Load character into scroll buffer



void loadBufferLong(int ascii){
  if (ascii >= 0x20 && ascii <=0x7f){
    for (int a=0;a<7;a++){                      // Loop 7 times for a 5x7 font
      //unsigned long c = pgm_read_byte_near(font5x7 + ((ascii - 0x20) * 8) + a);     // Index into character table to get row data
      unsigned long c = *(font5x7 + ((ascii -32) * 8) + (6-a));     // Index into character table to get row data
      unsigned long x = bufferLong [a*2];     // Load current scroll buffer
      x = x | c;                              // OR the new character onto end of current
      bufferLong [a*2] = x;                   // Store in buffer
    }
    byte count = *(font5x7 +((ascii -32) * 8) + 7);     // Index into character table for kerning data
    for (byte x=0; x<count;x++){
      rotateBufferLong();
      printBufferLong();
      delay(scrollDelay);
    }
  }
}
// Rotate the buffer
void rotateBufferLong(){
  for (int a=0;a<7;a++){                      // Loop 7 times for a 5x7 font
    unsigned long x = bufferLong [a*2];     // Get low buffer entry
    byte b = bitRead(x,31);                 // Copy high order bit that gets lost in rotation
    x = x<<1;                               // Rotate left one bit
    
    unsigned long x2 = bufferLong2[a*2];        // [a2+1] [a2]
    byte b2 = bitRead(x2,31);
    x2 = x2<<1;
    x2 = x2 | b;
    bufferLong2[a*2]=x2;
    x2 = bufferLong2[a*2+1];
    x2 = x2<<1;
    x2 = x2 | b2;
    bufferLong2[a*2+1]=x2;
    
    
    bufferLong [a*2] = x;                   // Store new low buffer
    x = bufferLong [a*2+1];                 // Get high buffer entry
    x = x<<1;                               // Rotate left one bit
    bitWrite(x,0,b);                        // Store saved bit
    bufferLong [a*2+1] = x;                 // Store new high buffer
  }
} 

void rotateBufferLongOld(){
  for (int a=0;a<7;a++){                      // Loop 7 times for a 5x7 font
    unsigned long x = bufferLong [a*2];     // Get low buffer entry
    byte b = bitRead(x,31);                 // Copy high order bit that gets lost in rotation
    x = x<<1;                               // Rotate left one bit
    bufferLong [a*2] = x;                   // Store new low buffer
    x = bufferLong [a*2+1];                 // Get high buffer entry
    x = x<<1;                               // Rotate left one bit
    bitWrite(x,0,b);                        // Store saved bit
    bufferLong [a*2+1] = x;                 // Store new high buffer
  }
} 
void printBufferLong(){
  for (int a=0;a<7;a++){                    // Loop 7 times for a 5x7 font
    unsigned long x = bufferLong [a*2+1];   // Get high buffer entry
    
    //unsigned long x = bufferLong[a*2+1];
    byte y = (x >>16);
    
    lc ->setRow(5,a,y);
    
    y = byte(x>>8);
    lc ->setRow(4,a,y);
    
    
    y = x;                             // Mask off first character
    //lc2.setRow(3,a,y);                       // Send row to relevent MAX7219 chip
    lc->setRow(3,a,y);                       // Send row to relevent MAX7219 chip
    x = bufferLong [a*2];                   // Get low buffer entry
    y = (x>>24);                            // Mask off second character
    lc->setRow(2,a,y);                       // Send row to relevent MAX7219 chip
    y = (x>>16);                            // Mask off third character
    lc->setRow(1,a,y);                       // Send row to relevent MAX7219 chip
    y = (x>>8);                             // Mask off forth character
    lc->setRow(0,a,y);                       // Send row to relevent MAX7219 chip
  }
  
}
// Display Buffer on LED matrix
void printBufferLongBackup(){
  for (int a=0;a<7;a++){                    // Loop 7 times for a 5x7 font
    unsigned long x = bufferLong [a*2+1];   // Get high buffer entry
    
    unsigned long x2 = bufferLong2[a*2];
    byte y2 = (x2 >>16);
    
    lc ->setRow(5,a,y2);
    
    x2 = bufferLong2[a*2];
    //y2 = (x2 >> 16);
    y2 = byte(x2>>8);
    lc ->setRow(4,a,y2);
    
    
    byte y = x;                             // Mask off first character
    //lc2.setRow(3,a,y);                       // Send row to relevent MAX7219 chip
    lc->setRow(3,a,y);                       // Send row to relevent MAX7219 chip
    x = bufferLong [a*2];                   // Get low buffer entry
    y = (x>>24);                            // Mask off second character
    lc->setRow(2,a,y);                       // Send row to relevent MAX7219 chip
    y = (x>>16);                            // Mask off third character
    lc->setRow(1,a,y);                       // Send row to relevent MAX7219 chip
    y = (x>>8);                             // Mask off forth character
    lc->setRow(0,a,y);                       // Send row to relevent MAX7219 chip
  }
  
}
void printBufferLongOld(){
  for (int a=0;a<7;a++){                    // Loop 7 times for a 5x7 font
    unsigned long x = bufferLong [a*2+1];   // Get high buffer entry
    byte y = x;                             // Mask off first character
    //lc2.setRow(3,a,y);                       // Send row to relevent MAX7219 chip
    lc->setRow(3,a,y);                       // Send row to relevent MAX7219 chip
    x = bufferLong [a*2];                   // Get low buffer entry
    y = (x>>24);                            // Mask off second character
    lc->setRow(2,a,y);                       // Send row to relevent MAX7219 chip
    y = (x>>16);                            // Mask off third character
    lc->setRow(1,a,y);                       // Send row to relevent MAX7219 chip
    y = (x>>8);                             // Mask off forth character
    lc->setRow(0,a,y);                       // Send row to relevent MAX7219 chip
  }
}


void TimeString(){
    timeStr =" ";
     //Serial.print(str);
     //Serial.print(Time.timeStr());
    String newTimeStr = String(Time.timeStr());
    timeStr = newTimeStr;
     
    timeStr.toCharArray(timebuf,timeStr.length());
    
    //set intensity
    byte currentHour =  Time.hour();
    //rtc.hour(currentTime);
    if ((currentHour >=5) && (currentHour<18)) {
        ledIntensity = 15;
    } else if ((currentHour >=18) && (currentHour<24)){
        ledIntensity = 1;
    } else {
        ledIntensity = 0;
    }
  
    for (int a=0; a<6; a++) {
        
        lc->setIntensity(a, ledIntensity);
    }
    
    if (ledIntensity == 0) {
        lcdBacklightOff();
    }
}


void statusPrint(String statusStr) {
    lcd->setCursor(8,3);
    lcd->print ("                       ");
    lcd->setCursor(8,3);
    lcd->print (statusStr);

}