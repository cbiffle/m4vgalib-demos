#ifndef LIB_STM32F4XX_APB_H
#define LIB_STM32F4XX_APB_H

namespace stm32f4xx {

enum class ApbPeripheral : unsigned short {
  /*
   * APB1 peripherals
   */
  #define STM32F4XX_APB_PERIPH(slot, name) name = (0 << 8) | slot,
  #define STM32F4XX_APB_RESERVED(slot) /* nothing */
  #include "lib/stm32f4xx/apb1_peripherals.def"
  #undef STM32F4XX_APB_RESERVED
  #undef STM32F4XX_APB_PERIPH

  /*
   * APB2 peripherals
   */
  #define STM32F4XX_APB_PERIPH(slot, name) name = (1 << 8) | slot,
  #define STM32F4XX_APB_RESERVED(slot) /* nothing */
  #include "lib/stm32f4xx/apb2_peripherals.def"
  #undef STM32F4XX_APB_RESERVED
  #undef STM32F4XX_APB_PERIPH
};

inline unsigned get_bus_index(ApbPeripheral p) {
  return (static_cast<unsigned>(p) >> 8) & 0xFF;
}

inline unsigned get_slot_index(ApbPeripheral p) {
  return static_cast<unsigned>(p) & 0xFF;
}

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_APB_H
