#ifndef _VOLTCRAFT_SEM6000_H_
  #define _VOLTCRAFT_SEM6000_H_
  #include <Arduino.h>

  enum voltcraft_f_types
    {
       UINT_FIELD,
       BOOL_FIELD,
       //ENUM_FIELD,
       STRING_FIELD,
       //DECIMAL_ARRAY_FIELD,
       DECIMAL_FIELD,
       VERSION_FIELD,
       SN_FIELD,
       TYPE_UNDEFINED
    };
  enum voltcraft_f_index
    {
      DEV_NAME,
      DEV_VENDOR,
      DEV_SERNUM,
      HW_Version,
      FW_VERSION,
      DATE_TIME,
      LED_ON,
      LED_RING,
      OVER_POWER,
      TIMER_ON,
      TIMER_TIME,
      ACT_POWER,
      ACT_VOLTAGE,
      ACT_CURRENT,
      DAY_POWER,
      DAY_VOLTAGE,
      DAY_CURRENT,
      MONTH_POWER,
      MONTH_VOLTAGE,
      MONTH_CURRENT,
      YEAR_POWER,
      YEAR_VOLTAGE,
      YEAR_CURRENT,
      FIELD_IDX_MAX
    };
  const char VOLT_DEV_F_NAMES [FIELD_IDX_MAX+1][25] =
    {
      "DEV_NAME",
      "DEV_VENDOR",
      "DEV_SERNUM",
      "HW_Version",
      "FW_VERSION",

      "DATE_TIME",
      "LED_ON",
      "LED_RING",
      "OVER_POWER",
      "TIMER_ON",

      "TIMER_TIME",
      "ACT_POWER",
      "ACT_VOLTAGE",
      "ACT_CURRENT",
      "DAY_POWER",

      "DAY_VOLTAGE",
      "DAY_CURRENT",
      "MONTH_POWER",
      "MONTH_VOLTAGE",
      "MONTH_CURRENT",

      "YEAR_POWER",
      "YEAR_VOLTAGE",
      "YEAR_CURRENT",
      "FIELD_UNDEFINED"
    };
    typedef struct voltcraft_dev_f_data
    {
      enum voltcraft_f_index volt_f_name;
      void*   p_f_value;
      uint8_t f_page;
      uint8_t f_offset;
      int8_t  f_size;
      int8_t  f_scale;
      int8_t  f_enum;
      enum voltcraft_f_types f_type;
    } voltcraft_dev_f_data_t;

#endif