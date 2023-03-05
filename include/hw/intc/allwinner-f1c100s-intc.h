#ifndef ALLWINNER_F1C100S_INTC_H
#define ALLWINNER_F1C100S_INTC_H

#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_AW_F1C100S_INTC  "allwinner-f1c100s-intc"
OBJECT_DECLARE_SIMPLE_TYPE(AwF1c100sIntcState, AW_F1C100S_INTC)

struct AwF1c100sIntcState {
    /*< private >*/
    SysBusDevice parent_obj;
    /*< public >*/
    MemoryRegion iomem;
    qemu_irq parent_fiq;
    qemu_irq parent_irq;

    uint32_t vector;
    uint32_t base_addr;
    uint32_t nmi_ctl;
    uint32_t pending[2];
    uint32_t enable[2];
    uint32_t mask[2];
};

#endif
