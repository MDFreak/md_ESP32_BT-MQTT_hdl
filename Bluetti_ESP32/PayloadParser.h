#ifndef PAYLOAD_PARSER_H
#define PAYLOAD_PARSER_H
#include "Arduino.h"
//#include "DeviceType.h"
#include "Device_AC300.h"
#include <md_util.h>
//#include <md_defines.h>

#define HEADER_SIZE 4
#define CHECKSUM_SIZE 2

void     init_dev_fields(device_field_data_t* _pdev_state, device_field_data_t* _pdev_comm,
                         device_field_data_t* _ppoll_comm, device_field_data_t* _pscan_comm);
uint16_t parse_uint_field(uint8_t data[]);
bool     parse_bool_field(uint8_t data[]);
float    parse_decimal_field(uint8_t data[]);
uint64_t parse_datime_field(uint8_t data[]);
uint64_t parse_serial_field(uint8_t data[]);
float    parse_version_field(uint8_t data[]);
String   parse_string_field(uint8_t data[]);
String   parse_enum_field(uint8_t data[]);

void     parse_bluetooth_data(device_field_data_t* pdev_field_data, uint8_t page, uint8_t offset, uint8_t* pData, size_t length);

#endif
