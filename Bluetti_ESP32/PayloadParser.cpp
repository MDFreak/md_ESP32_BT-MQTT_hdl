//#include "BluettiConfig.h"
//#include "MQTT.h"
#include "PayloadParser.h"
//#include "BWifi.h"

device_field_data_t* pbluetti_device_state    = NULL;
device_field_data_t* pbluetti_device_command  = NULL;
device_field_data_t* pbluetti_polling_command = NULL;
device_field_data_t* pbluetti_scan_command    = NULL;

void     init_dev_fields(device_field_data_t* _pdev_state, device_field_data_t* _pdev_comm,
                         device_field_data_t* _ppoll_comm, device_field_data_t* _pscan_comm)
  {
    pbluetti_device_state    = _pdev_state;
    //pbluetti_device_command  = pbluetti_polling_command;
    pbluetti_device_command  = _pdev_comm;
    pbluetti_polling_command = _ppoll_comm;
    pbluetti_scan_command    = _pscan_comm;
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
uint64_t parse_datime_field(uint8_t data[])
  {

    uint16_t val1 = ((uint16_t) data[0] << 8 ) | (uint16_t) data[1];
    uint16_t val2 = ((uint16_t) data[2] << 8 ) | (uint16_t) data[3];
    uint16_t val3 = ((uint16_t) data[4] << 8 ) | (uint16_t) data[5];

    uint64_t dt =  ((((uint64_t) val1) | ((uint64_t) val2 << 16)) | ((uint64_t) val3 << 32));
    return dt;
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
                              uint8_t page, uint8_t offset, uint8_t* pData, size_t length)
  {
    char     debMsg[200];
    String   tmps;
    float    tmpf;
    uint16_t tmp16;
    uint64_t tmp64;
    bool     tmpb;
    char     tmpc18[20];
    uint8_t* p8;

    //SOUT(" parse .. ");
    if (pdev_field_data)
      {
        SVAL(" parse..data set pdev_field_data ", (uint32_t) pdev_field_data);
        pbluetti_device_state = pdev_field_data;
      }
            //SOUT(" parse "); SOUT((uint32_t) pData); SOUT("  "); SOUTHEX(pData[0]);  SOUT(" "); SOUTHEX(length);  SOUT("  ");

    switch(pData[1])
      {
        // range request
        case 0x03:
            // select scan mode
            if (length < 0x100)  // < 0x100: mode = normal, > 0x100: mode = scan
              {
                //for(int i=0; i< sizeof(bluetti_device_state)/sizeof(device_field_data_t); i++)
                length -= 5;  // protokoll overhead
                //S3VAL(" found 0x03 page offs length ", page, offset, length); SOUTLN();
                for(int i=0; (i<FIELD_IDX_MAX); i++)
                  {
                        //tmps = " check  "; tmps.concat(DEVICE_F_NAMES[pbluetti_device_state[i].f_name]);
                        //S4VAL(tmps.c_str(), " f_age f_offs f_size ", pbluetti_device_state[i].f_page, pbluetti_device_state[i].f_offset, pbluetti_device_state[i].f_size);
                    // filter fields not in range
                    if (   (pbluetti_scan_command[i].f_page == page)
                        && (pbluetti_device_state[i].f_offset >= offset)
                        //&& (pbluetti_device_state[i].f_offset <= ((offset + length)/2))
                        //&& ((pbluetti_device_state[i].f_offset + pbluetti_device_state[i].f_size - 1) >= offset)
                        && ((pbluetti_device_state[i].f_offset + pbluetti_device_state[i].f_size - 1) <= (offset + length/2 - 1))
                       )
                      {
                        uint8_t data_start = (2* ((int)pbluetti_device_state[i].f_offset - (int)offset)) + HEADER_SIZE;
                        uint8_t data_end   = (data_start + 2 * pbluetti_device_state[i].f_size);
                        uint8_t data_payload_field[data_end - data_start];

                        int p_index = 0;
                        for (int i=data_start; i<= data_end; i++)
                          {
                            data_payload_field[p_index] = pData[i-1];
                            p_index++;
                          }

                        //STXT("Copy data:");
                        //serHEXdump(data_payload_field,sizeof(data_payload_field));
                        // S3HEXVAL(" parser device_state idx p_f_value[idx] 6 p_f_... ", i, (uint32_t) pbluetti_device_state[i].p_f_value,
                        //                                                                   (uint32_t) &pbluetti_device_state[i].p_f_value);
                        switch (pbluetti_device_state[i].f_type)
                          {
                            case UINT_FIELD:
                                tmp16 = parse_uint_field(data_payload_field);
                                if (pbluetti_device_state[i].p_f_value > NULL)
                                  {
                                    if (0) //(*((uint16_t*) pbluetti_device_state[i].p_f_value) == tmpi)
                                      {
                                        S4VAL(" UINT_FIELD stay idx ", DEVICE_F_NAMES[pbluetti_device_state[i].f_name], (uint32_t) pbluetti_device_state[i].p_f_value,
                                                                       "    ", tmp16); //SOUTLN();
                                        // nothing to do
                                      }
                                    else
                                      {
                                        pbluetti_device_state[i].f_new = TRUE;
                                        *((uint16_t*) pbluetti_device_state[i].p_f_value) = tmp16;
                                        S4VAL(" UINT_FIELD new idx ", DEVICE_F_NAMES[pbluetti_device_state[i].f_name], (uint32_t) pbluetti_device_state[i].p_f_value,
                                                                      "    ", tmp16); //SOUTLN();
                                      }
                                  }
                              break;
                            case BOOL_FIELD:
                                tmpb = parse_bool_field(data_payload_field);
                                if (pbluetti_device_state[i].p_f_value > NULL)
                                  {
                                    if (*((bool*) pbluetti_device_state[i].p_f_value) == tmpb)
                                      {
                                        S4VAL(" BOOL_FIELD stay idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmpb); //SOUTLN();
                                        // nothing to do
                                      }
                                    else
                                      {
                                        pbluetti_device_state[i].f_new = TRUE;
                                        *((bool*) pbluetti_device_state[i].p_f_value) = tmpb;
                                        S4VAL(" BOOL_FIELD new idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmpb); //SOUTLN();
                                      }
                                  }
                              break;
                            case DECIMAL_FIELD:
                                tmpf = parse_decimal_field(data_payload_field, pbluetti_device_state[i].f_scale);
                                if (pbluetti_device_state[i].p_f_value > NULL)
                                  {
                                    if (*((float*) pbluetti_device_state[i].p_f_value) == tmpf)
                                      {
                                        S4VAL(" DECIMAL_FIELD stay idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmpf); //SOUTLN();
                                        // nothing to do
                                      }
                                     else
                                      {
                                        pbluetti_device_state[i].f_new = TRUE;
                                        *((float*) pbluetti_device_state[i].p_f_value) = tmpf;
                                        S4VAL(" DECIMAL_FIELD new idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmpf); //SOUTLN();
                                      }
                                  }
                              break;
                            case SN_FIELD:
                                sprintf(tmpc18, "%lld", parse_serial_field(data_payload_field));
                                tmps = String(tmpc18);
                                if (pbluetti_device_state[i].p_f_value > NULL)
                                  {
                                    if (*((String*) pbluetti_device_state[i].p_f_value) == tmps)
                                      {
                                        S4VAL(" SN_FIELD stay idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmps); //SOUTLN();
                                        // nothing to do
                                      }
                                    else
                                      {
                                        pbluetti_device_state[i].f_new = TRUE;
                                        *((String*) pbluetti_device_state[i].p_f_value) = tmps;
                                        S4VAL(" SN_FIELD new idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmps); //SOUTLN();
                                      }
                                  }
                                //S2VAL(DEVICE_F_NAMES[pbluetti_device_state[i].f_name], pbluetti_device_state[i].f_name, String(sn));
                                //SVAL(bluetti_device_state[i].f_name, *((String*) bluetti_device_state[i].p_f_value));
                              break;
                            case DATIME_FIELD:
                                //tmp64 = parse_datime_field((data_payload_field));
                                p8 = data_payload_field;
                                //sprintf(tmpc18, "%02d.%02d.%02d %02d:%02d:%02d\n", p8[3], p8[4], p8[5], p8[2], p8[1], p8[0]);
                                sprintf(tmpc18, "%02d.%02d.%02d-%02d:%02d:%02d\n", p8[2], p8[1], p8[0], p8[3], p8[4], p8[5]);
                                tmps = String(tmpc18);
                                SVAL(" DATETIME ",tmps);
                                if (pbluetti_device_state[i].p_f_value > NULL)
                                  {
                                    if (*((String*) pbluetti_device_state[i].p_f_value) == tmps)
                                      {
                                        S4VAL(" DATE_TIME_FIELD stay idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmps); //SOUTLN();
                                        // nothing to do
                                      }
                                    else
                                      {
                                        pbluetti_device_state[i].f_new = TRUE;
                                        //*((String*) pbluetti_device_state[i].p_f_value) = tmps;
                                        S4VAL(" DATE_TIME_FIELD new idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmps); //SOUTLN();
                                      }
                                  }
                                //S2VAL(DEVICE_F_NAMES[pbluetti_device_state[i].f_name], pbluetti_device_state[i].f_name, String(sn));
                                //SVAL(bluetti_device_state[i].f_name, *((String*) bluetti_device_state[i].p_f_value));
                              break;
                            case VERSION_FIELD:
                                tmpf = parse_version_field(data_payload_field);
                                if (pbluetti_device_state[i].p_f_value > NULL)
                                  {
                                    if (*((float*) pbluetti_device_state[i].p_f_value) == tmpf)
                                      {
                                        S4VAL(" VERSION_FIELD stay idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmpf); //SOUTLN();
                                        // nothing to do
                                      }
                                    else
                                      {
                                        pbluetti_device_state[i].f_new = TRUE;
                                        *((float*) pbluetti_device_state[i].p_f_value) = tmpf;
                                        S4VAL(" VERSION_FIELD new idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmpf); //SOUTLN();
                                      }
                                  }
                              break;
                            case STRING_FIELD:
                                tmps = parse_string_field(data_payload_field);
                                if (pbluetti_device_state[i].p_f_value > NULL)
                                  {
                                    if (*((String*) pbluetti_device_state[i].p_f_value) == tmps)
                                      {
                                        S4VAL(" STRING_FIELD stay idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmps.c_str()); //SOUTLN();
                                        // nothing to do
                                      }
                                    else
                                      {
                                        pbluetti_device_state[i].f_new = TRUE;
                                        *((String*) pbluetti_device_state[i].p_f_value) = tmps;
                                        S4VAL(" STRING_FIELD new idx ", i, DEVICE_F_NAMES[pbluetti_device_state[i].f_name], "    ", tmps.c_str()); //SOUTLN();
                                      }
                                  }
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
              }
            else
              {
                //SOUT(" scan mode "); SOUTHEX(length); SOUT("  ");
                //for(int i=0; i< sizeof(bluetti_device_state)/sizeof(device_field_data_t); i++)
                length -= 0x105;  // protokoll overhead + 'scan' marker
                //S3VAL(" found 0x03 page offs length ", page, offset, length); SOUTLN();
                        //tmps = " check  "; tmps.concat(DEVICE_F_NAMES[pbluetti_device_state[i].f_name]);
                        //S4VAL(tmps.c_str(), " f_age f_offs f_size ", pbluetti_device_state[i].f_page, pbluetti_device_state[i].f_offset, pbluetti_device_state[i].f_size);

                //SOUT(" length "); SOUTHEX(length); SOUT("    ");
                uint8_t data_field[8];
                //SOUT(" payload ok "); SOUT("    ");

                int p_index = 0;
                for (int p=3; p <= 4; p++)
                  {
                    data_field[p_index] = pData[p-1];
                    p_index++;
                  }
                //SOUT(" payload ok "); SOUT("    ");
                        //STXT("Copy data:");
                        //serHEXdump(data_payload_field,sizeof(data_payload_field));
                        // S3HEXVAL(" parser device_state idx p_f_value[idx] 6 p_f_... ", i, (uint32_t) pbluetti_device_state[i].p_f_value,
                        //                                                                   (uint32_t) &pbluetti_device_state[i].p_f_value);
                // case UINT_FIELD:
                  tmp16 = parse_uint_field(data_field);
                SOUT(tmp16); SOUT("  ");
                // case DECIMAL_FIELD:
                  tmpf  = parse_decimal_field(data_field, 0);
                SOUT(tmpf); SOUT("  ");
                // case STRING_FIELD:
                  //tmps  = parse_string_field(data_field);
                  //sprintf(debMsg,"%2s\n", tmp16, tmp16, tmpf, data_field);
                  SOUTLN(String((char*) data_field, 2));
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
