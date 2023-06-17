#ifndef _BLUETTI_PAYLOAD_PARSER_H_
  #define _BLUETTI_PAYLOAD_PARSER_H_
  #include "Arduino.h"
  //#include "DeviceType.h"
  #include "Bluetti_AC300.h"
  #include <md_util.h>
  //#include <md_defines.h>

  #define HEADER_SIZE 4
  #define CHECKSUM_SIZE 2

  void     init_blu_dev_fields(bluetti_dev_f_data_t* _pdev_state,
                               bluetti_dev_f_data_t* _pdev_comm,
                               bluetti_dev_f_data_t* ppoll_comm);
  uint16_t parse_blu_uint_field(uint8_t data[]);
  bool     parse_blu_bool_field(uint8_t data[]);
  float    parse_blu_decimal_field(uint8_t data[], uint8_t scale);
  uint64_t parse_blu_serial_field(uint8_t data[]);
  float    parse_blu_version_field(uint8_t data[]);
  String   parse_blu_string_field(uint8_t data[]);
  String   parse_blu_enum_field(uint8_t data[]);
  void     parse_blu_bt_data(bluetti_dev_f_data_t* pdev_field_data, uint8_t page, uint8_t offset, uint8_t* pData, size_t length);
#endif // _BLUETTI_PAYLOAD_PARSER_H_
