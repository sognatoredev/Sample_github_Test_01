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

// DataParsing checkout branch
// Test branch 01
SocketSendReportPacket_t makeReportPacket;
ParseReceivedData_t parseReceivedData;

/* Type define */
typedef void ( * p_PacketPrintFuncArray ) ( ParseReceivedData_t * p_parserRxData );

/* create a hardware timer */
hw_timer_t * timer = NULL;
volatile uint32_t timer1_count, timer1_count2 = 0;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

uint8_t MakePacketData = 0x00;
String httpRequestData;

/* Uart buffer define */
String uart_buf;
char uart_buf_tmp[UART_BUF_MAX] = {0};
volatile uint8_t uart_buf_cnt = 0;

uint8_t ascii2hex_arr[UART_BUF_MAX] = { 0 };

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

    timer1_count++; // state 
    timer1_count2++; // http
    
    portEXIT_CRITICAL_ISR(&timerMux);
}

//입력의 str은 모두 대문자인 16진수 변환가능한 문자열이라고 가정
//size는 str의 크기
//hex는 변환된 16진수 배열
unsigned int ascii_to_hex(const char* str, size_t size, uint8_t* hex)
{
    #if 0
    unsigned int i, h, high, low;
    for (h = 0, i = 0; i < size; i += 2, ++h) {
        //9보다 큰 경우 : 알파벳 문자 'A' 이상인 문자로, 'A'를 빼고 10을 더함.
        //9이하인 경우 : 숫자 입력으로 '0'을 빼면 실제 값이 구해짐.
        high = (str[i] > '9') ? str[i] - 'A' + 10 : str[i] - '0';
        low = (str[i + 1] > '9') ? str[i + 1] - 'A' + 10 : str[i + 1] - '0';
        //high 4비트, low 4비트이므로, 1바이트를 만들어주기 위해 high를 왼쪽으로 4비트 shift
        //이후 OR(|)연산으로 합
        hex[h] = (high << 4) | low;
    }
    return h;
    #else
    unsigned int i, h, high, low, cnt;
    for (h = 0, i = 0; i < size; i += 2, ++h) 
    {
        cnt = i;
        //9보다 큰 경우 : 알파벳 문자 'A' 이상인 문자로, 'A'를 빼고 10을 더함.
        //9이하인 경우 : 숫자 입력으로 '0'을 빼면 실제 값이 구해짐.
        high = (str[i] > '9') ?
                str[i] - 'A' + 10 :
                str[i] - '0';
        low = (str[i + 1] > '9') ? 
                str[i + 1] - 'A' + 10 :
                str[i + 1] - '0';
        //high 4비트, low 4비트이므로, 1바이트를 만들어주기 위해 high를 왼쪽으로 4비트 shift
        //이후 OR(|)연산으로 합
        hex[h] = (high << 4) | low;
    }
    /* 홀수 인 경우 */
    if (i > size )
    {
        hex[h + 1] = high;
        //Serial.printf( "ascii to hex hol in : %x\r\n", hex[h + 1]);
    }
    return h;
    #endif
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
    #if 0 // packet struct
    /*** body ***/
    /* device info */
    /* MCU Unique ID */
    uint8_t unique_id[ 12 ];                 /**< Unique ID 12 byte [ MSB | LSB ] */ 

    /* Board type */
    uint8_t setboard_type[ 2 ];                 /**< current battery level */
    uint8_t setboard_id[ 2 ];                   /**< Set Board id _ Master _ Slave_n */
    
    /* Sensor type */
    uint8_t sensor_type_1[ 2 ];                 /**< Sensor Type id [ MSB | LSB ] */
    uint8_t sensor_type_2[ 2 ];                 /**< Sensor Type id [ MSB | LSB ] */

    /* sensor bd info */
    uint8_t sensor_id[ 2 ];
    uint8_t sensor_state[ 2 ];

    /* sensor value */
    uint8_t pt_press[ PT_DATA_MAX ][ 2 ];
    uint8_t pt_temperature[ PT_DATA_MAX ][ 2 ];
    uint8_t shtm_temperature[ SHTM_DATA_MAX ][ 2 ];
    uint8_t shtm_humi[ SHTM_DATA_MAX ][ 2 ];
    #endif
}

void Print_PTpress ( ParseReceivedData_t * p_parserRxData )
{
    uint8_t i,j = 0;
    /* sensor value */
    /* PT press */
    for (i = 0; i < 4; i++)
    {
        Serial.printf("pt press %d : 0x", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", p_parserRxData->packet.pt_press[i][j]);
        }
        Serial.printf("\r\n");
    }
}

void Print_PTTemperature ( ParseReceivedData_t * p_parserRxData )
{
    uint8_t i,j = 0;
    /* sensor value */
    /* PT temperature */
    for (i = 0; i < 4; i++)
    {
        Serial.printf("pt temperature %d : 0x", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", p_parserRxData->packet.pt_temperature[i][j]);
        }
        Serial.printf("\r\n");
    }
}

void Print_SHTMTemperature ( ParseReceivedData_t * p_parserRxData )
{
    uint8_t i,j = 0;
    /* sensor value */
    /* SHTM temperature */
    for (i = 0; i < 8; i++)
    {
        Serial.printf("shtm temperature %d : 0x", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", p_parserRxData->packet.shtm_temperature[i][j]);
        }
        Serial.printf("\r\n");
    }
}

void Print_SHTMHumidity ( ParseReceivedData_t * p_parserRxData )
{
    uint8_t i,j = 0;
    /* sensor value */
    /* SHTM Humidity */
    for (i = 0; i < 8; i++)
    {
        Serial.printf("shtm humidity %d : 0x", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", p_parserRxData->packet.shtm_temperature[i][j]);
        }
        Serial.printf("\r\n");
    }
}

/* print Sensor Parsing Data */
void Print_SensorParsingData ( ParseReceivedData_t * p_parserRxData )
{
    uint8_t i = 0;
    p_PacketPrintFuncArray PacketPrintFuncArray[4] = { Print_PTpress
                                                    , Print_PTTemperature
                                                    , Print_SHTMTemperature
                                                    , Print_SHTMHumidity 
                                                    };

    for (i = 0; i < 4; i ++)
    {
        PacketPrintFuncArray[i](p_parserRxData);
    }
}

// Test converter 01
#if 1
static void printParsingdata (ParseReceivedData_t * p_parserRxData)
{
    uint8_t i = 0;

    Serial.printf("\n\n************************ data parsing print ************************\r\n");

    #if 1 // 0x hex print
        /* Header */
    Serial.printf("\n[Packet Header data]\r\nstx : 0x%02X\r\ncmd_id : 0x%02X\r\ndata_length : 0x%02X\r\n",
                    p_parserRxData->packet.stx, p_parserRxData->packet.cmd_id, p_parserRxData->packet.data_length);

    /* Body */
    /* Unique ID */
    Serial.printf("\n[Packet body data]\r\n");
    Serial.printf("unique ID : 0x");
    for (i = 0; i < 12; i++)
    {
        Serial.printf("%02X", p_parserRxData->packet.unique_id[i]);
    }
    Serial.printf("\r\n");
    /* Board type */
    Serial.printf("board type : ");
    Serial.printf("0x%02X%02X\r\n",
                    p_parserRxData->packet.setboard_type[0], p_parserRxData->packet.setboard_type[1]);
    /* Board id */
    Serial.printf("board id : ");
    Serial.printf("0x%02X%02X\r\n",
                    p_parserRxData->packet.setboard_id[0], p_parserRxData->packet.setboard_id[1]);
    /* Sensor type */
    Serial.printf("sensor type 1 : ");
    Serial.printf("0x%02X%02X\r\n",
                    p_parserRxData->packet.sensor_type_1[0], p_parserRxData->packet.sensor_type_1[1]);
    Serial.printf("sensor type 2 : ");
    Serial.printf("0x%02X%02X\r\n",
                    p_parserRxData->packet.sensor_type_2[0], p_parserRxData->packet.sensor_type_2[1]);
    /* Sensor id */
    Serial.printf("sensor id : ");
    Serial.printf("0x%02X%02X\r\n",
                    p_parserRxData->packet.sensor_id[0], p_parserRxData->packet.sensor_id[1]);
    /* Sensor state */
    Serial.printf("sensor state : ");
    Serial.printf("0x%02X%02X\r\n",
                    p_parserRxData->packet.sensor_state[0], p_parserRxData->packet.sensor_state[1]);

    /* Sensor part print */ 
    Print_SensorParsingData(p_parserRxData);

    /* Footer */
    Serial.printf("\r\n[Packet Footer data]\r\ncrc16 : 0x%02X%02X\r\netx : 0x%02X\r\n",
                    p_parserRxData->packet.crc16[0], p_parserRxData->packet.crc16[1], p_parserRxData->packet.etx);
    #else
    /* Header */
    Serial.printf("\n[Packet Header data]\r\nstx : %02X\r\ncmd_id : %02X\r\ndata_length : %02X\r\n",
                    parseReceivedData.packet.stx, parseReceivedData.packet.cmd_id, parseReceivedData.packet.data_length);

    /* Body */
    /* Unique ID */
    Serial.printf("\n[Packet body data]\r\n");
    Serial.printf("unique ID : ");
    for (i = 0; i < 12; i++)
    {
        Serial.printf("%02X", parseReceivedData.packet.unique_id[i]);
    }
    Serial.printf("\r\n");

    /* Board type */
    Serial.printf("board type : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.setboard_type[0], parseReceivedData.packet.setboard_type[1]);
    /* Board id */
    Serial.printf("board id : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.setboard_id[0], parseReceivedData.packet.setboard_id[1]);
    /* Sensor type */
    Serial.printf("sensor type 1 : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.sensor_type_1[0], parseReceivedData.packet.sensor_type_1[1]);
    Serial.printf("sensor type 2 : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.sensor_type_2[0], parseReceivedData.packet.sensor_type_2[1]);
    /* Sensor id */
    Serial.printf("sensor id : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.sensor_id[0], parseReceivedData.packet.sensor_id[1]);
    /* Sensor state */
    Serial.printf("sensor state : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.sensor_state[0], parseReceivedData.packet.sensor_state[1]);
    /* sensor value */
    /* PT press */
    for (i = 0; i < 4; i++)
    {
        Serial.printf("pt press %d : ", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.pt_press[i][j]);
        }
        Serial.printf("\r\n");
    }
    /* PT temperature */
    for (i = 0; i < 4; i++)
    {
        Serial.printf("pt temperature %d : ", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.pt_temperature[i][j]);
        }
        Serial.printf("\r\n");
    }
    /* SHTM temperature */
    for (i = 0; i < 8; i++)
    {
        Serial.printf("shtm temperature %d : ", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.shtm_temperature[i][j]);
        }
        Serial.printf("\r\n");
    }
    /* SHTM press */
    for (i = 0; i < 8; i++)
    {
        Serial.printf("shtm humidity %d : ", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.shtm_humi[i][j]);
        }
        Serial.printf("\r\n");
    }

    /* Footer */
    Serial.printf("\r\n[Packet Footer data]\r\ncrc16 : %02X%02X\r\netx : %02X\r\n",
                    parseReceivedData.packet.crc16[0], parseReceivedData.packet.crc16[1], parseReceivedData.packet.etx);
    #endif
    
    Serial.printf("\n************************ end ************************\r\n\n");

    #if 0
    for (i = 0; i < 4; i++)
    {
        makePacket.packet.pt_temperature[i] = pData[num++];
    }

    for (i = 0; i < 8; i++)
    {
        makePacket.packet.shtm_temperature[i] = pData[num++];
    }

    for (i = 0; i < 8; i++)
    {
        makePacket.packet.shtm_humi[i] = pData[num++];
    }

    
    /* Footer */
    makePacket.packet.crc16[0] = pData[num++];
    makePacket.packet.crc16[1] = pData[num++];
    makePacket.packet.etx = pData[num++];
    #endif
}
#else
void printParsingdata (void)
{
    uint8_t i = 0;

    Serial.printf("\n\n************************ data parsing print ************************\r\n");

    #if 1 // 0x hex print
        /* Header */
    Serial.printf("\n[Packet Header data]\r\nstx : 0x%02X\r\ncmd_id : 0x%02X\r\ndata_length : 0x%02X\r\n",
                    parseReceivedData.packet.stx, parseReceivedData.packet.cmd_id, parseReceivedData.packet.data_length);

    /* Body */
    /* Unique ID */
    Serial.printf("\n[Packet body data]\r\n");
    Serial.printf("unique ID : 0x");
    for (i = 0; i < 12; i++)
    {
        Serial.printf("%02X", parseReceivedData.packet.unique_id[i]);
    }
    Serial.printf("\r\n");
    /* Board type */
    Serial.printf("board type : ");
    Serial.printf("0x%02X%02X\r\n",
                    parseReceivedData.packet.setboard_type[0], parseReceivedData.packet.setboard_type[1]);
    /* Board id */
    Serial.printf("board id : ");
    Serial.printf("0x%02X%02X\r\n",
                    parseReceivedData.packet.setboard_id[0], parseReceivedData.packet.setboard_id[1]);
    /* Sensor type */
    Serial.printf("sensor type 1 : ");
    Serial.printf("0x%02X%02X\r\n",
                    parseReceivedData.packet.sensor_type_1[0], parseReceivedData.packet.sensor_type_1[1]);
    Serial.printf("sensor type 2 : ");
    Serial.printf("0x%02X%02X\r\n",
                    parseReceivedData.packet.sensor_type_2[0], parseReceivedData.packet.sensor_type_2[1]);
    /* Sensor id */
    Serial.printf("sensor id : ");
    Serial.printf("0x%02X%02X\r\n",
                    parseReceivedData.packet.sensor_id[0], parseReceivedData.packet.sensor_id[1]);
    /* Sensor state */
    Serial.printf("sensor state : ");
    Serial.printf("0x%02X%02X\r\n",
                    parseReceivedData.packet.sensor_state[0], parseReceivedData.packet.sensor_state[1]);
    /* sensor value */
    /* PT press */
    for (i = 0; i < 4; i++)
    {
        Serial.printf("pt press %d : 0x", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.pt_press[i][j]);
        }
        Serial.printf("\r\n");
    }
    /* PT temperature */
    for (i = 0; i < 4; i++)
    {
        Serial.printf("pt temperature %d : 0x", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.pt_temperature[i][j]);
        }
        Serial.printf("\r\n");
    }
    /* SHTM temperature */
    for (i = 0; i < 8; i++)
    {
        Serial.printf("shtm temperature %d : 0x", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.shtm_temperature[i][j]);
        }
        Serial.printf("\r\n");
    }
    /* SHTM press */
    for (i = 0; i < 8; i++)
    {
        Serial.printf("shtm humidity %d : 0x", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.shtm_humi[i][j]);
        }
        Serial.printf("\r\n");
    }

    /* Footer */
    Serial.printf("\r\n[Packet Footer data]\r\ncrc16 : 0x%02X%02X\r\netx : 0x%02X\r\n",
                    parseReceivedData.packet.crc16[0], parseReceivedData.packet.crc16[1], parseReceivedData.packet.etx);
    #else
    /* Header */
    Serial.printf("\n[Packet Header data]\r\nstx : %02X\r\ncmd_id : %02X\r\ndata_length : %02X\r\n",
                    parseReceivedData.packet.stx, parseReceivedData.packet.cmd_id, parseReceivedData.packet.data_length);

    /* Body */
    /* Unique ID */
    Serial.printf("\n[Packet body data]\r\n");
    Serial.printf("unique ID : ");
    for (i = 0; i < 12; i++)
    {
        Serial.printf("%02X", parseReceivedData.packet.unique_id[i]);
    }
    Serial.printf("\r\n");

    /* Board type */
    Serial.printf("board type : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.setboard_type[0], parseReceivedData.packet.setboard_type[1]);
    /* Board id */
    Serial.printf("board id : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.setboard_id[0], parseReceivedData.packet.setboard_id[1]);
    /* Sensor type */
    Serial.printf("sensor type 1 : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.sensor_type_1[0], parseReceivedData.packet.sensor_type_1[1]);
    Serial.printf("sensor type 2 : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.sensor_type_2[0], parseReceivedData.packet.sensor_type_2[1]);
    /* Sensor id */
    Serial.printf("sensor id : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.sensor_id[0], parseReceivedData.packet.sensor_id[1]);
    /* Sensor state */
    Serial.printf("sensor state : ");
    Serial.printf("%02X%02X\r\n",
                    parseReceivedData.packet.sensor_state[0], parseReceivedData.packet.sensor_state[1]);
    /* sensor value */
    /* PT press */
    for (i = 0; i < 4; i++)
    {
        Serial.printf("pt press %d : ", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.pt_press[i][j]);
        }
        Serial.printf("\r\n");
    }
    /* PT temperature */
    for (i = 0; i < 4; i++)
    {
        Serial.printf("pt temperature %d : ", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.pt_temperature[i][j]);
        }
        Serial.printf("\r\n");
    }
    /* SHTM temperature */
    for (i = 0; i < 8; i++)
    {
        Serial.printf("shtm temperature %d : ", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.shtm_temperature[i][j]);
        }
        Serial.printf("\r\n");
    }
    /* SHTM press */
    for (i = 0; i < 8; i++)
    {
        Serial.printf("shtm humidity %d : ", i);
        for (uint8_t j = 0; j < 2; j++)
        {
            Serial.printf("%02X", parseReceivedData.packet.shtm_humi[i][j]);
        }
        Serial.printf("\r\n");
    }

    /* Footer */
    Serial.printf("\r\n[Packet Footer data]\r\ncrc16 : %02X%02X\r\netx : %02X\r\n",
                    parseReceivedData.packet.crc16[0], parseReceivedData.packet.crc16[1], parseReceivedData.packet.etx);
    #endif
    
    Serial.printf("\n************************ end ************************\r\n\n");

    #if 0
    for (i = 0; i < 4; i++)
    {
        makePacket.packet.pt_temperature[i] = pData[num++];
    }

    for (i = 0; i < 8; i++)
    {
        makePacket.packet.shtm_temperature[i] = pData[num++];
    }

    for (i = 0; i < 8; i++)
    {
        makePacket.packet.shtm_humi[i] = pData[num++];
    }

    
    /* Footer */
    makePacket.packet.crc16[0] = pData[num++];
    makePacket.packet.crc16[1] = pData[num++];
    makePacket.packet.etx = pData[num++];
    #endif
}
#endif

#if 1
ret_code_t parse_MainboardPacket ( ParseReceivedData_t * p_parserRxData, uint8_t * pData )
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint16_t num = 0;

    /* Header */
    p_parserRxData->packet.stx = pData[num++];
    p_parserRxData->packet.cmd_id = pData[num++];
    p_parserRxData->packet.data_length = pData[num++];

    /* Body */
    /* Unique ID */
    for (i = 0; i < 12; i++)
    {
        p_parserRxData->packet.unique_id[i] = pData[num++];
    }
    /* Board type */
    p_parserRxData->packet.setboard_type[0] = pData[num++];
    p_parserRxData->packet.setboard_type[1] = pData[num++];
    /* Board id */
    p_parserRxData->packet.setboard_id[0] = pData[num++];
    p_parserRxData->packet.setboard_id[1] = pData[num++];
    /* Sensor type */
    p_parserRxData->packet.sensor_type_1[0] = pData[num++];
    p_parserRxData->packet.sensor_type_1[1] = pData[num++];

    p_parserRxData->packet.sensor_type_2[0] = pData[num++];
    p_parserRxData->packet.sensor_type_2[1] = pData[num++];
    /* Sensor boardinfo*/
    p_parserRxData->packet.sensor_id[0] = pData[num++];
    p_parserRxData->packet.sensor_id[1] = pData[num++];
    p_parserRxData->packet.sensor_state[0] = pData[num++];
    p_parserRxData->packet.sensor_state[1] = pData[num++];
    /* sensor value */
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 2; j++)
        {
            p_parserRxData->packet.pt_press[i][j] = (uint8_t) pData[num++];
        }
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 2; j++)
        {
            p_parserRxData->packet.pt_temperature[i][j] = (uint8_t) pData[num++];
        }
    }

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 2; j++)
        {
            p_parserRxData->packet.shtm_temperature[i][j] = (uint8_t) pData[num++];
        }
    }

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 2; j++)
        {
            p_parserRxData->packet.shtm_humi[i][j] = (uint8_t) pData[num++];
        }
    }

    /* Footer */
    p_parserRxData->packet.crc16[0] = pData[num++];
    p_parserRxData->packet.crc16[1] = pData[num++];
    p_parserRxData->packet.etx = pData[num++];

    printParsingdata(p_parserRxData);

    return num; // 50
}
#else
ret_code_t parse_MainboardPacket ( uint8_t * pData )
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint16_t num = 0;

    /* Header */
    parseReceivedData.packet.stx = pData[num++];
    parseReceivedData.packet.cmd_id = pData[num++];
    parseReceivedData.packet.data_length = pData[num++];

    /* Body */
    /* Unique ID */
    for (i = 0; i < 12; i++)
    {
        parseReceivedData.packet.unique_id[i] = pData[num++];
    }
    /* Board type */
    parseReceivedData.packet.setboard_type[0] = pData[num++];
    parseReceivedData.packet.setboard_type[1] = pData[num++];
    /* Board id */
    parseReceivedData.packet.setboard_id[0] = pData[num++];
    parseReceivedData.packet.setboard_id[1] = pData[num++];
    /* Sensor type */
    parseReceivedData.packet.sensor_type_1[0] = pData[num++];
    parseReceivedData.packet.sensor_type_1[1] = pData[num++];

    parseReceivedData.packet.sensor_type_2[0] = pData[num++];
    parseReceivedData.packet.sensor_type_2[1] = pData[num++];
    /* Sensor boardinfo*/
    parseReceivedData.packet.sensor_id[0] = pData[num++];
    parseReceivedData.packet.sensor_id[1] = pData[num++];
    parseReceivedData.packet.sensor_state[0] = pData[num++];
    parseReceivedData.packet.sensor_state[1] = pData[num++];
    /* sensor value */
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 2; j++)
        {
            parseReceivedData.packet.pt_press[i][j] = (uint8_t) pData[num++];
        }
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 2; j++)
        {
            parseReceivedData.packet.pt_temperature[i][j] = (uint8_t) pData[num++];
        }
    }

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 2; j++)
        {
            parseReceivedData.packet.shtm_temperature[i][j] = (uint8_t) pData[num++];
        }
    }

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 2; j++)
        {
            parseReceivedData.packet.shtm_humi[i][j] = (uint8_t) pData[num++];
        }
    }

    /* Footer */
    parseReceivedData.packet.crc16[0] = pData[num++];
    parseReceivedData.packet.crc16[1] = pData[num++];
    parseReceivedData.packet.etx = pData[num++];

    return num; // 50
}
#endif


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

void StateLED_Process (void)
{
    if ( timer1_count >= 1000 )
    {
        timer1_count = 0;
        digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
    }
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
        Serial.printf("Timer 1 Count Value : %d\r\n", timer1_count2);

        // Serial.print("Timer 1 Count Value : ");
        // Serial.println(timer1_count);

        /* Critical section enter */
        portENTER_CRITICAL(&timerMux);
    
        //timer1_count = 0;
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
        // digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
        // delay(30000000);  //Send a request every 10 seconds
        //delay(1000);  //Send a request every 10 seconds
    }
}

/* Test __ Temperature Converter 2byte signaed int. */
float Temperature_Converter (uint16_t temperatureValue)
{
    uint8_t symbol = 0;
    uint16_t Value = 0;

    float fValue = 0.0;
    
    if(0x8000 & temperatureValue)
    {
        symbol = '-';
        Value = temperatureValue - 0x8000;
    }
    else
    {
        symbol = '+';
        Value = temperatureValue;
    }

    fValue = (float) Value;
    Serial.printf("Test Converter Temperature : %c%d.%d\r\n", symbol, Value / 10, Value % 10);
    
    return fValue;
}

void Debug_Process (void)
{
    uint8_t i = 0;
    uint16_t tmp_temperature = 0x80cd;    // Test variable      0x80cd = -20.5     0x00cd = +20.5 
    uint8_t ascii2hex_length = 0;

    // // Serial.printf("Timer 1 Count Value : %d\r\n", timer1_count);
    #if 1
    if(Serial.available() > 0)
    {
        uart_buf = Serial.readStringUntil('\n');
        uart_buf_cnt = uart_buf.length();

        uart_buf.toCharArray(uart_buf_tmp, uart_buf_cnt + 2);
        Serial.printf("Uart Buffer Length Count Value : %d\r\n", uart_buf_cnt);
        Serial.printf("Test Temperature : %04X\r\n", tmp_temperature); // Test print.
        Temperature_Converter(tmp_temperature); // Test print.
        
        //Serial.println(uart_buf_tmp);

        strupr(uart_buf_tmp);
        ascii2hex_length = ascii_to_hex(uart_buf_tmp, uart_buf_cnt, ascii2hex_arr);
        //Serial.println(ascii2hex_arr);

        if (( uart_buf_cnt % 2 ) == 0 )
        {
            //scii_to_hex(uart_buf_tmp, uart_buf_cnt, ascii2hex_arr);
            for (i = 0; i < (uart_buf_cnt / 2); i++)
            {
                Serial.printf("%02X", ascii2hex_arr[i]);
            }
            Serial.print("\r\n");
        }
        else if (( uart_buf_cnt % 2 ) > 0 )
        {
            for (i = 0; i < (uart_buf_cnt / 2); i++)
            {
                // Serial.printf("%02X", ascii2hex_arr[i]);
                Serial.printf("%02X", ascii2hex_arr[i]);
            }
            Serial.printf("%X\r\n", ascii2hex_arr[ascii2hex_length + 1]);
            Serial.printf("Last odd number: %x \r\n", ascii2hex_arr[ascii2hex_length + 1]);

        }

        Serial.printf("parsing data length : %d\r\n", parse_MainboardPacket(&parseReceivedData, ascii2hex_arr));
        //printParsingdata(&parseReceivedData);
    }
    #else
    if(Serial.available() > 0)
    {
        char str[128] = "CAFEcafe0102";
        size_t size = strlen(str);
        uint8_t hex2[128] = { 0, };
        //Serial.println(uart_buf_tmp);
        uart_buf = Serial.readStringUntil('\n');



        strupr(str);

        ascii_to_hex(str, size, hex2);
        //Serial.println(ascii2hex_arr);
        for (i = 0; i < (size / 2); i++)
        {
            Serial.printf("%02X\n", hex2[i]);
        }
        
    }
    #endif
    //MakeMainPacket();
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

void loop() {
    StateLED_Process();
    WiFi_Process();
    Debug_Process();
}
