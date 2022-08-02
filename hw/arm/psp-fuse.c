/*
 * AMD PSP Fuse emulation
 *
 * Copyright (C) 2022 Pascal Harprecht <p.harprecht@gmail.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qapi/error.h"
#include "hw/sysbus.h"
#include "hw/arm/psp-fuse.h"

static uint64_t psp_fuse_read(void *opaque, hwaddr offset, unsigned int size) {
    // TODO
    uint64_t r = 0;
    switch(offset) {
    case 0x0:
        r = 0x1a060900;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: bad offset 0x%" HWADDR_PRIx "\n",
                      __func__, offset);
        break;
    }
    return r;
}

/* No know write operations at this point */
static const MemoryRegionOps timer_mem_ops = {
    .read = psp_fuse_read,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl.min_access_size = 1,
    .impl.max_access_size = 4,
};

static void psp_fuse_realize(DeviceState *dev, Error **errp)
{
    PSPFuseState *s = PSP_FUSE(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &timer_mem_ops, s, TYPE_PSP_FUSE, PSP_FUSE_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);
}

static void psp_fuse_class_init(ObjectClass *klass, void * data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = psp_fuse_realize;
}

static const TypeInfo psp_fuse_info = {
    .name = TYPE_PSP_FUSE,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size =  sizeof(PSPFuseState),
    .class_init = psp_fuse_class_init
};

static void psp_fuse_register_types(void) {
    type_register_static(&psp_fuse_info);
}

type_init(psp_fuse_register_types);
