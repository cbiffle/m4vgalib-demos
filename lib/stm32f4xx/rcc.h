#ifndef LIB_STM32F4XX_RCC_H
#define LIB_STM32F4XX_RCC_H

#include "lib/stm32f4xx/types.h"
#include "lib/stm32f4xx/ahb.h"
#include "lib/stm32f4xx/apb.h"

namespace stm32f4xx {

struct ClockConfig;

struct Rcc {
  enum class pprex_t : unsigned {
    div1 = 0b000,
    div2 = 0b100,
    div4 = 0b101,
    div8 = 0b110,
    div16 = 0b111,
  };

  #define BFF_DEFINITION_FILE lib/stm32f4xx/rcc.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE

  void enter_reset(AhbPeripheral);
  void enter_reset(ApbPeripheral);

  void leave_reset(AhbPeripheral);
  void leave_reset(ApbPeripheral);

  void enable_clock(AhbPeripheral);
  void enable_clock(ApbPeripheral);

  void disable_clock(AhbPeripheral);
  void disable_clock(ApbPeripheral);

  void configure_clocks(ClockConfig const &);

  float get_cpu_clock_hz() const;
  float get_ahb_clock_hz() const;
  float get_apb1_clock_hz() const;
  float get_apb2_clock_hz() const;
  float get_pll48_clock_hz() const;

  float get_clock_hz(ApbPeripheral) const;
};

extern Rcc rcc;

struct ClockSpeeds {
  float cpu;
  float ahb;
  float apb1;
  float apb2;
  float pll48;
};

struct ClockConfig {
  float crystal_hz;
  unsigned crystal_divisor;
  unsigned vco_multiplier;
  unsigned general_divisor;
  unsigned pll48_divisor;

  unsigned ahb_divisor;
  unsigned apb1_divisor;
  unsigned apb2_divisor;

  unsigned flash_latency;

  void compute_speeds(ClockSpeeds *) const;
};

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_RCC_H
