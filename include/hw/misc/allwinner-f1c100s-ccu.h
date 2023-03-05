/*
 * Allwinner f1c100s Clock Control Unit emulation
 *
 * Copyright (C) 2023 Lu Hui <luhux76@gmail.com>
 * some code copy from: ./allwinner-h3-ccu.h
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

#ifndef HW_MISC_ALLWINNER_F1C100S_CCU_H
#define HW_MISC_ALLWINNER_F1C100S_CCU_H

#include "qom/object.h"
#include "hw/sysbus.h"

#define TYPE_AW_F1C100S_CCU    "allwinner-f1c100s-ccu"
OBJECT_DECLARE_SIMPLE_TYPE(AwF1c100sClockCtlState, AW_F1C100S_CCU)

struct AwF1c100sClockCtlState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
};

#endif /* HW_MISC_ALLWINNER_F1C100S_CCU_H */
