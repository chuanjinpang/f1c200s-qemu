#ifndef HW_ARM_ALLWINNER_F1C100S_H
#define HW_ARM_ALLWINNER_F1C100S_H

#include "hw/intc/allwinner-f1c100s-intc.h"
#include "hw/misc/allwinner-f1c100s-ccu.h"
#include "hw/misc/allwinner-sid.h"
#include "hw/sd/allwinner-sdhost.h"
#include "hw/timer/allwinner-a10-pit.h"
#include "hw/ssi/allwinner-sun6i-spi.h"
#include "hw/i2c/allwinner-i2c.h"

#include "target/arm/cpu.h"
#include "qom/object.h"

enum {
    AW_F1C100S_DEV_SRAM_A1,
    AW_F1C100S_DEV_SPI0,
    AW_F1C100S_DEV_SPI1,
    AW_F1C100S_DEV_CCU,
    AW_F1C100S_DEV_INTC,
    AW_F1C100S_DEV_TIMER,
    AW_F1C100S_DEV_UART0,
    AW_F1C100S_DEV_UART1,
    AW_F1C100S_DEV_UART2,
    AW_F1C100S_DEV_SID,
    AW_F1C100S_DEV_MMC0,
    AW_F1C100S_DEV_MMC1,
    AW_F1C100S_DEV_TWI0,
    AW_F1C100S_DEV_TWI1,
    AW_F1C100S_DEV_TWI2,
    AW_F1C100S_DEV_LOG_BUF,
    AW_F1C100S_DEV_SDRAM,
    AW_F1C100S_DEV_BOOTROM,
};

#define TYPE_AW_F1C100S "allwinner-f1c100s"
OBJECT_DECLARE_SIMPLE_TYPE(AwF1C100SState, AW_F1C100S)

struct AwF1C100SState {
    /*< private >*/
    DeviceState parent_obj;
    /*< public >*/
    const hwaddr *memmap;

    ARMCPU cpu;
    AwF1c100sClockCtlState ccu;
    AwF1c100sIntcState intc;
    AwA10PITState timer;
    AwSun6iSpiState spi[2];
    AWI2CState i2c[3];
    AwSidState sid;
    AwSdHostState mmc[2];
    MemoryRegion sram_a1;
    MemoryRegion sram_logbuf;
    MemoryRegion bootrom;
};

#endif
