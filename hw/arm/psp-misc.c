/*
 * AMD PSP emulation
 *
 * Copyright (C) 2020 Robert Buhren <robert@robertbuhren.de>
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
 *
 * Special PSP device that covers various unknown MMIO regs.
 */
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "sysemu/sysemu.h"
#include "exec/address-spaces.h"
#include "hw/sysbus.h"
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/qdev-properties.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/arm/psp.h"
#include "qemu/log.h"
#include "hw/arm/psp-misc.h"

/* TODO: Migrate misc regs from PSPEmu.
 * TODO: Implement call back to handle more complicated misc devices
 */

/* Look up misc values from the smn memory space
 * @addr The phys address in the smn memory
 * @val The loaded value
 * @return If a value was found
 * TODO
 */
static int psp_misc_load_value(hwaddr addr, uint64_t * val)
{
    switch (addr){

    case 0x5b304:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0xffffffff; break;

    case 0x5bb04:
        /* Read by the on chip bootloader and acted upon. TODO verify */
        *val = 0xffffffff; break;

    case 0x5e000:
        /* The on chip bootloader waits for bit 0 to go 1 */
        *val = 0x1; break;

    case 0x5d0cc:
        /* The off chip bootloader wants bit 5 to be one, otherwise it returns an error
         * dubbed PSPSTATUS_CCX_SEC_BISI_EN_NOT_SET_IN_FUSE_RAM. */
        *val = BIT(5); break;

    case 0x1025034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e113; break;

    case 0x01003034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e112; break;

    case 0x01004034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e112; break;
        /* Read by the on chip bootloader and acted upon. */
    case 0x0102e034:
        *val = 0x1e312; break;

    case 0x01046034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e103; break;

    case 0x01047034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e103; break;

    case 0x01030034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e312; break;

    case 0x01018034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e113; break;

    case 0x0106c034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e113; break;

    case 0x0106d034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e113; break;

    case 0x0106e034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e312; break;

    case 0x01080034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e113; break;

    case 0x01081034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e113; break;

    case 0x01096034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e313; break;

    case 0x01097034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e313; break;

    case 0x010a8034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e313; break;

    case 0x010d8034:
        /* Read by the on chip bootloader and acted upon. */
        *val = 0x1e313; break;

    case 0x5a088:
        /* The on chip bootloader waits for bit 0 to go 1. */
        *val = 0x1; break;

    case 0x3b10034:
        /* Some SMU ready/online bit the off chip bootloader waits for after the firmware was loaded. */
        /* TODO: Move SMU related stuff into separate device */
        *val = 0x1; break;

    case 0x3b10704:
        /* Some SMU ready/online bit the off chip bootloader waits for after the firmware was loaded. */
        /* TODO: Move SMU related stuff into separate device */
        *val = 0x1; break;

    case 0x18080064:
        /* The on chip bootloader waits for bit 9 and 10 to become set. */
        *val = BIT(10) | BIT(9); break;

    case 0x18480064:
        /* The on chip bootloader waits for bit 9 and 10 to become set. */
        *val = BIT(10) | BIT(9); break;

    case 0x5A078:
        *val = 0x10; break;

    case 0x5A870:
        *val = 0x1; break;

    default:
        /* No value found */
        return -1;
    }

    return 0;
}

static void psp_misc_set_identifier(Object *obj, const char *str, Error **errp)
{
    PSPMiscState *s = PSP_MISC(obj);

    s->ident = g_strdup(str);
}

static char *psp_misc_get_identifier(Object *obj, Error **errp)
{
    PSPMiscState *s = PSP_MISC(obj);

    return g_strdup(s->ident);
}

static void psp_misc_write(void *opaque, hwaddr offset, uint64_t value,
                           unsigned int size)
{
    PSPMiscState *misc = PSP_MISC(opaque);
    hwaddr phys_base = misc->iomem.addr;
    hwaddr phys_abs;

    /* TODO: Allow write to specific registers */

    phys_abs = phys_base + offset;

    qemu_log_mask(LOG_UNIMP, "%s: unimplemented device write at: 0x%"
                  HWADDR_PRIx " (size: %d, value: 0x%lx)\n",
                  misc->ident, phys_abs, size, value);
}

static uint64_t psp_misc_read(void *opaque, hwaddr offset, unsigned int size)
{
    PSPMiscState *misc = PSP_MISC(opaque);
    hwaddr phys_abs;
    //int i;
    hwaddr phys_base;
    uint64_t value;

    phys_base = misc->iomem.addr;
    phys_abs = phys_base + offset;
    value = 0;

    if(psp_misc_load_value(phys_abs, &value)) {
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented read at:  0x%"
                      HWADDR_PRIX " (size: %d, value: 0x%lx)\n",
                      misc->ident, phys_abs, size, value);
    } else {
        qemu_log_mask(LOG_TRACE, "%s: [misc] read at: 0x%"
                      HWADDR_PRIX " (size: %d, value: 0x%lx)\n",
                      misc->ident, phys_abs, size, value);
    }

    return value;
}

static const MemoryRegionOps misc_mem_ops = {
    .read = psp_misc_read,
    .write = psp_misc_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl.min_access_size = 1,
    .impl.max_access_size = 4,
};

static void psp_misc_init(Object *obj)
{
    PSPMiscState *s = PSP_MISC(obj);

    s->mmio_size = 0;
    s->ident = NULL;

    /* TODO: What is the difference to e.g. DEFINE_PROP_UINT32 ? */
    object_property_add_uint64_ptr(obj,"psp_misc_msize", &s->mmio_size,
                                   OBJ_PROP_FLAG_READWRITE);

    object_property_add_str(obj,"psp_misc_ident",psp_misc_get_identifier,
                            psp_misc_set_identifier);

}

static void psp_misc_realize(DeviceState *dev, Error **errp)
{
    PSPMiscState *s = PSP_MISC(dev);

    if (s->mmio_size == 0) {
        error_setg(errp, "property 'mmio_size' not specified or zero");
        return;
    }

    if (s->ident == NULL) {
        error_setg(errp, "property 'ident' not specified");
        return;
    }

    memory_region_init_io(&s->iomem, OBJECT(dev), &misc_mem_ops, s,
                          s->ident, s->mmio_size);

    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);

}

static void psp_misc_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);
    dc->realize = psp_misc_realize;
}

static const TypeInfo psp_misc_info = {
    .name = TYPE_PSP_MISC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_init = psp_misc_init,
    .instance_size = sizeof(PSPMiscState),
    .class_init = psp_misc_class_init,

};

static void psp_misc_register_types(void)
{
    type_register_static(&psp_misc_info);
}

type_init(psp_misc_register_types);
