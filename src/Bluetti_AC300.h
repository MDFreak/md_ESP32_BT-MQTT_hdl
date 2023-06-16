#ifndef _BLUETTI_AC300_H_
  #define _BLUETTI_AC300_H_
  #include <Arduino.h>

  /* Not implemented yet
    enum output_mode
      {
          STOP = 0,
          INVERTER_OUTPUT = 1,
          BYPASS_OUTPUT_C = 2,
          BYPASS_OUTPUT_D = 3,
          LOAD_MATCHING = 4
      };

    enum ups_mode
      {
          CUSTOMIZED = 1,
          PV_PRIORITY = 2,
          STANDARD = 3,
          TIME_CONTROl = 4
      };

    enum auto_sleep_mode
      {
        THIRTY_SECONDS = 2,
        ONE_MINNUTE = 3,
        FIVE_MINUTES = 4,
        NEVER = 5
      };
   */

  enum field_types
    {
       UINT_FIELD,
       BOOL_FIELD,
       ENUM_FIELD,
       STRING_FIELD,
       DECIMAL_ARRAY_FIELD,
       DECIMAL_FIELD,
       VERSION_FIELD,
       SN_FIELD,
       TYPE_UNDEFINED
    };
  enum field_index
    {
      DC_OUTPUT_ON,
      AC_OUTPUT_ON,
      DC_OUTPUT_POWER,
      AC_OUTPUT_POWER,
      POWER_GENERATION,
      TOTAL_BATTERY_PERCENT,
      DC_INPUT_POWER,
      AC_INPUT_POWER,
      PACK_VOLTAGE,
      SERIAL_NUMBER,
      ARM_VERSION,
      DSP_VERSION,
      DEVICE_TYPE,
      INTERNAL_AC_VOLTAGE,
      INTERNAL_CURRENT_ONE,
      PACK_NUM_MAX,
      UPS_MODE,
      AUTO_SLEEP_MODE,
      GRID_CHANGE_ON,
      FIELD_UNDEFINED,
      FIELD_IDX_MAX
    };
  const char DEVICE_F_NAMES [FIELD_IDX_MAX][25] =
    {
      "AC_OUTPUT_ON",
      "DC_OUTPUT_ON",
      "DC_OUTPUT_POWER",
      "AC_OUTPUT_POWER",
      "POWER_GENERATION",
      "TOTAL_BATTERY_PERCENT",
      "DC_INPUT_POWER",
      "AC_INPUT_POWER",
      "PACK_VOLTAGE",
      "SERIAL_NUMBER",
      "ARM_VERSION",
      "DSP_VERSION",
      "DEVICE_TYPE",
      "INTERNAL_AC_VOLTAGE",
      "INTERNAL_CURRENT_ONE",
      "PACK_NUM_MAX",
      "UPS_MODE",
      "AUTO_SLEEP_MODE",
      "GRID_CHANGE_ON",
      "FIELD_UNDEFINED"
    };
  typedef struct device_field_data
    {
      enum field_index f_name;
      void*   p_f_value;
      uint8_t f_page;
      uint8_t f_offset;
      int8_t  f_size;
      int8_t  f_scale;
      int8_t  f_enum;
      enum field_types f_type;
    } device_field_data_t;


  // { FIELD_NAME, PAGE, OFFSET, SIZE, SCALE (if scale is needed e.g. decimal value, defaults to 0) , ENUM (if data is enum, defaults to 0) , FIELD_TYPE }
  //extern device_field_data_t bluetti_device_state[];
  //extern device_field_data_t bluetti_device_command[];
  //extern device_field_data_t bluetti_polling_command[];
#endif // _BLUETTI_AC300_H_
