// Store data in flash (program) memory instead of SRAM
#include <pgmspace.h>
#define SECRET
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "" // Here goes your information
#define BLYNK_TEMPLATE_NAME "" // Here goes your information
#define BLYNK_AUTH_TOKEN "" // Here goes your information
const char WIFI_SSID[] = "  "; // Here goes your information
const char WIFI_PASSWORD[] = "  "; // Here goes your information
#define VPIN_Text           V0
#define VPIN_Mode           V1
#define VPIN_currentTemp    V2
#define VPIN_currentHumi    V3
#define VPIN_setTemp        V4
#define VPIN_setHumi        V5
#define VPIN_Light         V6
#define VPIN_Fan            V7
#define VPIN_currentLUX    V8
#define VPIN_currentPress    V9
#define VPIN_setLUX    V10
#define VPIN_Alerts   V11
#define VPIN_Temp   V12
#define VPIN_LUX   V13