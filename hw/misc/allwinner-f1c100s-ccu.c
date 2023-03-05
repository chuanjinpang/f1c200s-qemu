/*
 * Allwinner f1c100s Clock Control Unit emulation
 *
 * Copyright (C) 2023 Lu Hui <luhux76@gmail.com>
 * some code from ./allwinner-h3-ccu.c:
 * Copyright (C) 2019 Niek Linnenbank <nieklinnenbank@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/sysbus.h"
#include "migration/vmstate.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/misc/allwinner-f1c100s-ccu.h"

enum {
    REG_PLL_CPU_CTL     = 0x00,
    REG_PLL_AUDIO_CTL   = 0x08,
    REG_PLL_VIDEO_CTL   = 0x10,
    REG_PLL_VE_CTL      = 0x18,
    REG_PLL_DDR_CTL     = 0x20,
    REG_PLL_PERIPH_CTRL = 0x28,
};

static uint64_t allwinner_f1c100s_ccu_read(void *opaque, hwaddr offset,
                                      unsigned size)
{
    const AwF1c100sClockCtlState *s = AW_F1C100S_CCU(opaque);
    (void)s;
    uint32_t val = 0x0;

    switch (offset) {
    case REG_PLL_CPU_CTL:
    case REG_PLL_AUDIO_CTL:
    case REG_PLL_VIDEO_CTL:
    case REG_PLL_VE_CTL:
    case REG_PLL_DDR_CTL:
    case REG_PLL_PERIPH_CTRL:
        val |= (1 << 28); /* always locked */
        return val;
    default:
        return 0x0;
    }
}

static void allwinner_f1c100s_ccu_write(void *opaque, hwaddr offset,
                                   uint64_t val, unsigned size)
{
    AwF1c100sClockCtlState *s = AW_F1C100S_CCU(opaque);
    (void)s;

    switch (offset) {
    default:
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented write offset 0x%04x\n",
                      __func__, (uint32_t)offset);
        break;
    }
}

static const MemoryRegionOps allwinner_f1c100s_ccu_ops = {
    .read = allwinner_f1c100s_ccu_read,
    .write = allwinner_f1c100s_ccu_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
    .impl.min_access_size = 4,
};

static void allwinner_f1c100s_ccu_reset(DeviceState *dev)
{
    AwF1c100sClockCtlState *s = AW_F1C100S_CCU(dev);
    (void)s;
}

static void allwinner_f1c100s_ccu_init(Object *obj)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    AwF1c100sClockCtlState *s = AW_F1C100S_CCU(obj);

    /* Memory mapping */
    memory_region_init_io(&s->iomem, OBJECT(s), &allwinner_f1c100s_ccu_ops, s,
                          TYPE_AW_F1C100S_CCU, 0x400);
    sysbus_init_mmio(sbd, &s->iomem);
}

static const VMStateDescription allwinner_f1c100s_ccu_vmstate = {
    .name = "allwinner-f1c100s-ccu",
    .version_id = 1,
    .minimum_version_id = 1,
};

static void allwinner_f1c100s_ccu_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = allwinner_f1c100s_ccu_reset;
    dc->vmsd = &allwinner_f1c100s_ccu_vmstate;
}

static const TypeInfo allwinner_f1c100s_ccu_info = {
    .name          = TYPE_AW_F1C100S_CCU,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_init = allwinner_f1c100s_ccu_init,
    .instance_size = sizeof(AwF1c100sClockCtlState),
    .class_init    = allwinner_f1c100s_ccu_class_init,
};

static void allwinner_f1c100s_ccu_register(void)
{
    type_register_static(&allwinner_f1c100s_ccu_info);
}

type_init(allwinner_f1c100s_ccu_register)
