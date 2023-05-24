#ifndef _DEVICE_AC300_H_
  #define _DEVICE_AC300_H_
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
      // INFO device offset sort
        DEVICE_TYPE,
        SERIAL_NUMBER,
        ARM_VERSION,
        DSP_VERSION,
        DC_INPUT_POWER,
        AC_INPUT_POWER,
        AC_OUTPUT_POWER,
        DC_OUTPUT_POWER,
        POWER_GENERATION,
        TOTAL_BATT_PERC,
        AC_OUTPUT_ON,
        DC_OUTPUT_ON,
      // INFO internal
        INFO_INTERNAL = DC_OUTPUT_ON,
        AC_OUTPUT_MODE,
        INTERN_AC_VOLT,
        INTERN_CURR_1,
        INTERN_POWER_1,
        INTERN_AC_FREQ,
        INTERN_CURR_2,
        INTERN_POWER_2,
        AC_INPUT_VOLT,
        INTERN_CURR_3,
        INTERN_POWER_3,
        AC_INPUT_FREQ,
        INT_DC_INP_VOLT,
        INT_DC_INP_POW,
        INT_DC_INP_CURR,
        PACK_NUM_MAX,
        TOTAL_BATT_VOLT,
        TOTAL_BATT_CURR,
        PACK_NUM,
        PACK_STATUS,
        PACK_VOLTAGE,
        PACK_BATT_PERC,
        //CELL_VOTAGES,
        PACK_BMS_VERSION,
      // CONTROL elements
        CTRL_ELEMENTS = PACK_BMS_VERSION,
        UPS_MODE,
        SPLIT_PHASE_ON,
        SPLIT_PH_MACH_MODE,
        SET_PACK_NUM,
        SET_AC_OUT_ON,
        SET_DC_OUT_ON,
        GRID_CHANGE_ON,
        TIME_CTRL_ON,
        BATT_RANGE_START,
        BATT_RANGE_END,
        BLUETOOTH_CONN,
        AUTO_SLEEP_MODE,
        LED_CONTROL,
        FIELD_UNDEFINED,
        FIELD_IDX_MAX
    };
  const char DEVICE_F_NAMES [FIELD_IDX_MAX][40] =
    {
      // INFO device offset sort
        "DEVICE_TYPE",
        "SERIAL_NUMBER",
        "ARM_VERSION",
        "DSP_VERSION",
        "DC_INPUT_POWER",
        "AC_INPUT_POWER",
        "AC_OUTPUT_POWER",
        "DC_OUTPUT_POWER",
        "POWER_GENERATION",
        "TOTAL_BATT_PERC",
        "AC_OUTPUT_ON",
        "DC_OUTPUT_ON",
      // INFO internal
        "AC_OUTPUT_MODE",
        "INTERN_AC_VOLT",
        "INTERN_CURR_1",
        "INTERN_POWER_1",
        "INTERN_AC_FREQ",
        "INTERN_CURR_2",
        "INTERN_POWER_2",
        "AC_INPUT_VOLT",
        "INTERN_CURR_3",
        "INTERN_POWER_3",
        "AC_INPUT_FREQ",
        "INT_DC_INP_VOLT",
        "INT_DC_INP_POW",
        "INT_DC_INP_CURR",
        "PACK_NUM_MAX",
        "TOTAL_BATT_VOLT",
        "TOTAL_BATT_CURR",
        "PACK_NUM"
        "PACK_STATUS"
        "PACK_VOLTAGE",
        "PACK_BATT_PERC",
        //"CELL_VOLTAGES",
        "PACK_BMS_VERSION",
      // CONTROL elements
        "UPS_MODE",
        "SPLIT_PHASE_ON",
        "SPLIT_PH_MACH_MODE",
        "SET_PACK_NUM",
        "SET_AC_OUT_ON",
        "SET_DC_OUT_ON",
        "GRID_CHANGE_ON",
        "TIME_CTRL_ON",
        "BATT_RANGE_START",
        "BATT_RANGE_END",
        "BLUETOOTH_CONN",
        "AUTO_SLEEP_MODE",
        "LED_CONTROL",
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
#endif
