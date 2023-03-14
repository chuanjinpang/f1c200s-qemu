/*
 * allwinner sun6i spi control emulation.
 *
 * Copyright (C) 2023 Lu Hui <luhux76@gmail.com>
 *
 * some code from ./pl022.c:
 * Copyright (c) 2007 CodeSourcery.
 * Written by Paul Brook
 * some code from ./sifive_spi.c
 * Copyright (c) 2021 Wind River Systems, Inc.
 * Bin Meng <bin.meng@windriver.com>
 * some code from mainline uboot:
 * drivers/spi/spi-sunxi.c
 *
 * This code is licensed under the GPL.
 */

#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/sysbus.h"
#include "migration/vmstate.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-properties-system.h"
#include "hw/irq.h"
#include "hw/ssi/allwinner-sun6i-spi.h"
#include "hw/ssi/ssi.h"
#include "qemu/log.h"
#include "qemu/module.h"

/* #define DEBUG_AW_SUN6I_SPI 1 */

#ifdef DEBUG_AW_SUN6I_SPI
#define DPRINTF(fmt, ...) \
do { printf("aw_sun6i_spi: " fmt , ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) do {} while (0)
#endif

/* sun6i spi registers */
#define SUN6I_GLB_CTL_REG               0x04
#define SUN6I_TFR_CTL_REG               0x08
#define SUN6I_INT_CTL_REG               0x10
#define SUN6I_FIFO_CTL_REG              0x18
#define SUN6I_FIFO_STA_REG              0x1c
#define SUN6I_CLK_CTL_REG               0x24
#define SUN6I_BURST_CNT_REG             0x30
#define SUN6I_SEND_CNT_REG              0x34
#define SUN6I_BURST_CTL_REG             0x38
#define SUN6I_TXDATA_REG                0x200
#define SUN6I_RXDATA_REG                0x300

static void aw_sun6i_spi_update_ss(AwSun6iSpiState *s, int ss_active,
                                   bool ss_level)
{
    if (ss_active == s->ss_active) {
        if (s->ss_level == ss_level) {
            return;
        }
    }
    int i;
    /* deassert all */
    for (i = 0; i < 4; i++) {
        qemu_irq_raise(s->ss_lines[i]);
    }
    s->ss_level = ss_level;
    s->ss_active = ss_active;
    if (s->ss_active >= 0 && s->ss_active < 4) {
        if (s->ss_level) {
            qemu_irq_raise(s->ss_lines[s->ss_active]);
            DPRINTF("%s: deassert ss %d\n", __func__, s->ss_active);
        } else {
            qemu_irq_lower(s->ss_lines[s->ss_active]);
            DPRINTF("%s: assert ss %d\n", __func__, s->ss_active);
        }
    }
}

static void aw_sun6i_spi_reset_tx_fifo(AwSun6iSpiState *s)
{
    DPRINTF("%s: tx fifo reset\n", __func__);
    fifo8_reset(&s->tx_fifo);
}

static void aw_sun6i_spi_reset_rx_fifo(AwSun6iSpiState *s)
{
    DPRINTF("%s: rx fifo reset\n", __func__);
    fifo8_reset(&s->rx_fifo);
}

static void aw_sun6i_spi_xfer(AwSun6iSpiState *s)
{
    if (s->start_burst == 0) {
        return;
    }
    DPRINTF("%s: spi xfer start\n", __func__);
    int i;
    uint8_t rx = 0x00;
    uint8_t tx = 0x00;
    /* 
     * act a 'smart spi controller'
     * because mainline uboot:
     * arch/arm/mach-sunxi/spl_spi_sunxi.c
     * doesnt do any assert or deassert cs line
     * so, spi controller need do it.
     */
    if (s->send_bytes < s->burst_bytes) {
        aw_sun6i_spi_update_ss(s, s->ss_active, 0);
    }
    for (i = 0; i < s->burst_bytes; i++) {
        if (i >= s->send_bytes) {
            DPRINTF("%s: fill dummy byte to tx fifo\n", __func__);
            tx = 0xFF;
        } else if (!fifo8_is_empty(&s->tx_fifo)) {
            tx = fifo8_pop(&s->tx_fifo);
        }
        DPRINTF("%s: tx [%d] %02x\n", __func__, i, tx);
        rx = ssi_transfer(s->spi, tx);
        if (!fifo8_is_full(&s->rx_fifo)) {
            fifo8_push(&s->rx_fifo, rx);
            DPRINTF("%s: rx [%d] %02x \n", __func__, i, rx);
        }
    }
    if (s->send_bytes < s->burst_bytes) {
        aw_sun6i_spi_update_ss(s, s->ss_active, 1);
    }
    DPRINTF("%s: spi xfer end\n", __func__);
    s->start_burst = 0;
}

static uint64_t aw_sun6i_spi_read(void *opaque, hwaddr offset,
                           unsigned size)
{
    AwSun6iSpiState *s = (AwSun6iSpiState *)opaque;
    uint32_t val = 0x0;

    unsigned sz;
    switch (offset) {
    case SUN6I_GLB_CTL_REG:
        return 0x0;
    case SUN6I_TFR_CTL_REG:
        if (s->ss_active >= 0) {
            val |= s->ss_active << 4;
        }
        val |= s->ss_level << 7;
        val |= s->start_burst << 31;
        return val;
    case SUN6I_FIFO_CTL_REG:
        return 0x0;
    case SUN6I_BURST_CNT_REG:
        val |= s->burst_bytes;
        return val;
    case SUN6I_SEND_CNT_REG:
        val |= s->send_bytes;
        return val;
    case SUN6I_BURST_CTL_REG:
        val |= s->send_bytes;
        return val;
    case SUN6I_FIFO_STA_REG:
        val |= extract32(fifo8_num_used(&s->rx_fifo), 0, 7) << 0;
        val |= extract32(fifo8_num_used(&s->tx_fifo), 0, 7) << 16;
        return val;
    case SUN6I_RXDATA_REG:
        sz = 0;
        for (sz = 0; sz < size; sz++) {
            if (!fifo8_is_empty(&s->rx_fifo)) {
                val |= fifo8_pop(&s->rx_fifo) << (sz * 8);
            }
        }
        DPRINTF("%s: read rxfifo, data: %08x\n", __func__, val);
        return val;
    default:
        return 0x0;
    }
}

static void aw_sun6i_spi_do_reset(void *opaque) {
    AwSun6iSpiState *s = (AwSun6iSpiState *)opaque;

    DPRINTF("%s: reset spi controller\n", __func__);
    s->ss_active = 0;
    s->ss_level = 1;
    aw_sun6i_spi_update_ss(s, 0, 1);
    s->start_burst = 0;
    s->burst_bytes = 0;
    s->send_bytes = 0;
    aw_sun6i_spi_reset_rx_fifo(s);
    aw_sun6i_spi_reset_tx_fifo(s);
}

static void aw_sun6i_spi_write(void *opaque, hwaddr offset,
                        uint64_t value, unsigned size)
{
    AwSun6iSpiState *s = (AwSun6iSpiState *)opaque;

    int ss_active;
    bool ss_level;
    switch (offset) {
    case SUN6I_GLB_CTL_REG:
        if (test_bit(31, (void *)&value)) {
            aw_sun6i_spi_do_reset(s);
        }
        break;
    case SUN6I_TFR_CTL_REG:
        ss_active = extract32(value, 4, 2);
        ss_level = test_bit(7, (void *)&value);
        aw_sun6i_spi_update_ss(s, ss_active, ss_level);
        s->start_burst = test_bit(31, (void *)&value);
        aw_sun6i_spi_xfer(s);
        break;
    case SUN6I_FIFO_CTL_REG:
        if (test_bit(32, (void *)&value)) {
            aw_sun6i_spi_reset_tx_fifo(s);
        }
        if (test_bit(15, (void *)&value)) {
            aw_sun6i_spi_reset_rx_fifo(s);
        }
        break;
    case SUN6I_BURST_CNT_REG:
        s->burst_bytes = value;
        break;
    case SUN6I_SEND_CNT_REG:
        s->send_bytes = value;
        break;
    case SUN6I_BURST_CTL_REG:
        s->send_bytes = value;
        break;
    case SUN6I_TXDATA_REG:
        if (!fifo8_is_full(&s->tx_fifo)) {
            fifo8_push(&s->tx_fifo, (uint8_t)value);
            DPRINTF("%s: write txfifo, data: %02x\n", __func__, (uint8_t)value);
        }
        break;
    default:
        break;
    }
}

static void aw_sun6i_spi_reset(DeviceState *dev)
{
    AwSun6iSpiState *s = AW_SUN6I_SPI(dev);

    aw_sun6i_spi_do_reset(s);
}

static const MemoryRegionOps aw_sun6i_spi_ops = {
    .read = aw_sun6i_spi_read,
    .write = aw_sun6i_spi_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 4,
    },
    .impl.min_access_size = 1,
};

static Property aw_sun6i_spi_properties[] = {
    DEFINE_PROP_UINT8("fifo_depth", AwSun6iSpiState, fifo_depth, 64),
    DEFINE_PROP_END_OF_LIST()
};

static const VMStateDescription vmstate_aw_sun6i_spi = {
    .name = "aw_sun6i_spi",
    .version_id = 1,
    .minimum_version_id = 1,
};

static void aw_sun6i_spi_realize(DeviceState *dev, Error **errp)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    AwSun6iSpiState *s = AW_SUN6I_SPI(dev);

    memory_region_init_io(&s->iomem, OBJECT(s), &aw_sun6i_spi_ops, s,
                          "aw_sun6i_spi", 4 * KiB);
    int i;
    for (i = 0; i < 4; i++) {
        sysbus_init_irq(sbd, &s->ss_lines[i]);
    }
    sysbus_init_mmio(sbd, &s->iomem);
    s->spi = ssi_create_bus(dev, "ssi");
    fifo8_create(&s->tx_fifo, s->fifo_depth);
    fifo8_create(&s->rx_fifo, s->fifo_depth);
}

static void aw_sun6i_spi_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = aw_sun6i_spi_reset;
    dc->vmsd = &vmstate_aw_sun6i_spi;
    dc->realize = aw_sun6i_spi_realize;
    device_class_set_props(dc, aw_sun6i_spi_properties);
}

static const TypeInfo aw_sun6i_spi_info = {
    .name          = TYPE_AW_SUN6I_SPI,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(AwSun6iSpiState),
    .class_init    = aw_sun6i_spi_class_init,
};

static void aw_sun6i_spi_register_types(void)
{
    type_register_static(&aw_sun6i_spi_info);
}

type_init(aw_sun6i_spi_register_types)
