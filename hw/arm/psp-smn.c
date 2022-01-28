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
 */
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "sysemu/sysemu.h"
#include "exec/address-spaces.h"
#include "hw/sysbus.h"
#include "qemu-common.h"
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/qdev-properties.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/arm/psp.h"
#include "qemu/log.h"
#include "hw/arm/psp-smn.h"

/* DEFINE_PROP_STRING("rom_path", AmdPspState,flash_rom_path), */

/* PSPSmnCTRL: Controller, attaches SMN devices on demand? */

const char* ident = "SMN Control";

static PSPMiscReg psp_regs[] = {
    {
        /* The on chip bootloader waits for bit 0 to go 1 */
        .addr = 0x5e000,
        .val = 0x1,
    },
    {
        /* The off chip bootloader wants bit 5 to be one, otherwise it returns an error
         * dubbed PSPSTATUS_CCX_SEC_BISI_EN_NOT_SET_IN_FUSE_RAM. */
        .addr = 0x5d0cc,
        .val = BIT(5),

    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x1025034,
        .val = 0x1e113,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x01003034,
        .val = 0x1e112,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x01004034,
        .val = 0x1e112,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x0102e034,
        .val = 0x1e312,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x01046034,
        .val = 0x1e103,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x01047034,
        .val = 0x1e103,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x01030034,
        .val = 0x1e312,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x01018034,
        .val = 0x1e113,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x0106c034,
        .val = 0x1e113,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x0106d034,
        .val = 0x1e113,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x0106e034,
        .val = 0x1e313,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x01080034,
        .val = 0x1e113,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x01081034,
        .val = 0x1e113,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x01096034,
        .val = 0x1e313,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x01097034,
        .val = 0x1e313,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x010a8034,
        .val = 0x1e313,
    },
    {
        /* Read by the on chip bootloader and acted upon. */
        .addr = 0x010d8034,
        .val = 0x1e313,
    },
    {
        /* The on chip bootloader waits for bit 0 to go 1. */
        .addr = 0x5a088,
        .val = 0x1,
    },
    {
        /* Some SMU ready/online bit the off chip bootloader waits for after the firmware was loaded. */
        /* TODO: Move SMU related stuff into separate device */
        .addr = 0x3b10034,
        .val = 0x1,

    },
    {
        /* Some SMU ready/online bit the off chip bootloader waits for after the firmware was loaded. */
        /* TODO: Move SMU related stuff into separate device */
        .addr = 0x3b10704,
        .val = 0x1,

    },
    {
        /* The on chip bootloader waits for bit 9 and 10 to become set. */
        .addr = 0x18080064,
        .val = BIT(10) | BIT(9),
    },
    {
        /* The on chip bootloader waits for bit 9 and 10 to become set. */
        .addr = 0x18480064,
        .val = BIT(10) | BIT(9),
    },
};

static void psp_smn_update_slot(PSPSmnState *smn, uint32_t idx) {
    char name[PSP_SMN_SLOT_NAME_LEN] = { 0 };

    PSPSmnAddr addr = smn->psp_smn_slots[idx];

    snprintf(name, PSP_SMN_SLOT_NAME_LEN, "%s%d", PSP_SMN_SLOT_NAME, idx);

    /* TODO documentation */
    memory_region_set_alias_offset(&smn->psp_smn_containers[idx], addr);

    /* TODO fix log entries */
    qemu_log_mask(LOG_UNIMP, "%s: SMN mapped 0x%x to SMN slot %d\n",ident, addr,
                  idx);

}

static void psp_smn_write(void *opaque, hwaddr offset, uint64_t value,
                          unsigned int size) {
    PSPSmnState *smn = PSP_SMN(opaque);
    uint32_t idx;
    hwaddr phys_base = smn->psp_smn_control.addr;

    qemu_log_mask(LOG_UNIMP, "%s: SMN write at 0x%" HWADDR_PRIx
        " (size %d, raw val 0x%lx)\n",
        ident, phys_base + offset, size, value);
    switch (size) {
        case 4:
            idx = (offset / 4) * 2;
            smn->psp_smn_slots[idx] = ((uint32_t) value & 0xffff) << 20;
            smn->psp_smn_slots[idx + 1] = ((uint32_t) value >> 16) << 20;
            psp_smn_update_slot(smn, idx);
            psp_smn_update_slot(smn, idx + 1);
            break;
        case 2:
            idx = offset / 2;
            smn->psp_smn_slots[idx] = (uint32_t) value << 20;
            psp_smn_update_slot(smn, idx);
            break;
        default:
            /* TODO Fix line breaks */
            qemu_log_mask(LOG_UNIMP,
                          "%s: SMN ERROR. Unsupported write access size: %d\n",
                          ident, size);
            break;
    }
}

static uint64_t psp_smn_read(void *opaque, hwaddr offset, unsigned int size) {

    PSPSmnState *smn = PSP_SMN(opaque);
    uint32_t idx;
    uint32_t val = 0;
    hwaddr phys_base = smn->psp_smn_control.addr;

    switch (size) {
        case 4:
            /* Each 4 byte access programs two slots.
             * SMN regions are 1M large, thus each SMN address is 1M aligned.
             */
            idx = (offset / 4) * 2;
            val = ((smn->psp_smn_slots[idx + 1] >> 20) << 16) |
                  (smn->psp_smn_slots[idx] >> 20);
            break;
        default:
            qemu_log_mask(LOG_UNIMP,
                          "%s: SMN ERROR. Unsupported read access size: %d\n",
                          ident, size);
            break;

    }
    qemu_log_mask(LOG_UNIMP, "%s: SMN read at 0x%" HWADDR_PRIx
        " (size %d, raw val 0x%x)\n",
        ident, phys_base + offset, size, val);
    return val;
}

static const MemoryRegionOps smn_ctlr_ops = {
    .read = psp_smn_read,
    .write = psp_smn_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl.min_access_size = 1,
    .impl.max_access_size = 4,
};

static void psp_smn_init(Object *obj)
{
    PSPSmnState *s = PSP_SMN(obj);
    /* SysBusDevice *sbd = SYS_BUS_DEVICE(obj); */


    /* Base address of the SMN containers in the PSP address space */
    object_property_add_uint64_ptr(obj,"smn-container-base", &s->psp_smn_base,
                                   OBJ_PROP_FLAG_READWRITE);

    object_initialize_child(obj, "smn_misc", &s->psp_smn_misc,
                            TYPE_PSP_MISC);

    object_initialize_child(obj, "smn_flash", &s->psp_smn_flash,
                            TYPE_PSP_SMN_FLASH);

    object_property_set_uint(OBJECT(&s->psp_smn_misc),"psp_misc_msize",
                             0xFFFFFFFF, &error_abort);

    object_property_set_str(OBJECT(&s->psp_smn_misc),"psp_misc_ident",
                            "SMN MEM", &error_abort);

    object_property_set_bool(OBJECT(&s->psp_smn_misc), "realized", true,
                             &error_abort);

}

static void psp_smn_init_slots(DeviceState *dev) {
    PSPSmnState *s = PSP_SMN(dev);
    int i;
    char name[PSP_SMN_SLOT_NAME_LEN] = { 0 };
    hwaddr slot_offset;

    for(i = 0; i < PSP_SMN_SLOT_COUNT; i++) {
        /* The SMN MMIO region of the PSP */
        snprintf(name, PSP_SMN_SLOT_NAME_LEN, "%s%d", PSP_SMN_SLOT_NAME, i);

        /* TODO: How to check for errors? */
        /* Each container is an alias to the SMN addres space. Here we
         * initialize each container beginning with the address 0 in the SMN
         * address space.
         */
        memory_region_init_alias(&s->psp_smn_containers[i], OBJECT(dev), name,
                                 &s->psp_smn_space, i * PSP_SMN_SLOT_SIZE,
                                 PSP_SMN_SLOT_SIZE);

        /* Map the containers to the PSP address space */
        slot_offset = s->psp_smn_base + i * PSP_SMN_SLOT_SIZE;

        memory_region_add_subregion_overlap(get_system_memory(), slot_offset,
                                            &s->psp_smn_containers[i], 0);

    }

}

static void psp_smn_realize(DeviceState *dev, Error **errp) {
    PSPSmnState *s = PSP_SMN(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    MemoryRegion *mr_smn_misc;
    PSPSmnFlashState* flash;

    /* The SMN address space. Independent from the PSP address space */
    memory_region_init(&s->psp_smn_space, OBJECT(dev), "smn-address-space",
                       0xFFFFFFFF);

    /* The SMN MMIO regions containing the control regs */
    memory_region_init_io(&s->psp_smn_control, OBJECT(dev), &smn_ctlr_ops, s,
                          TYPE_PSP_SMN, PSP_SMN_CTRL_SIZE);

    sysbus_init_mmio(sbd, &s->psp_smn_control);

    /* Connect the misc device to the SMN address space */
    /* TODO: Is this the way to go? ... */
    s->psp_smn_misc.regs = psp_regs;
    s->psp_smn_misc.regs_count = ARRAY_SIZE(psp_regs);

    mr_smn_misc = sysbus_mmio_get_region(SYS_BUS_DEVICE(&s->psp_smn_misc), 0);
    memory_region_add_subregion_overlap(&s->psp_smn_space, 0x0, mr_smn_misc,
                                        -1000);

    /* Map the flash region into the SMN address space */
    flash = &s->psp_smn_flash;
    memory_region_add_subregion_overlap(&s->psp_smn_space, PSP_SMN_FLASH_BASE,
                                        &flash->psp_smn_flash, 0);

    /* Setup the initial SMN to PSP MemoryRegion alias. */
    psp_smn_init_slots(dev);
}

static void psp_smn_class_init(ObjectClass *oc, void *data) {
    DeviceClass *dc = DEVICE_CLASS(oc);
    dc->realize = psp_smn_realize;
}

static const TypeInfo psp_smn_info = {
    .name = TYPE_PSP_SMN,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_init = psp_smn_init,
    .instance_size = sizeof(PSPSmnState),
    .class_init = psp_smn_class_init,

};

static void psp_smn_register_types(void) {
    type_register_static(&psp_smn_info);

}
type_init(psp_smn_register_types);
