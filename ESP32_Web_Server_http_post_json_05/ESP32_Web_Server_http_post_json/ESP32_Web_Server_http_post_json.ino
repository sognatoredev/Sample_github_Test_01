#include <iostream>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>
#include <SPI.h>
#include "data_type.h"

#ifndef DEBUG
#define DEBUG
#endif

#define LED_BLUE  2

#define CONNECT_TIMEOUT 5 // sec

typedef uint32_t ret_code_t;

/* create a hardware timer */
hw_timer_t * timer = NULL;
volatile uint32_t timer1_count, timer1_count2 = 0;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

uint8_t MakePacketData = 0x00;
String httpRequestData;

String uart_buf;

const char* ssid = "scs_tms";// "sw_epc";
const char* password = "scstms0903";
const char* serverName = "http://192.168.219.241:9983/api-docs/"; // 웹서버주소
int value;
int sensor_number = 12;  // 임의의 숫자를 넣어주었다. 
int analog = 25;  // esp에 연결된 핀 번호

IPAddress local_IP(192, 168, 219, 246); // ESP32가 사용할 IP address
IPAddress subnet(255, 255, 255, 0);   // subnet mask
IPAddress gateway(192, 168, 219, 1);    // Gateway IP address (공유기 IP주소)
IPAddress primaryDNS(164, 124, 101, 2);     // primary DNS server IP address
IPAddress secondaryDNS(203, 248, 252, 2);   // secondary DNS server IP address

IPAddress hostIp(192, 168, 219, 241);  //웹서버의 ip 주소
int SERVER_PORT = 9983;  // 웹서버 포트 번호
WiFiClient client;

StaticJsonDocument <500> doc;

#ifdef DEBUG
#define CONCAT_3(p1, p2, p3)  CONCAT_3_(p1, p2, p3)
/** Auxiliary macro used by @ref CONCAT_3 */
#define CONCAT_3_(p1, p2, p3) p1##p2##p3

#define STRINGIFY_(val) #val
/** Converts a macro argument into a character constant.*/
#define STRINGIFY(val)  STRINGIFY_(val)

/* FW and Board Information */
#define PROJECT_NAME                "MainboardData_ServerDB_Transmit"

/* board revision */
#define BD_VER_MAJOR                0
#define BD_VER_MINOR                1
/* firmware version */
#define FW_VER_MAJOR                0
#define FW_VER_MINOR                1

/* board revision */
#define HW_BD_REV_01    ( ( 0 << 4 ) | ( 1 ) )
#define STR_BD_VER      "v" STRINGIFY( BD_VER_MAJOR ) "." STRINGIFY( BD_VER_MINOR )
#define DEF_BD_VER      CONCAT_3( HW_BD_REV_01, BD_VER_MAJOR, BD_VER_MINOR )
/* firmware version */
#define STR_FW_VER                      "v" STRINGIFY( FW_VER_MAJOR ) "." STRINGIFY( FW_VER_MINOR ) "." STRINGIFY( FW_VER_PATCH )
#define STR_FW_VER_LEN                  6
#define STR_HW_VER                      "v" STRINGIFY( BD_VER_MAJOR ) "." STRINGIFY( BD_VER_MINOR )
#endif

/* Timer ch 1 interrupt service routine */
void IRAM_ATTR onTimer1()
{
    // Increment the counter and set the time of ISR
    // Period 1s
    portENTER_CRITICAL_ISR(&timerMux);

    timer1_count++;
    timer1_count2++;
    
    portEXIT_CRITICAL_ISR(&timerMux);
}

/* Display Boot Message */
void BootMessagePrint (void)
{
    Serial.printf( "----------------------------------------\r\n" );
    Serial.printf( " Project Name : %s\r\n", PROJECT_NAME );
    Serial.printf( " - HW VERSION        : %s\r\n", STR_HW_VER );
    Serial.printf( " - FW VERSION        : %s\r\n", STR_FW_VER );
    Serial.printf( " - BUILD TIME        : %s, %s\r\n", __DATE__, __TIME__ );
    Serial.printf( "----------------------------------------\r\n" );
}

/* Test Project Name Print */
void printProjectInfo (void)
{
    Serial.printf("Project Information :  ID - %d Name - %s Test count - %s\r\n",
                ext_project_info[ 4 ].id, ext_project_info[ 7 ].name, ext_project_info[ 2 ].testnum);
}

/*Test Data 1*/
void setJsonObject_PTSensor (void)
{
    uint8_t i = 0;
    String str1, str2;

    for(i=1; i < PT_DATA_MAX+1; i++)
    {
        str1 = "PT_"    + String(i) + "_Press:";
        doc[str1] = ""  + String(i) + "";
        str2 = "PT_"    + String(i) + "_Temper:";
        doc[str2] = ""  + String(i) + "";
    }
}
 
/*Test Data 2*/
void setJsonObject_SHTMSensor (void)
{
    uint8_t i = 0;
    String str1, str2;

    for(i=1; i < SHTM_DATA_MAX+1; i++)
    {
        str1 = "SHTM_" + String(i) + "_Temper:";
        doc[str1] = "" + String(i) + "";
        str2 = "SHTM_" + String(i) + "_Humi:";
        doc[str2] = "" + String(i) + "";
    }
}

/*Test Board Info Set*/
void setJsonObject_BoardInfo (BoardSetType_t board_id, ProjectSetType_t project_id)
{
    uint8_t i = 0;
    uint8_t str1, str2;

    str1 = board_id;
    str2 = project_id;

    doc["board_id"] = "" + String(str1) + "";
    doc["project"] = "" + String(str2) + "";
}

void printWifiData() 
{
    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();

    Serial.print("\r\n");
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print your MAC address:
    //byte mac[6];
    byte mac[6];

    WiFi.macAddress(mac);

    Serial.print("MAC address: ");

    Serial.printf("%02X", mac[5]);
    Serial.print(":");
    Serial.printf("%02X", mac[4]);
    Serial.print(":");
    //Serial.print(mac[3], HEX);
    Serial.printf("%02X", mac[3]);
    Serial.print(":");
    Serial.printf("%02X", mac[2]);
    Serial.print(":");
    Serial.printf("%02X", mac[1]);
    Serial.print(":");
    Serial.printf("%02X", mac[0]);
    Serial.println();
}

void printCurrentNet() 
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print the MAC address of the router you're attached to:
    //byte bssid[6];

    uint8_t *bssid = WiFi.BSSID();
    //WiFi.BSSID(bssid);
    Serial.print("BSSID: ");
    Serial.printf("%02X", bssid[5]);
    Serial.print(":");
    Serial.printf("%02X", bssid[4]);
    Serial.print(":");
    Serial.printf("%02X", bssid[3]);
    Serial.print(":");
    Serial.printf("%02X", bssid[2]);
    Serial.print(":");
    Serial.printf("%02X", bssid[1]);
    Serial.print(":");
    Serial.printf("%02X", bssid[0]);
    Serial.println();

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.println(rssi);
    Serial.print("\r\n");

    // print the encryption type:
    #if 0
    byte encryption = WiFi.encryptionType();
    Serial.print("Encryption Type:");
    Serial.println(encryption, HEX);
    #endif
}

/* Timer Initialize */
void Timer_init()
{
    /* Timer channel 1 Set */
    timer = timerBegin(1, 80, true);
    timerAttachInterrupt(timer, &onTimer1, true);
    
    /* Timer 1us x 1000 count = 1 msec */
    timerAlarmWrite(timer, 1000, true);
    timerAlarmEnable(timer);
}

ret_code_t MakeMainPacket (void)
{

}

void setup() {

    uint8_t connect_cnt = 0;
    pinMode(LED_BLUE,OUTPUT);
    
    Serial.begin(115200);

    Timer_init();
    /* Timer Setting Value */
    // timer = timerBegin(1000000);
    // timerAttachInterrupt(timer, &onTimer1);
    // timerAlarm(timer, 1000, true, 0); // 1ms

    BootMessagePrint();

    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
        Serial.println("STA failed to configure");
    }
    WiFi.begin(ssid, password);   // 와이파이 접속
    
    #if 1
    while (WiFi.status() != WL_CONNECTED)
    { //Check for the connection
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    #else
    while (WiFi.status() != WL_CONNECTED)
    { //Check for the connection
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    #endif
    
    Serial.println("Connected to the WiFi network");
}

String getTestData()
{
    //  예시
    // {
    //   "data": {
    //     "key": "i4u0qDd7y24qpRpjFFvLWDhV0PqMJiuvcZWTfDYU",
    //     "inverter" : [
    //         {"temperature" : "10"},
    //         {"temperature" : "20"}
    //     ]
    //   }
    // }
    //  구현 결과
    // {
    //   "data":{
    //     "key":"i4u0qDd7y24qpRpjFFvLWDhV0PqMJiuvcZWTfDYU",
    //     "inverter":[
    //       {"temperature":"10"},
    //       {"temperature":"20"}
    //     ]
    //   }
    // }
    String jsondata = "";

    //StaticJsonDocument <1500> doc;
    // DynamicJsonDocument doc(1024); 동적json 생성
    JsonObject root = doc.to<JsonObject>();
    /*
    JsonArray leftPressure = root.createNestedArray("leftPressure");
    JsonArray rightPressure =root.createNestedArray("rightPressure");
    JsonArray leftCop =root.createNestedArray("leftCop");
    JsonArray rightCop =root.createNestedArray("rightCop");
    JsonArray leftTemper =root.createNestedArray("leftTemper");
    JsonArray rightTemper =root.createNestedArray("rightTemper");
    leftPressure.add("1");
    leftPressure.add("2");
    leftPressure.add("3");
    leftPressure.add("4");
    */

    //printWifiData(); // WiFi 정보 출력
    printCurrentNet(); 
    printProjectInfo();

    setJsonObject_BoardInfo(SLAVE_1, INCELL);
    setJsonObject_PTSensor(); // 임시 테스트 Json 데이터 생성
    setJsonObject_SHTMSensor(); // 임시 테스트 Json 데이터 생성
    
    // doc["PT_1_Press"] = "1";
    // doc["PT_1_Temper"] = "1";
    // doc["PT_2_Press"] = "2";
    // doc["PT_2_Temper"] = "2";
    // doc["PT_3_Press"] = "3";
    // doc["PT_3_Temper"] = "3";
    // doc["PT_4_Press"] = "4";
    // doc["PT_4_Temper"] = "4";

    // doc["SHTM_1_Temper"] = "1";
    // doc["SHTM_1_Humi"] = "1";
    // doc["SHTM_2_Temper"] = "2";
    // doc["SHTM_2_Humi"] = "2";
    // doc["SHTM_3_Temper"] = "3";
    // doc["SHTM_3_Humi"] = "3";
    // doc["SHTM_4_Temper"] = "4";
    // doc["SHTM_4_Humi"] = "4";
    // doc["SHTM_5_Temper"] = "5";
    // doc["SHTM_5_Humi"] = "5";
    // doc["SHTM_6_Temper"] = "6";
    // doc["SHTM_6_Humi"] = "6";
    // doc["SHTM_7_Temper"] = "7";
    // doc["SHTM_7_Humi"] = "7";
    // doc["SHTM_8_Temper"] = "8";
    // doc["SHTM_8_Humi"] = "8";

    serializeJsonPretty(doc, jsondata);

    return (jsondata);
} 

void WiFi_Process (void)
{
    if ( MakePacketData > 0x00 )
    {
        httpRequestData = getTestData();
        Serial.println(httpRequestData); //시리얼 모니터에 Json 형식의 데이터를 찍어준다.

        MakePacketData = 0x00;
    }
    
    if ( timer1_count2 >= 1000 )
    {
        Serial.printf("Timer 1 Count Value : %d\r\n", timer1_count);

        // Serial.print("Timer 1 Count Value : ");
        // Serial.println(timer1_count);

        /* Critical section enter */
        portENTER_CRITICAL(&timerMux);
    
        timer1_count = 0;
        timer1_count2 = 0;

        /* Critical senction exit */
        portEXIT_CRITICAL(&timerMux);

        if(WiFi.status()== WL_CONNECTED)
        {   //Check WiFi connection status
            //value = analogRead(analog); // esp32에서 읽은 co2 값을 value에 저장한다.
            HTTPClient http;   
        
            //****************
            http.begin(serverName);  //Specify destination for HTTP request    "http://00.000.00.00:5000/"
            http.addHeader("Content-Type",  "apsplication/json");   //Specify content-type header,  Json형식의 타입이다.
            //****************
            // String httpRequestData = getTestData();
            // Serial.println(httpRequestData); //시리얼 모니터에 Json 형식의 데이터를 찍어준다.
            //****************
            int httpResponseCode = http.POST(httpRequestData);   //Send the actual POST request
        
            if(httpResponseCode>0)
            { 
                // 잘 전송되었으면
                //String response = http.getString();                       //Get the response to the request
                
                Serial.println(httpResponseCode);   //Print return code
                // Serial.println(response);           //Print request answer
            }
            else
            {
                Serial.print("Error on sending POST: ");
                Serial.println(httpResponseCode);
            }
            http.end();  //Free resources
            //****************
            MakePacketData = 0x01;
        }
        else
        {
            Serial.println("Error in WiFi connection");   
        }
        
        // LED Blue Pin GPIO2 Toggle.
        digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
        // delay(30000000);  //Send a request every 10 seconds
        //delay(1000);  //Send a request every 10 seconds
    }
}

void Debug_Process (void)
{
    // // Serial.printf("Timer 1 Count Value : %d\r\n", timer1_count);
    if(Serial.available() > 0)
    {
        uart_buf = Serial.readStringUntil('\n');
        Serial.println(uart_buf);
    }
}

void loop() {
    WiFi_Process();
    Debug_Process();
}
