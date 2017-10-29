    // compiled with core 0.5.3
// blynk 0.4.7

// This #include statement was automatically added by the Particle IDE.
#include <blynk.h>

//// This #include statement was automatically added by the Particle IDE.
#include "blynk/blynk.h"


// This #include statement was automatically added by the Particle IDE.
//#include "thingspeak/thingspeak.h"

// This #include statement was automatically added by the Spark IDE.
//#include "XivelyDatastream.h"

// This #include statement was automatically added by the Spark IDE.
//#include "XivelyClient.h"




#define Version "0.999a97a57"


// This #include statement was automatically 1added by the Spark IDE.
//#include "SparkTime/SparkTime.h"

// This #include statement was automatically added by the Spark IDE.
//#include "pulse.h"

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

// Byklin Auth key
char auth[] ="5b752cbeca284df19a1c0d8deb1f1b88";

// Twitter API Key
//#define TOKEN "2927024191-XLHyWduCLFMP0ouEvhxzBfZ86ATqSFBK9Rm3cDK"
// Twitter Proxy
//#define LIB_DOMAIN "arduino-tweet.appspot.com"

// ThinkSpeak parts.
//byte TSserver[]  = { 184, 106, 153, 149 }; // ThingSpeak IP Address: 184.106.153.149
//#define THINKSPEAKTOKEN "OS7CAQM44RGB0AI2"

//cnwang.bot
// #define THINKSPEAKTWITTERTOKEN "TGOF09S158REUUZC"
//cnwang406
//#define THINKSPEAKTWITTERTOKEN "LCQ2C784KIUGR5QG"
//#define THINKSPEAKSERVER "api.thingspeak.com"
//TCPClient TSclient;
//TCPServer server = TCPServer(8081);
//ThingSpeakLibrary::ThingSpeak thingspeak ("OS7CAQM44RGB0AI2");


#define CHECK_DHT_PERIOD    1*60   //check DHT every 10 min
//#define CHECK_PIR_PERIOD    5       //check PIR every 5 sec.
//#define CHECK_PIR_SILENTCYCLE   36  // 5 * 36 = 180 = 3 min silent.
#define LCD_BACKLIGHT_PERIOD    10  //lcd backlight last 10 sec.

#define pirLEDPin D6
#define pirPin D3

#define pirEnable FALSE

#define breathLED D5

#define WIFIRETRY 3         // will reboot WIFI after failed 
int wifiRetries = 0;

// SRF-05
// const int numOfReadings = 3;                   // number of readings to take/ items in the array
// int readings[numOfReadings];                    // stores the distance readings in an array
// int arrayIndex = 0;                             // arrayIndex of the current item in the array
// int total = 0;                                  // stores the cumlative total
// int averageDistance = 0;                        // stores the average value
// unsigned long pulseTime = 0;                    // stores the pulse in Micro Seconds
// unsigned long distance = 0;                     // variable for storing the distance (cm)
// #define SRFThreshold 38
// #define SRFMAX  50


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


// TCPServer server = TCPServer(8081);

//--------------------------------------
//TCPClient client;


//-----------
// void sendToThingSpeakWithLed(int led, float temperature, float humidity, int pirMove,int distance){
//     //int r = sendToXively(temperature, humidity, pirMove,distance);
//     int r = writeToThingSpeak(temperature, humidity, pirMove, distance);
//     if (r==0){
//         ledStatus(led, 1, 100);    
//     } else {
//         ledStatus (led, 2,1000);
//     }
// }



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

float oh;   // outdoor humidity
float ot;   // outdoor temperature
float obatm; // bmp's outdoor atmosphere
float obt;  // bmp's temperature
float obalt;

unsigned long lastUp;
//unsigned long lastMove;
//unsigned long lastMoveUp;

// int pirMove=0;
// int oldPir=0;
// int pirAlert = CHECK_PIR_SILENTCYCLE;   // 5 pir sensor cycle no move and moved again will alert
char loopStr[]="|\\-/!@#$%^&*";
int loopIdx = 0;
int dataSent=0;
char buf[65];
bool BTReady;

unsigned long lcdBacklightDelay=15;   // trun off after 15 sec.
unsigned long lcdBacklightLastUp=millis();

//sparkTime rtc;
UDP UDPClient;
//SparkTime rtc;

unsigned long currentTime;
unsigned long startTime=millis()/1000;
unsigned long upTime=0L;;
unsigned long lastTime = 0UL;
unsigned long blynkTimer = 15L; // 60 seconcds
unsigned long lastBlynkTimer; // 60 seconcds
unsigned long lastBlynkTimer2; // 60 seconcds

String timeStr;
String newTimeStr;
String emptyStr = "                   ";

#define ONE_DAY_MILLIS (8 * 60 * 60 * 1000)
unsigned long lastSync = millis();
char timebuf[60];

#define MINIUMTWITTERPERIOD ( 60 *60 * 1000)         // twitter at least 1 hour a time
unsigned long lastTwitter = millis();


//BLYNK 
WidgetTerminal terminal(V18);
#define BLYNK_roomTemp  V1
#define BLYNK_roomHumidity V2
#define BLYNK_outdoorTemp V3
#define BLYNK_outdoorHumidity V6
#define BLYNK_IMHERE V10
#define BLYNK_cloud V11
#define BLYNK_terminal V18
#define BLYNK_ATM V8
#define BLYNK_BMPTEMP V7
#define BLYNK_ALT V5
#define BLYNK_FREQ V9



void setup() {
     ledStatus(D7, 5,1000); // I'm on 
 
    lcd = new LiquidCrystal_I2C(0x27, 20,4);
  
    lcd->init();                      // initialize the lcd
    lcd->clear();
    lcd->backlight();
    
    
  
 // InitLED 
    lc = new LedControl(A0,A2,A1,LedMatrics);
    for (int i=0; i<LedMatrics; i++){
        lc->shutdown(i,false);
        lc->setIntensity(i,8);
        lc->clearDisplay(i);
    }
  
    // PIR
    // pinMode(pirPin, INPUT);
    // pinMode(pirLEDPin, OUTPUT);
    // // digitalWrite(pirLEDPin, HIGH);
    // delay(500);
    // digitalWrite(pirLEDPin, LOW);
    
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
    // byklin initialize
  
    
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
    
     //terminal.flush();
      TimeString();
    Blynk.begin(auth);
    terminal.println(timebuf);
    terminal.println("Weather Monitor Start");
    terminal.print("Ver ");
    terminal.println(Version);
    terminal.println("----------------------");


    
    lastBlynkTimer=millis();
    lastBlynkTimer2=millis();
    Blynk.virtualWrite(BLYNK_FREQ, blynkTimer);
    Blynk.syncVirtual(BLYNK_FREQ);
    terminal.print("BLYNK freq = ");
    terminal.print(blynkTimer);
    terminal.println(" seconds");
    terminal.println(blynkTimer);
    terminal.flush();
    

    lcd->setCursor(0,3);
    lcd->print(timebuf);

//Init Port
    dht.begin();
    pinMode(D7, OUTPUT);
    ledStatus(D7, 3,1000);
    ledStatus(D7, 2,100);
    lastUp=millis()-3*1000*60;
    // lastMoveUp = millis();
    lcdBacklightLastUp=millis();
    loopIdx=0;
    
    ledStatus(D5, 1,100);   
    

    
    
// 20170
//initial BT
    
    Serial1.begin(9600);
    Serial1.flush();
    Serial1.flush();
    
 

}

int phase = 0;
//char message[64];
int bufPos=0;
int bufLen=0;
char buf2[]="                                                      ";
char Accbuf[40];

int x=0 ;
char btBuf[128];

int counterTotal = 0;
int counterOK = 0;
bool dataWrote = TRUE;
bool dataWrite2 = TRUE;

unsigned long btTimer=millis();
int displayTemp = 0;
int lastDisplayTemp=0;
void loop() {
    //char myEndTXmarker = '#' ;//for example
    char readByte;
 
    Blynk.run();
    TimeString();
    if ((millis()-btTimer>30*1000L) || (millis()<btTimer)){
        ledStatus(D7, 3,50);
        //terminal.print("read BT...");
        if (Serial1.available()) {
            //BT:[
            //   16.7 83.1 17.1 1033.8 0055.0]#
            //  123456789012345678901234567890 
            // terminal.println("data in");
            // terminal.flush();
            readByte = Serial1.read();
            if (readByte == '#') {
                delay(20);
                float tempot = Serial1.parseFloat();
                //tempot = (int(tempot*10) % 1000)/10;
                while (tempot>100) {
                    tempot-=100;
                }
                delay(20);
                float tempoh = Serial1.parseFloat();
                delay(20);
                float tempobatm=Serial1.parseFloat();
                delay(20);
                float tempobt = Serial1.parseFloat();
                delay(20);
                float tempobalt=Serial1.parseFloat();
                delay(20);
                int checkSum = Serial1.parseInt();
                delay(20);
                Serial1.flush();    
                int checkSum2 = int(tempoh)+int(tempobatm)+int(tempobt)+int(tempobalt)+int(tempot); 
                counterTotal++;
                
                terminal.print (timebuf);
                terminal.print (" ");
                terminal.print("checksum= " + String(tempot)+ "+"+ String(tempoh)+ "+"+String(tempobatm)+"+"+
                    String(tempobalt)+"+"+String(tempobt)+"="+String(checkSum) );
                    
                if (checkSum2== checkSum){
                
                    ot = (tempot>100.0) ? ot:tempot;
                    oh = (tempoh>100.0)?oh : tempoh;
                    obt = (tempobt>100 || tempobt<=0)? ot : tempobt;
                    //obt = tempobt;
                    obatm = tempobatm;
                    obalt = tempobalt;
                    terminal.print(" == ");
                    dataWrote=FALSE;
                    counterOK++;
                    
                } else { // do something ....
                    delay(1000);    // delay 1 second
                    Serial1.flush();    // just remove all
                    terminal.print (" != ");
                    
                    
                }
                terminal.print(checkSum2);
                // terminal.print(counterOK);
                
                // terminal.print(counterTotal);
                double perc = counterOK*100.0/counterTotal;
                terminal.printf(" %d out of %d OK (%2i.%1i%%) \n",counterOK, counterTotal,
                    int (perc),int(int(perc*10)%10));    
                
                if (tempot>100.0 || tempoh >100) { terminal.println(" outdoor DHT22 FAIL!");}
                if (tempobt>100.0 || tempobatm>1045) { terminal.println( " outdoor BMP FAIL!");}
                
                terminal.flush();
                
                ledStatus(D7, 1,20);
            
            }

        }else {
            ledStatus(D7, 2,100);
            // terminal.print(timebuf);
            // terminal.println("  NO data in");
            terminal.flush();
        }
        
        //terminal.println("BT:[ot="+String(ot)+" oh="+String(oh)+ " obt="+String(obt)+" obatm="+String(obatm)+" obalt="+String(obalt)+"]");
       // if (blynkTimer!=0) {
           if ((millis()-lastBlynkTimer > blynkTimer*1000L) || (millis() < lastBlynkTimer)) {
            //   terminal.println(timebuf);
            //   terminal.printf("write! %u -  %u = %u , int=%u \n",millis(), lastBlynkTimer, millis()-lastBlynkTimer, blynkTimer*1000L);
                lastBlynkTimer=millis();
                
                myTimerEvent();
                
            } else {
                // terminal.println(timebuf);
                // terminal.printf("No write ! %u -  %u = %u , int=%u \n",millis(), lastBlynkTimer, millis()-lastBlynkTimer, blynkTimer*1000L);
            }
            terminal.flush();
       // }
        
        btTimer=millis();  //every 60 seconds
        
    }
   

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

    // internal DHT22
    if ((millis()-lastUp>(CHECK_DHT_PERIOD*1000)) || (millis()<lastUp)){  
        lcdBacklightOn();
        lastUp=millis();
        ledStatus(led, 1,300);

        if (!WiFi.ready()) {
            WiFi.connect();
        }
// reading DHT
        statusPrint ("Reading DHT");
        f = 0;
        t = dht.readTemperature();
        h = dht.readHumidity();
        
        statusPrint ("Read OK");
        if ((h<100)&& (t<100)) {
        // if (blynkTimer !=0) {
            
                // if ((millis()-lastBlynkTimer2>blynkTimer*1000L) || (millis()<lastBlynkTimer2)){
                //     lastBlynkTimer2 = millis();
                    // terminal.print(timebuf);
                    // terminal.printf( " Write roomTemp\n");
                    Blynk.virtualWrite(BLYNK_roomTemp, t);
                    Blynk.virtualWrite(BLYNK_roomHumidity, h); 
                // }
           // }
        } else {
                terminal.println(" indoor DHT FAIL...\n   ** REBOOT **\n");
                terminal.flush();
//                System.reset();
        }
 
 
        
        lcd->clear();
        lcd->setCursor(0,0);
        lcd->print("  Weather monitor  ");

        //lcd->setCursor(0,1);
        //lcd->print(emptyStr);
        lcd->setCursor(0,1);
        lcd->print("T  = ");
        //lcd->setCursor(5,1);
        lcd->print(t,2);
        lcd->print(" C");
        lcd->setCursor(0,2);
        lcd->print("RH%= ");
        lcd->print(h,2);
        lcd->print(" %");
        
    }
    
    
    
//Process LED matrix
    lcd->setCursor(19,3);
    lcd->print(loopStr[loopIdx++]);
    if (loopIdx>13) loopIdx=0;
   
 
    int displayTime = int(Time.second() /10);
    switch (displayTime) {
        case 0:
        case 1:
        case 2:
            
        //..99.9oc / 99.1oc  hh:mm.."
            sprintf(buf, "  %2i.%1i%cC / %2i.%0i%cC  %s  ", 
            int(t), abs(int(int(t*10)%10)), 0x7f, 
            int(ot), abs(int(int(ot*10)%10)), 0x7f, 
            timebuf);
            bufLen = strlen(buf);
            if (lastDisplayTemp !=0) {
                bufPos=0;
                lastDisplayTemp=0;
            }
            break;
        case 3:
        case 4:
        
        //..99.9oc / 99.1oc  hh:mm.."
        //..99.9% / 99.9%    hh:mm.."
            sprintf(buf, "  %2i.%1i%% / %2i.%1i%%    %s  ", 
            int(h), int(int(h*10)%10),
            int(oh), int(int(oh*10)%10),
            
            timebuf);
            bufLen = strlen(buf);
            sprintf(buf2,"");
            if (lastDisplayTemp !=1) {
                bufPos=0;
                lastDisplayTemp=1;
            }


            break;
        case 5 :
        
        //12345678901234567890123456
        //..99.9oc / 99.1oc  hh:mm.."
        //..99.9% / 99.9%    hh:mm.."
        //..9999.9 mb........hh:mm..
            sprintf(buf, "  %4i.%1i mb   . .  %s  ", 
            int(obatm), int(int(obatm*10)%10),
            timebuf);
            bufLen = strlen(buf);
            sprintf(buf2,"");
            if (lastDisplayTemp !=2) {
                bufPos=0;
                lastDisplayTemp=2;
            }
            break;
        
    }
    
    

    // for (int i=0; i<10; i++) {
    //     *(buf2+i)=0;
    // }
    
    for (int i=0; i<8 && bufPos<bufLen; i++, bufPos++) {
        *(buf2+i) = *(buf+bufPos);
        
    }
    *(buf2+bufPos)=0;
    if (bufPos>=bufLen) {
        bufPos=0;
        
    }
    scrollMessage(buf2);
 
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

String left(int sss, int leng) {
    String temp = String("0000") + String(sss);
    String temp2 = temp.substring(temp.length()-leng);
    return temp2;
    //return temp2;
    
}

void TimeString(){
    sprintf(timebuf, "%02d/%02d %02d:%02d",Time.month(),Time.day(),Time.hour(),Time.minute());
    
    // timeStr =" ";
    // String newTimeStr = left(Time.month(),2) + "/" + left(Time.day(),2) + " " + left(Time.hour(),2)+ ":"+ left(Time.minute(),2)+" ";
    
    // timeStr = newTimeStr;
    
     
    // timeStr.toCharArray(timebuf,timeStr.length());
    
    //set intensity
    byte currentHour =  Time.hour();
    //rtc.hour(currentTime);
    if ((currentHour >=5) && (currentHour<18)) {
        ledIntensity = 15;
    } else if ((currentHour >=18) && (currentHour<24)){
        ledIntensity = 1;
    } //else {
    //     ledIntensity = 0;
    //     lcdBacklightOff();
    // }
  
    for (int a=0; a<6; a++) {
        
        lc->setIntensity(a, ledIntensity);
    }
    
}

char * getUpTime(){
    static char str[14];
    if (upTime<millis()/1000) {
        upTime = millis()/1000+ (4294967295-startTime);
    } else {
        upTime = millis()/1000-startTime;
    }
    long d = upTime/86400;  //86400 seconds a day
    int hr = upTime % 86400;
    int h = hr / 3600;
    int tr = hr % 3600;
    int m = tr /60;
    int s = m % 60;
    sprintf(str,"%dD %02dh %02dm",d,h,m);
    return str;
    
}


void statusPrint(String statusStr) {
    lcd->setCursor(0,3);
    lcd->print(emptyStr);
    lcd->setCursor(0,3);
    lcd->print (statusStr);

}





BLYNK_WRITE(BLYNK_IMHERE ) {

    ledStatus(D7,3,10);
    terminal.println("it should blink 3 times and reboot");
    terminal.flush();
    //Blynk.virtualWrite(BLYNK_roomTemp, t);
    
    System.reset();
    
}

BLYNK_WRITE(BLYNK_cloud) {
    //Blykn.virtualRead(V2,)
    if (param.asInt() ==1) { // connect 
        Particle.connect();
        terminal.println("Particle Cloud connected");
        ledStatus(D7,1,1000);
    } else {
        Particle.disconnect();
        terminal.println("Particle Cloud disconnected");
        ledStatus(D7,2,500);;
    }
    terminal.println(getUpTime());
    
    terminal.flush();
}

BLYNK_WRITE(BLYNK_FREQ ) {

    ledStatus(D7,3,1000);
    if (param.asInt()==0) { // stop write to blynk
        blynkTimer = 0L;
        terminal.println("Suspend write to Blynk");
    } else {
        blynkTimer = param.asInt();
        terminal.print("Blynk write frequency change to ");
        terminal.print(blynkTimer);
        terminal.println(" seconds");
    }
    
    terminal.flush();
    
}


void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  
  
  //DHT22
    if ((oh!=0)) {
        Blynk.virtualWrite(BLYNK_outdoorHumidity, oh);
        Blynk.virtualWrite(BLYNK_outdoorTemp, ot);
        
    }
    
    //BMP
    if ((obatm!=0) && (obalt!=0)) {
        Blynk.virtualWrite(BLYNK_ATM, obatm);
        Blynk.virtualWrite(BLYNK_BMPTEMP, obt);
        Blynk.virtualWrite(BLYNK_ALT, obalt);
    }
}