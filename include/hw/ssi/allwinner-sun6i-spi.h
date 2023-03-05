/*
 * Allwinner sun6i spi control emulation.
 */

#ifndef HW_SSI_AW_SUN6I_SPI_H
#define HW_SSI_AW_SUN6I_SPI_H

#include "qemu/fifo8.h"
#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_AW_SUN6I_SPI "aw-f1c100s-spi"
OBJECT_DECLARE_SIMPLE_TYPE(AwSun6iSpiState, AW_SUN6I_SPI)

struct AwSun6iSpiState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    SSIBus *spi;

    int8_t ss_active;
    bool ss_level;
    qemu_irq ss_lines[4];

    bool start_burst;
    uint8_t burst_bytes;
    uint8_t send_bytes;

    bool irq_enable;
    bool trans_complete;
    bool txfifo_overflow;
    bool rxfifo_overflow;
    bool txfifo_empty;
    bool rxfifo_ready;

    Fifo8 tx_fifo;
    Fifo8 rx_fifo;
    uint8_t fifo_depth;
};

#endif
