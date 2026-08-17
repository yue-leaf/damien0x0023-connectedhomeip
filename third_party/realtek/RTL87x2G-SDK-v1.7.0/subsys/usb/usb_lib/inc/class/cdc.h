#ifndef __CDC_H__
#define __CDC_H__
/*
 * CDC brequest field value
 */
#define CDC_SEND_ENCAPSULATED_COMMAND               0x00
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01
#define CDC_SET_COMM_FEATURE                        0x02
#define CDC_GET_COMM_FEATURE                        0x03
#define CDC_CLEAR_COMM_FEATURE                      0x04
#define CDC_SET_LINE_CODING                         0x20
#define CDC_GET_LINE_CODING                         0x21
#define CDC_SET_CONTROL_LINE_STATE                  0x22
#define CDC_SEND_BREAK                              0x23
#define USB_CLASS_CODE_COMM                         0x02
#define USB_CDC_SUBCLASS_CM                         1
#define USB_CDC_SUBCLASS_ACM                        2
#define USB_CDC_SUBCLASS_ECM                        6
#define USB_CDC_ETHERNET_FUNC_DESC                  0x0F

/** Communications Class Subclass Codes */
#define ACM_SUBCLASS   0x02
#define ECM_SUBCLASS   0x06
#define EEM_SUBCLASS   0x0c

/**
 * @brief Class-Specific Request Codes for Ethernet subclass
 * @note ECM120.pdf, 6.2, Table 6
 */
#define CDC_SET_ETHERNET_MULTICAST_FILTERS 0x40
#define CDC_SET_ETHERNET_PM_FILTER         0x41
#define CDC_GET_ETHERNET_PM_FILTER         0x42
#define CDC_SET_ETHERNET_PACKET_FILTER     0x43
#define CDC_GET_ETHERNET_STATISTIC         0x44

/** Ethernet Packet Filter Bitmap */
#define PACKET_TYPE_MULTICAST          0x10
#define PACKET_TYPE_BROADCAST          0x08
#define PACKET_TYPE_DIRECTED           0x04
#define PACKET_TYPE_ALL_MULTICAST      0x02
#define PACKET_TYPE_PROMISCUOUS        0x01

#pragma pack(push)
#pragma pack(1)
typedef struct t_cdc_header_function_desc
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint16_t bcdCDC;
} T_CDC_HEADER_FUNC_DESC;
typedef struct t_cdc_acm_function_desc
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
} T_CDC_ACM_FUNC_DESC;
typedef struct t_cm_func_desc
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
    uint8_t bDataInterface;
} T_CM_FUNC_DESC;
typedef struct t_cdc_union_function_desc
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bControlInterface;
    uint8_t bSubordinateInterface;
} T_CDC_UNION_FUNC_DESC;

typedef struct t_cdc_ecm_function_desc
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t iMACAddress;
    uint32_t bmEthernetStatistics;
    uint16_t wMaxSegmentSize;
    uint16_t wNumberMCFilters;
    uint8_t bNumberPowerFilters;
} T_ECM_FUNC_DESC;


typedef struct t_line_coding
{
    uint32_t dwDTERate;
    uint8_t  bCharFormat;
    uint8_t bParityType;
    uint8_t bDataBits;
} T_LINE_CODING;
#pragma pack(pop)
#endif
