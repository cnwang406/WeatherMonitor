// This #include statement was automatically added by the Particle IDE.
#include "thingspeak/thingspeak.h"

// This #include statement was automatically added by the Spark IDE.
//#include "XivelyDatastream.h"

// This #include statement was automatically added by the Spark IDE.
//#include "XivelyClient.h"


#define Version "0.997a89"


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



int led=7;
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))


// Twitter API Key
#define TOKEN "2927024191-XLHyWduCLFMP0ouEvhxzBfZ86ATqSFBK9Rm3cDK"
// Twitter Proxy
#define LIB_DOMAIN "arduino-tweet.appspot.com"

// ThinkSpeak parts.
byte TSserver[]  = { 184, 106, 153, 149 }; // ThingSpeak IP Address: 184.106.153.149
#define THINKSPEAKTOKEN "OS7CAQM44RGB0AI2"

//cnwang.bot
// #define THINKSPEAKTWITTERTOKEN "TGOF09S158REUUZC"
//cnwang406
#define THINKSPEAKTWITTERTOKEN "LCQ2C784KIUGR5QG"
#define THINKSPEAKSERVER "api.thingspeak.com"
TCPClient TSclient;
//TCPServer server = TCPServer(8081);
ThingSpeakLibrary::ThingSpeak thingspeak ("OS7CAQM44RGB0AI2");


#define CHECK_DHT_PERIOD    10*60   //check DHT every 10 min
#define CHECK_PIR_PERIOD    5       //check PIR every 5 sec.
#define LCD_BACKLIGHT_PERIOD    10  //lcd backlight last 10 sec.

#define pirLEDPin D6
#define pirPin D3

#define breathLED D5

#define WIFIRETRY 3         // will reboot WIFI after failed 
int wifiRetries = 0;

// SRF-05
const int numOfReadings = 3;                   // number of readings to take/ items in the array
int readings[numOfReadings];                    // stores the distance readings in an array
int arrayIndex = 0;                             // arrayIndex of the current item in the array
int total = 0;                                  // stores the cumlative total
int averageDistance = 0;                        // stores the average value
unsigned long pulseTime = 0;                    // stores the pulse in Micro Seconds
unsigned long distance = 0;                     // variable for storing the distance (cm)
#define SRFThreshold 38
#define SRFMAX  50


// setup pins/values for LED

#define redLEDPin  D6  //A4                              // Red LED, connected to digital PWM pin 9
int redLEDValue = 0;                            // stores the value of brightness for the LED (0 = fully off, 255 = fully on)

//setup

int ledIntensity = 15;                          



// void print(String str) {
//     // Serial.begin(9600);
//     // Serial.print(str);
//     // Serial.end();
// }
// void println(String str) {
//     // Serial.begin(9600);
//     // Serial.println(str);
//     // Serial.end();

// }


TCPServer server = TCPServer(8081);

//--------------------------------------
//TCPClient client;


//-----------
void sendToThingSpeakWithLed(int led, float temperature, float humidity, int pirMove,int distance){
    //int r = sendToXively(temperature, humidity, pirMove,distance);
    int r = writeToThingSpeak(temperature, humidity, pirMove, distance);
    if (r==0){
        ledStatus(led, 1, 100);    
    } else {
        ledStatus (led, 2,1000);
    }
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
int oldPir=0;
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
String emptyStr;
#define ONE_DAY_MILLIS (8 * 60 * 60 * 1000)
unsigned long lastSync = millis();
char timebuf[60];

#define MINIUMTWITTERPERIOD ( 60 *60 * 1000)         // twitter at least 1 hour a time
unsigned long lastTwitter = millis();

void setup() {
  
    
  
    lcd = new LiquidCrystal_I2C(0x27, 20,4);
  
    lcd->init();                      // initialize the lcd
    lcd->backlight();
    lcd->clear();
    
    
  
 // InitLED 
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
    
// Init LCD
    lcd->setCursor(3,0);
    lcd->print("Weather monitor");
    lcd->setCursor(6,1);
    lcd->print("by cnwang");
    lcd->setCursor(0,2);
    lcd->print("Version :");
    lcd->print(Version);
         
   
    scrollMessage("Weather Monitor. by cnwang ");


    delay(3000);
    lcd->clear();
    lcd->setCursor(0,0);
    lcd->print("SSID:");
    lcd->print(WiFi.SSID());
    lcd->setCursor(0,1);
    lcd->print("IP:");
    lcd->print(WiFi.localIP());
    lcd->setCursor(0,2);
    lcd->print("RSSI:");
    switch (WiFi.RSSI()){
        case 1:
            lcd->print("WIFI chip fail");
            break;
        case 2:
            lcd->print("timeout");
            break;
        default:
            lcd->print(WiFi.RSSI());
            lcd->print(" dB");
    }

//lcd->print(WiFi.RSSI());
    //SyncTime

    Time.zone(+8);
    Particle.syncTime();
    lastSync = millis();
    
    TimeString();
    lcd->setCursor(0,3);
    lcd->print(timebuf);

//Init Port
    dht.begin();
    pinMode(D7, OUTPUT);
    ledStatus(D7, 3,1000);
    ledStatus(D7, 2,100);
    lastUp=millis()-3*1000*60;
    lastMoveUp = millis();
    lcdBacklightLastUp=millis();
    loopIdx=0;
    
    ledStatus(D5, 1,100);   
    
    
//Set thingspeak    
     thingspeak.setConnectionTimeout(1500);
     
//writeToThingSpeak(50.5,50.5,3,0);
     
    
    emptyStr="";
    for (int i=0;i<19;i++){
        emptyStr +=" ";
    }
    statusPrint("TWITTER......");
    String twStart = "Weather Moniotr Start (" + String(Version) + ")";    

    updateTwitterStatus2(twStart);


}

int phase = 0;
//char message[64];
int bufPos=0;
int bufLen=0;
char buf2[]="                                                      ";
char Accbuf[40];



void loop() {

        
    if (millis() - lastSync > ONE_DAY_MILLIS) {
    // Request time synchronization from the Spark Cloud
        lcd->init();  
        lcd->setCursor(3,0);
        lcd->print("Weather monitor");

        Particle.syncTime();
        lastSync = millis();
        
         lcdBacklightOn();
        //twitterStr("11 WM Time Sync");
        
        statusPrint ("Time Sync");
        
    }
    if ((millis()-lastUp>(CHECK_DHT_PERIOD*1000)) || (millis()<lastUp)){  
        lcdBacklightOn();
        lastUp=millis();
        ledStatus(led, 1,300);
        //updateTwitterStatus2("lalala");

        switch (WiFi.RSSI()){
        case 1:
            statusPrint("WIFI chip fail");
            break;
        case 2:
            statusPrint("WIFI timeout");
            break;
        default:
            String wifiStr = String(WiFi.SSID()) + "/" + String(WiFi.RSSI()) + " dB";
            statusPrint(wifiStr);
        }

        if (wifiRetries == -1) {
            if (WiFi.ready()) {
                updateTwitterStatus2("wifi module reboot ok");
                statusPrint("wifi Rebooted ok");
                wifiRetries=0;
            } else {
                // wifi reboot, but has not readied yet
                statusPrint("Wifi not ready");
            }
        }

// reading DHT
        statusPrint ("Reading DHT");
        f = 0;
        t = dht.readTemperature();
        h = dht.readHumidity();
        
        statusPrint ("Read OK");
        String twmsg = "WM T="+ String(t).substring(0,4)+ " RH="+String(h).substring(0,4)+" P="+String(pirMove,DEC) + " " + timebuf;

        if (h<100) {    // only send valid data
            sendToThingSpeakWithLed(led, t,h,pirMove,0);
            //updateTwitterStatus2(twmsg);
        }
        
// twitter if PIR changed
        if (pirMove>0 ) {
            updateTwitterStatus2(twmsg );
        } else {    // no move
            if(((millis()-lastTwitter) >MINIUMTWITTERPERIOD) || ( millis()<lastTwitter)){ // or 1 hour not twittered
                updateTwitterStatus2(twmsg );
           
            }
        }
         pirMove=0;

        //delay(500);
        lcd->setCursor(0,0);
        lcd->print("  Weather monitor  ");

        lcd->setCursor(0,1);
        lcd->print(emptyStr);
        lcd->setCursor(0,1);
        lcd->print("T  = ");
        //lcd->setCursor(5,1);
        lcd->print(t,2);
        lcd->print(" C");
        
        lcd->setCursor(0,2);
        lcd->print(emptyStr);
        lcd->setCursor(0,2);
        lcd->print("RH%= ");
        //lcd->setCursor(5,2);
        lcd->print(h,2);
        lcd->print(" %");
        lcd->setCursor(15,2);
        //lcd->print( " ");
        lcd->print(++dataSent);
        

        
    }
    
    
    
//Process LED matrix
    lcd->setCursor(19,3);
    lcd->print(loopStr[loopIdx++]);
    if (loopIdx>13) loopIdx=0;
   
    TimeString();
    sprintf(buf, "%hi.%01hi%cC  %i.%01i%%  %s  ", int(t), abs(int(int(t*10)%10)), 0x7f, int(h), int(int(h*10)%10),timebuf);
    bufLen = strlen(buf);
    
    for (int i=0; i<10; i++) {
        *(buf2+i)=0;
    }
    
    for (int i=0; i<8 && bufPos<bufLen; i++, bufPos++) {
        *(buf2+i) = *(buf+bufPos);
        
    }
    *(buf2+bufPos)=0;
    if (bufPos>=bufLen) bufPos=0;
    scrollMessage(buf2);
    
    
// Process PIR    
    if ((millis()-lastMoveUp>CHECK_PIR_PERIOD*1000) || (millis()<lastMoveUp)) {
        int k = digitalRead(pirPin);
        lastMoveUp = millis();
     
        if (k==0 ) { // no move
            if (millis()-lastMove>LCD_BACKLIGHT_PERIOD*1000 || (millis()<lastMove)) {
                digitalWrite(pirLEDPin, LOW); // turn off the light
            //    lcdBacklightOff();
            }
        } else {
            digitalWrite(pirLEDPin, HIGH);   // trun on the light
            lastMove=millis();
            lcdBacklightOn();
            pirMove+=1;
            
        }
        
    } // check PIR every 5 seconds
    
    
    lcdBacklightOff();

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
const long scrollDelay = 8;
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

String left(int sss, int leng) {
    String temp = String("0000") + String(sss);
    String temp2 = temp.substring(temp.length()-leng);
    return temp2;
    //return temp2;
    
}

void TimeString(){
    timeStr =" ";
     //Serial.print(str);
     //Serial.print(Time.timeStr());
    //String newTimeStr = String(Time.timeStr());
    //String newTimeStr = String(Time.month()) + "/" + String(Time.day()) + " " + String(Time.hour())+ ":"+String(Time.minute());
    String newTimeStr = left(Time.month(),2) + "/" + left(Time.day(),2) + " " + left(Time.hour(),2)+ ":"+ left(Time.minute(),2)+" ";
    
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
    lcd->setCursor(0,3);
    lcd->print(emptyStr);
    lcd->setCursor(0,3);
    lcd->print (statusStr);

}



int writeToThingSpeak(float temperature, float humidity, int pirMove,int distance) {
    lcdBacklightOn();
    statusPrint ("send to TS......");
    bool valSet2 = thingspeak.recordValue(2, String(temperature, 1));
    bool valSet1 = thingspeak.recordValue(1, String(humidity, 1));
    bool valSet3 = thingspeak.recordValue(3, String(pirMove, DEC));
    
    bool valSent = thingspeak.sendValues();
    
    if ( valSent) {
        statusPrint("Sent to TS done");
        wifiRetries = 0;
        return 0;
    }else {
        wifiRetries ++;
        statusPrint("Sent to TS Fail (" + String(wifiRetries,DEC) + ")");
        if ( wifiRetries > WIFIRETRY) { 
            WiFi.off();
            delay(1000);
            WiFi.on();
            
            wifiRetries=-1;     // means rebooted
            return -1; // come another day
           
        }
            
        return 1;
    }
    lcdBacklightOff();

}


void updateTwitterStatus2(String tweetData){
    int result;
    //return 0;
    
    TCPClient twitterClient;
    //tweetData="api_key=TGOF09S158REUUZC&status=" + tweetData;
    //String tw2 = "api_key=TGOF09S158REUUZC&status=WeGoII1";
    String tw2 = "api_key=TGOF09S158REUUZC&status=" + tweetData;
    
    // Connecting and sending Tweet data to Thingspeak
    if(twitterClient.connect("api.thingspeak.com", 80))
    {
        twitterClient.print("POST /apps/thingtweet/1/statuses/update HTTP/1.1\n");
        twitterClient.print("Host: api.thingspeak.com\n");
        twitterClient.print("Connection: close\n");
        twitterClient.print("Content-Type: application/x-www-form-urlencoded\n");
        //twitterClient.print("Content-Length: " + String(tweetData.length(),DEC) + "\n\n");
        //twitterClient.println(tweetData); //the ""ln" is important here.
        
        twitterClient.print("Content-Length: "+ String(tw2.length(),DEC) + "\n\n");
        
        twitterClient.println(tw2);
        twitterClient.print("\n");
        // This delay is pivitol without it the TCP client will often close before the data is fully sent
       //statusPrint (" TW = "  + String(tw2.length(),DEC));
        delay(200);
        
        lastTwitter = millis();
        result= 0;
    }
    else{
        result= 1;
        
    }
        
    // if(!twitterClient.connected()){
    //     twitterClient.stop();
    // }
    twitterClient.flush();
    twitterClient.stop();
    
       
}



