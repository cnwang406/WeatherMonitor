#define ARDUINO_H
//#include <stdint.h>
#include "application.h"
#define DHTPIN D6    // Digital pin D2
#define DHTTYPE DHT22


//int led=7;
//#define led 7
       

//================

// DHT, modified, for those data difference > outliner and repeat not more than validPoints, 
// will return old value.
#define MAXTIMINGS 85

#define cli noInterrupts
#define sei interrupts

#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21

#define NAN 999999

class DHT {
        public:
        DHT(uint8_t pin, uint8_t type, uint8_t count=6);
        void begin(void);
        float readTemperature(bool S=false);
        float convertCtoF(float);
        float readHumidity(void);

    private:
        uint8_t data[6];
        uint8_t _pin, _type, _count;
        bool read(void);
        unsigned long _lastreadtime;
        bool firstreading;
        float lastTemperature;
        float lastHumidity;
        int outlinerTemperature = 15; 
        int outlinerHumidity=20;
        int makeValidPoints=10;
        int realValidPointsT;
        int realValidPointsH;
    

};

