#ifndef LIB_STM32F4XX_DMA_H
#define LIB_STM32F4XX_DMA_H

namespace stm32f4xx {

struct Dma {
  struct Stream {
    enum class BurstSize {
      single = 0b00,
      incr_4_beat = 0b01,
      incr_8_beat = 0b10,
      incr_16_beat = 0b11,
    };

    enum class TransferSize {
      byte = 0b00,
      half_word = 0b01,
      word = 0b10,
    };

    #define BFF_DEFINITION_FILE lib/stm32f4xx/dma_stream.reg
    #include <biffield/generate.h>
    #undef BFF_DEFINITION_FILE
  };

  #define BFF_DEFINITION_FILE lib/stm32f4xx/dma.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE

  Stream stream0;
  Stream stream1;
  Stream stream2;
  Stream stream3;
  Stream stream4;
  Stream stream5;
  Stream stream6;
  Stream stream7;
};

extern Dma dma1;
extern Dma dma2;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_DMA_H
