#ifndef NODEMCU_MIXED_DEFINES_H
#define NODEMCU_MIXED_DEFINES_H

#define TEST_LED        (D0)

#define DHTPIN          (D3)
#define DHTTYPE         (DHT22)

#define SERIAL_INTERVAL (60000u) /* 60 seconds */
#define NTP_INTERVAL    (1000u) /* 1000 seconds */

#define DEBUG 1

#if (DEBUG==1)
#define pr(...) Serial.print(__VA_ARGS__)
#else
#define pr(...) do {} while(0)
#endif

typedef struct
{
    uint32_t SHCP;  /* Shift clock */
    uint32_t STCP;  /* Shift storage clock */
    uint32_t DS;    /* Serial data input */
} Led74Hc_Type;

typedef struct
{
    uint8_t b0:1;
    uint8_t b1:1;
    uint8_t b2:1;
    uint8_t b3:1;
    uint8_t b4:1;
    uint8_t b5:1;
    uint8_t b6:1;
    uint8_t b7:1;
} DataBit_Type;

typedef struct
{
    uint8_t s;
    uint8_t m;
    uint8_t h;
} TimeStruct_Type;

extern TimeStruct_Type currentTime;


#endif /* NODEMCU_MIXED_DEFINES_H */