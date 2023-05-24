//#include "BluettiConfig.h"
//#include "MQTT.h"
#include "PayloadParser.h"
//#include "BWifi.h"

device_field_data_t* pbluetti_device_state    = NULL;
device_field_data_t* pbluetti_device_command  = NULL;
device_field_data_t* pbluetti_polling_command = NULL;

void     init_dev_fields(device_field_data_t* _pdev_state, device_field_data_t* _pdev_comm, device_field_data_t* _ppoll_comm)
  {
    pbluetti_device_state    = _pdev_state;
    //pbluetti_device_command  = pbluetti_polling_command;
    pbluetti_device_command  = _pdev_comm;
    pbluetti_polling_command = _ppoll_comm;
  }
uint16_t parse_uint_field(uint8_t data[])
  {
    return ((uint16_t) data[0] << 8 ) | (uint16_t) data[1];
  }
bool     parse_bool_field(uint8_t data[])
  {
    return (data[1]) == 1;
  }
float    parse_decimal_field(uint8_t data[], uint8_t scale)
  {
    uint16_t raw_value = ((uint16_t) data[0] << 8 ) | (uint16_t) data[1];
    return (raw_value) / pow(10, scale);
  }
float    parse_version_field(uint8_t data[])
  {
   uint16_t low = ((uint16_t) data[0] << 8 ) | (uint16_t) data[1];
   uint16_t high = ((uint16_t) data[2] << 8) | (uint16_t) data[3];
   long val = (low ) | (high << 16) ;

   return (float) val/100;
  }
uint64_t parse_serial_field(uint8_t data[])
  {

   uint16_t val1 = ((uint16_t) data[0] << 8 ) | (uint16_t) data[1];
   uint16_t val2 = ((uint16_t) data[2] << 8 ) | (uint16_t) data[3];
   uint16_t val3 = ((uint16_t) data[4] << 8 ) | (uint16_t) data[5];
   uint16_t val4 = ((uint16_t) data[6] << 8 ) | (uint16_t) data[7];

   uint64_t sn =  ((((uint64_t) val1) | ((uint64_t) val2 << 16)) | ((uint64_t) val3 << 32)) | ((uint64_t) val4 << 48);

   return  sn;
  }
String   parse_string_field(uint8_t data[])
  {
    return String((char*) data);
  }
String   parse_enum_field(uint8_t data[])
  {
    return "";
  }

void     parse_bluetooth_data(device_field_data_t* pdev_field_data,
                              uint8_t page, uint8_t offset, uint8_t* pData, size_t length){
    //char mqttMessage[200];
    if (pdev_field_data)
      {
        SVAL(" parse..data set pdev_field_data ", (uint32_t) pdev_field_data);
        pbluetti_device_state = pdev_field_data;
      }

    switch(pData[1])
      {
        // range request
        case 0x03:
            //for(int i=0; i< sizeof(bluetti_device_state)/sizeof(device_field_data_t); i++)
            for(int i=0; i< FIELD_IDX_MAX; i++)
              {
                // filter fields not in range
                if(pbluetti_device_state[i].f_page == page &&
                   pbluetti_device_state[i].f_offset >= offset &&
                   pbluetti_device_state[i].f_offset <= (offset + length)/2 &&
                   pbluetti_device_state[i].f_offset + pbluetti_device_state[i].f_size-1 >= offset &&
                   pbluetti_device_state[i].f_offset + pbluetti_device_state[i].f_size-1 <= (offset + length)/2
                  )
                  {

                    uint8_t data_start = (2* ((int)pbluetti_device_state[i].f_offset - (int)offset)) + HEADER_SIZE;
                    uint8_t data_end = (data_start + 2 * pbluetti_device_state[i].f_size);
                    uint8_t data_payload_field[data_end - data_start];

                    int p_index = 0;
                    for (int i=data_start; i<= data_end; i++)
                      {
                        data_payload_field[p_index] = pData[i-1];
                        p_index++;
                      }
                    //S3HEXVAL(" parser device_state idx p_f_value[idx] 6 p_f_... ", i, (uint32_t) pbluetti_device_state[i].p_f_value,
                    //                                                                  (uint32_t) &pbluetti_device_state[i].p_f_value);
                    switch (pbluetti_device_state[i].f_type)
                      {
                        case UINT_FIELD:
                            //publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(data_payload_field)));
                            if (pbluetti_device_state[i].p_f_value > NULL)
                              {
                                *((uint16_t*) pbluetti_device_state[i].p_f_value) = parse_uint_field(data_payload_field);
                              }
                            //S2VAL(DEVICE_F_NAMES[pbluetti_device_state[i].f_name], pbluetti_device_state[i].f_name, String(parse_uint_field(data_payload_field)));
                            //SVAL(bluetti_device_state[i].f_name, *((uint16_t*) bluetti_device_state[i].p_f_value));
                          break;
                        case BOOL_FIELD:
                            //publishTopic(bluetti_device_state[i].f_name, String((int)parse_bool_field(data_payload_field)));
                            if (pbluetti_device_state[i].p_f_value > NULL)
                              {
                                *((bool*) pbluetti_device_state[i].p_f_value) = parse_bool_field(data_payload_field);
                              }
                            //S2VAL(DEVICE_F_NAMES[pbluetti_device_state[i].f_name], pbluetti_device_state[i].f_name, String((int)parse_bool_field(data_payload_field)));
                            //SVAL(bluetti_device_state[i].f_name, *((bool*) bluetti_device_state[i].p_f_value));
                          break;
                        case DECIMAL_FIELD:
                            //publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(data_payload_field, bluetti_device_state[i].f_scale ), 2) );
                            if (pbluetti_device_state[i].p_f_value > NULL)
                              {
                                *((float*) pbluetti_device_state[i].p_f_value) = parse_decimal_field(data_payload_field, pbluetti_device_state[i].f_scale);
                              }
                            S2VAL(DEVICE_F_NAMES[pbluetti_device_state[i].f_name], pbluetti_device_state[i].f_name, String(parse_decimal_field(data_payload_field, pbluetti_device_state[i].f_scale ), 2) );
                            //SVAL(bluetti_device_state[i].f_name, *((float*) bluetti_device_state[i].p_f_value) );
                          break;
                        case SN_FIELD:
                            char sn[16];
                            sprintf(sn, "%lld", parse_serial_field(data_payload_field));
                            SVAL(" Seriennr ",sn);
                            //publishTopic(bluetti_device_state[i].f_name, String(sn));
                            if (pbluetti_device_state[i].p_f_value > NULL)
                              {
                                *((String*) pbluetti_device_state[i].p_f_value) = String(sn);
                              }
                            //S2VAL(DEVICE_F_NAMES[pbluetti_device_state[i].f_name], pbluetti_device_state[i].f_name, String(sn));
                            //SVAL(bluetti_device_state[i].f_name, *((String*) bluetti_device_state[i].p_f_value));
                          break;
                        case VERSION_FIELD:
                            //publishTopic(bluetti_device_state[i].f_name, String(parse_version_field(data_payload_field),2) );
                            if (pbluetti_device_state[i].p_f_value > NULL)
                              {
                                *((float*) pbluetti_device_state[i].p_f_value) = parse_version_field(data_payload_field);
                              }
                            //S2VAL(DEVICE_F_NAMES[pbluetti_device_state[i].f_name], pbluetti_device_state[i].f_name, String(parse_version_field(data_payload_field),2) );
                            //SVAL(bluetti_device_state[i].f_name, *((float*) bluetti_device_state[i].p_f_value) );
                          break;
                        case STRING_FIELD:
                            //publishTopic(bluetti_device_state[i].f_name, parse_string_field(data_payload_field));
                            if (pbluetti_device_state[i].p_f_value > NULL)
                              {
                                *((String*) pbluetti_device_state[i].p_f_value) = parse_string_field(data_payload_field);
                              }
                            //S2VAL(DEVICE_F_NAMES[pbluetti_device_state[i].f_name], pbluetti_device_state[i].f_name, parse_string_field(data_payload_field));
                            //SVAL(bluetti_device_state[i].f_name, *((String*) bluetti_device_state[i].p_f_value));
                          break;
                        case ENUM_FIELD:
                            //publishTopic(bluetti_device_state[i].f_name, pase_enum_field(data_payload_field));
                            //S2VAL(DEVICE_F_NAMES[pbluetti_device_state[i].f_name], pbluetti_device_state[i].f_name, parse_enum_field(data_payload_field));
                          break;
                        default:
                          break;
                      }

                  }
                  else
                  {
                    //AddtoMsgView(String(millis()) + ": skip filtered field: "+ String(bluetti_device_state[i].f_name));
                  }
              }
          break;
        case 0x06:
          //AddtoMsgView(String(millis()) + ":skip 0x06 request! page: "+ String(page) + " offset: " + offset);
          break;
        default:
          //AddtoMsgView(String(millis()) + ":skip unknow request! page: "+ String(page) + " offset: " + offset);
          break;
      }

}
