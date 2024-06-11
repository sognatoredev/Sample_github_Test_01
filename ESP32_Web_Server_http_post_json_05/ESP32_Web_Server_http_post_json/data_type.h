
#ifndef _DATA_TYPES_H_
#define _DATA_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PT_DATA_MAX                         4
#define SHTM_DATA_MAX                       8

#define SENSOR_STATE_MAX                    8
#define SOCKET_SEND_REPORT_PACKET_LENGTH    78  // server로 던질 데이터 길이에 맞게 수정 필요함.

/* Socket packet stx, ext */
#define PACKET_DATA_STX                     0xFF
#define PACKET_DATA_ETX                     0xFE
/* Socket send packet command id */
#define PACKET_SEND_PKT_ID_INITIAL_REPORT                0x00
#define PACKET_SEND_PKT_ID_PERIODIC_REPORT               0x01
#define PACKET_SEND_PKT_ID_FINAL_REPORT                  0x05
#define PACKET_SEND_PKT_ID_EVENT_REPORT_TEMP             0x06
#define PACKET_SEND_PKT_ID_EVENT_REPORT_LEAK             0x07
#define PACKET_SEND_PKT_ID_EVENT_REPORT_HIGH_V_24        0x08
#define PACKET_SEND_PKT_ID_EVENT_REPORT_HIGH_V_110       0x09
#define PACKET_SEND_PKT_ID_EVENT_REPORT_OUTAGE           0x0A
#define PACKET_SEND_PKT_ID_EVENT_REPORT_TEMP_HUMIDITY    0x0B
#define PACKET_SEND_PKT_ID_EVENT_REPORT_GAS              0x0C
#define PACKET_SEND_PKT_ID_EVENT_REPORT_PT100            0x0D
#define PACKET_SEND_PKT_ID_UNKNOWN_REPORT                0xFF
/* Socket send packet command id */
#define PACKET_SEND_CMD_ID_INITIAL_REPORT                0x11
#define PACKET_SEND_CMD_ID_PERIODIC_REPORT               0x31
#define PACKET_SEND_CMD_ID_EVENT_REPORT                  0x41
#define PACKET_SEND_CMD_ID_FINAL_REPORT                  0x51
#define PACKET_SEND_CMD_ID_UNKNOWN                       0xFF
/* sensor state */
#define SENSOR_STATE_VALID                               ( 0x00 )
#define SENSOR_STATE_INVALID_0                           ( 0x01 )
#define SENSOR_STATE_INVALID_1                           ( 0x02 )
#define SENSOR_STATE_INVALID_2                           ( 0x03 )
#define SENSOR_STATE_UNKNOWN                             ( 0xFF )

typedef enum BoardSetType
{
    MASTER = 0,
    SLAVE_1,
    SLAVE_2,
    SLAVE_3,
    SLAVE_4,
    SLAVE_5,
    SLAVE_6,
    SLAVE_7,
    SLAVE_8,
    SLAVE_9
} BoardSetType_t;

/* Project setting info */
typedef enum ProjectSetType
{
    A2Z_SD = 0,
    DAECHANG,
    A2Z_MS,
    EVAR_M1,
    INCELL,
    KITECH,
    AIR_PTC_HEATER,
    WATER_TANK_HEATER,
    IM,
    M2_AI,
    ALL_MODEL,
    VINSSEN
} ProjectSetType_t;

typedef struct
{
    uint8_t id;
    //char * name;
    char * name;
    const char * testnum;

} ProjectExtInfo_t;

ProjectExtInfo_t ext_project_info[ 20 ] =
{
    {A2Z_SD, "A2Z_SD", "Test 01"},
    {DAECHANG, "DAECHANG", "Test 02"},
    {A2Z_MS, "A2Z_MS", "Test 03"},
    {EVAR_M1, "EVAR_M1", "Test 04"},
    {INCELL, "INCELL", "Test 05"},
    {KITECH, "KITECH", "Test 06"},
    {AIR_PTC_HEATER, "ARI_PTC_HEATER", "Test 07"},
    {WATER_TANK_HEATER, "WATER_TANK_HEATER", "Test 08"},
    {IM, "IM", "Test 09"},
    {M2_AI, "M2_AI", "Test 10"},
    {ALL_MODEL, "ALL_MODEL", "Test 11"},
    {VINSSEN, "VIBSSEN", "Test 12"}
};

// typedef struct BoardSetType
// {
//     uint8_t MASTER = 0x00;
//     uint8_t SLAVE_1 = 0x01;
//     uint8_t SLAVE_2 = 0x02;
//     uint8_t SLAVE_3 = 0x03;
//     uint8_t SLAVE_4 = 0x04;
//     uint8_t SLAVE_5 = 0x05;
//     uint8_t SLAVE_6 = 0x06;
//     uint8_t SLAVE_7 = 0x07;
//     uint8_t SLAVE_8 = 0x08;
//     uint8_t SLAVE_9 = 0x09;
// } BoardSetType_t;

// /* Project setting info */
// typedef struct
// {
//     A2Z_SD = 0,
//     DAECHANG,
//     A2Z_MS,
//     EVAR_M1,
//     INCELL,
//     KITECH,
//     AIR_PTC_HEATER,
//     WATER_TANK_HEATER,
//     IM,
//     M2_AI,
//     ALL_MODEL,
//     VINSSEN
// } ProjectSetType_t;

typedef struct FwVerData
{
    /* data */
    uint8_t major;
    uint8_t minor;
} FwVerData_t;

/* Deviece setting info */
typedef struct BoardSettingInfo
{
    uint16_t proj_num;

    uint32_t unique_id_lsb;
    uint32_t unique_id_mid;
    uint32_t unique_id_msb;

    uint16_t setboard_type;
    uint16_t setboard_id;

    uint16_t sensor_type1;
    uint16_t sensor_type2;
} BoardSettingInfo_t;

#if 1
/* Board setting info */
// ProjectSetType_t    ProjectSetType;
// BoardSetType_t      BoardSetType;
typedef struct
{
    union
    {
        struct
        {
            /*** header ***/
            uint8_t stx;                            /**< STX (Start of Text) 0xFF */
            uint8_t cmd_id;                         /**< Command ID */
            uint8_t data_length;                    /**< Data length */

            /*** body ***/
            /* device info */
            /* MCU Unique ID */
            uint16_t unique_id_9to12_l;                 /**< Unique ID LSB */
            uint16_t unique_id_9to12_m;                 /**< Unique ID MSB */
            uint16_t unique_id_5to12_l;                 /**< Unique ID LSB */
            uint16_t unique_id_5to12_m;                 /**< Unique ID MSB */
            uint16_t unique_id_1to4_l;                  /**< Unique ID LSB */
            uint16_t unique_id_1to4_m;                  /**< Unique ID MSB */

            uint16_t setboard_type;                 /**< current battery level */
            uint16_t setboard_id;                   /**< Set Board id _ Master _ Slave_n */
            
            /* Sensor type */
            uint16_t sensor_type_1;                 /**< Sensor Type id [ MSB | LSB ] */
            uint16_t sensor_type_2;                 /**< Sensor Type id [ MSB | LSB ] */

            /* sensor bd info */
            uint16_t sensor_id;
            uint16_t sensor_state[ SENSOR_STATE_MAX ];

            /* sensor value */
            int16_t pt_press[ PT_DATA_MAX ];
            int16_t pt_temperature[ PT_DATA_MAX ];
            int16_t shtm_temperature[ SHTM_DATA_MAX ];
            int16_t shtm_humi[ SHTM_DATA_MAX ];
            
            /*** footer ***/
            uint8_t etx;                                  /**< ETX (End of Text) */
        } packet;
        uint8_t data[ SOCKET_SEND_REPORT_PACKET_LENGTH ]; // length 수정 필요함
    };
} SocketSendReportPacket_t;

#else
typedef struct
{
    union
    {
        struct
        {
            /* header */
            uint8_t stx;                                  /**< STX (Start of Text) 0xFF */
            uint8_t cmd_id;                               /**< Command ID */
            uint8_t data_length;                          /**< Data length */

            /*body*/
            /* device info */
            uint32_t unique_id_lsb;                            /**< Packet ID */
            uint32_t unique_id_mid;                            /**< EUID [ MSB | LSB ] */
            uint32_t unique_id_msb;                            /**< EUID [ MSB | LSB ] */
            uint16_t setboard_type;                              /**< current battery level */
            uint16_t setboard_id;                          /**< firmware version major, minor */
            uint16_t sensor_type_1;                         /**< cell id [ MSB | LSB ] */
            uint16_t sensor_type_2;                         /**< cell id [ MSB | LSB ] */
            
            uint8_t lac[ 2 ];                             /**< LAC [ MSB | LSB ] */
            uint8_t mcc[ 2 ];                             /**< MCC [ MSB | LSB ] */
            uint8_t mnc[ 2 ];                             /**< MNC [ MSB | LSB ] */
            int8_t rsrp;                                  /**< LTE Signal Info */
            
            /* sensor bd info */
            uint8_t sensor_id;
            uint8_t sensor_state[ SENSOR_STATE_MAX ];
            /* sensor value */
            int16_t pt_press[ PT_DATA_MAX ];
            int16_t pt_temperature[ PT_DATA_MAX ];
            int16_t shtm_temperature[ SHTM_DATA_MAX ];
            int16_t shtm_humi[ SHTM_DATA_MAX ];
            // uint8_t nh3[ 2 ];
            // uint8_t h2s[ 2 ];
            // uint8_t pt100[ 2 ];
            /* operation value */
            // uint8_t acc_stdv_x[ 12 ];
            // uint8_t acc_stdv_y[ 12 ];
            // uint8_t acc_stdv_z[ 12 ];
            /* footer */
            uint8_t etx;                                  /**< ETX (End of Text) */
        } packet;
        uint8_t data[ SOCKET_SEND_REPORT_PACKET_LENGTH ]; // length 수정 필요함
    };
} SocketSendReportPacket_t;
#endif



#ifdef __cplusplus
}
#endif

#endif /* _DATA_TYPES_H_ */

