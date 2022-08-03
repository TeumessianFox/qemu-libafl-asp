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
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/qdev-properties.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/arm/psp.h"
#include "hw/arm/psp-misc.h"
#include "hw/arm/psp-smn.h"
#include "hw/arm/psp-x86.h"
#include "hw/arm/psp-timer.h"
#include "hw/arm/psp-sts.h"
#include "qemu/log.h"

/* TODO: use mmio_map_overlap with memory_regions_dispatch_rw to log access to SPI flash */

PspGeneration PspNameToGen(const char* name) {
  /* TODO: Make the generation a property of the "psp" class */
  return ZEN;
}

static uint32_t PspGetSramSize(PspGeneration gen) {
  switch(gen) {
    case ZEN:
      return PSP_SRAM_SIZE_ZEN;
    case ZEN_PLUS:
      return PSP_SRAM_SIZE_ZEN_PLUS;
    case ZEN2:
      return PSP_SRAM_SIZE_ZEN_2;
    default:
      return PSP_SRAM_SIZE_ZEN;
  }

}

//* TODO: This is currently not mapped */
//  .addr = 0x0320004c,
//  .val = 0xbc090072,

/* static const char* GenNames[3] = { "Zen", "Zen+", "Zen2"}; */
/* TODO: Maybe use TYPE_CPU_CLUSTER to create an SoC with multiple PSP's.
 * Example in armsse.c: armsse_init() */
/* NOTE: MachineState = state of instantiated MachineClass */

/* TODO: Check CPU Object properties */

/* Initialize psp and its device. This should never fail */
static void amd_psp_init(Object *obj)
{

    AmdPspState *s = AMD_PSP(obj);

    object_initialize_child(obj, "cpu", &s->cpu,
                            ARM_CPU_TYPE_NAME("cortex-a9"));

    object_initialize_child(obj, "smn", &s->smn, TYPE_PSP_SMN);

    //object_initialize_child(obj, "x86", &s->x86, TYPE_PSP_X86);

    object_initialize_child(obj, "base_mem", &s->base_mem, TYPE_PSP_MISC);

    object_initialize_child(obj, "timer1", &s->timer1,TYPE_PSP_TIMER);

    object_initialize_child(obj, "timer2", &s->timer2, TYPE_PSP_TIMER);

    object_initialize_child(obj, "psp-sts", &s->sts, TYPE_PSP_STS);

    object_initialize_child(obj, "psp-ccp", &s->ccp, TYPE_CCP_V5);

    object_initialize_child(obj, "psp-fuse", &s->fuse, TYPE_PSP_FUSE);

    object_initialize_child(obj, "unimp", &s->unimp, TYPE_UNIMPLEMENTED_DEVICE);
}

/* Realize the psp and configure devices */
static void amd_psp_realize(DeviceState *dev, Error **errp)
{
    AmdPspState *s = AMD_PSP(dev);

    // TODO check and set generation
    PspGeneration gen;
    if(s->gen_ident) {
        qemu_log("TODO set generations");
        exit(1);
    } else {
        // default
        s->gen_ident = g_strdup("Zen");
        gen = PspNameToGen("Zen");
    }
    s->gen = gen;
    qemu_log("Generation: %s", s->gen_ident);

    /* TODO: Maybe use "&error_abort" instead? */
    Error *err = NULL;
    uint32_t sram_size;
    uint32_t sram_addr;

    /* Enable the ARM TrustZone extensions */
    //object_property_set_bool(OBJECT(&s->cpu), "has_el3", true,  &err);

    /* Init CPU object. TODO convert to qdev_init_nofail */
    qdev_realize(DEVICE(&s->cpu), NULL, &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }

    object_property_set_uint(OBJECT(&s->smn), "smn-container-base",
                             PSP_SMN_BASE, &error_abort);
    sysbus_realize(SYS_BUS_DEVICE(&s->smn), &err);

    //object_property_set_uint(OBJECT(&s->x86), "x86-container-base",
    //                        PSP_X86_BASE, &error_abort);
    //sysbus_realize(SYS_BUS_DEVICE(&s->x86), &err);

    object_property_set_uint(OBJECT(&s->base_mem), "psp_misc_msize",
                             (1UL << 32), &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }

    object_property_set_str(OBJECT(&s->base_mem), "psp_misc_ident",
                            "BASE MEM", &error_abort);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }

    sysbus_realize(SYS_BUS_DEVICE(&s->base_mem), &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }

    /* Init SRAM */
    sram_size = PspGetSramSize(s->gen);
    sram_addr = 0x0;
    memory_region_init_ram(&s->sram, OBJECT(dev), "sram", sram_size,
                           &error_abort);
    memory_region_add_subregion(get_system_memory(), sram_addr, &s->sram);

    /* Init ROM */
    memory_region_init_rom(&s->rom, OBJECT(dev), "rom", PSP_ROM_SIZE,
                           &error_abort);
    memory_region_add_subregion(get_system_memory(), PSP_ROM_BASE, &s->rom);

    /* Map SMN control registers */
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->smn), 0, PSP_SMN_CTRL_BASE);

    /* Map X86 control registers */
    //sysbus_mmio_map(SYS_BUS_DEVICE(&s->x86), 0, PSP_X86_CTRL1_BASE);

    /* Map X86 control registers */
    //sysbus_mmio_map(SYS_BUS_DEVICE(&s->x86), 1, PSP_X86_CTRL2_BASE);

    /* Map X86 control registers */
    //sysbus_mmio_map(SYS_BUS_DEVICE(&s->x86), 2, PSP_X86_CTRL3_BASE);

    /* Map timers */
    sysbus_realize(SYS_BUS_DEVICE(&s->timer1), &err);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->timer1), 0, PSP_TIMER1_BASE);
    sysbus_realize(SYS_BUS_DEVICE(&s->timer2), &err);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->timer2), 0, PSP_TIMER2_BASE);

    /* Map PSP Status port */
    sysbus_realize(SYS_BUS_DEVICE(&s->sts), &error_abort);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->sts), 0, PSP_STS_ZEN1_BASE);

    /* Map CCP */
    sysbus_realize(SYS_BUS_DEVICE(&s->ccp), &error_abort);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->ccp), 0, PSP_CCP_BASE);

    /* Map Fuse */
    sysbus_realize(SYS_BUS_DEVICE(&s->fuse), &error_abort);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->fuse), 0, PSP_FUSE_BASE);

    /* Map the misc device as an overlap with low priority */
    /* This device covers all "unknown" psp registers */
    /* TODO reduce this to only cover known misc regs */
    sysbus_mmio_map_overlap(SYS_BUS_DEVICE(&s->base_mem), 0, 0, -1000);

    /* General unimplemented device that maps the whole memory with low priority */
    qdev_prop_set_string(DEVICE(&s->unimp), "name", "unimp");
    qdev_prop_set_uint64(DEVICE(&s->unimp), "size", sram_size);
    sysbus_realize(SYS_BUS_DEVICE(&s->unimp), &error_abort);
    sysbus_mmio_map_overlap(SYS_BUS_DEVICE(&s->unimp), 0, 0, -1000);

}

/* User-configurable options with "-global amd-psp.<property>=<value> */
static Property amd_psp_properties[] = {
    DEFINE_PROP_STRING("generation", AmdPspState, gen_ident),
    DEFINE_PROP_BOOL("dbg_mode", AmdPspState, dbg_mode, false),
    DEFINE_PROP_END_OF_LIST(),
};

static void amd_psp_class_init(ObjectClass *oc, void* data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);
    device_class_set_props(dc,amd_psp_properties);
    /* TODO: Add generation to the description */
    dc->desc = "AMD PSP";
    dc->realize = amd_psp_realize;
    /* Disabling device instantiation with arbitrary machines */
    dc->user_creatable = false;
}

/* TODO: Use "create_unimplemented_device" to log accesses to unknown mem areas */

static const TypeInfo amd_psp_type_info = {
    .name = TYPE_AMD_PSP,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(AmdPspState),
    .instance_init = amd_psp_init,
    .class_init = amd_psp_class_init,
};

static void amd_psp_register_types(void)
{
    type_register_static(&amd_psp_type_info);
}

type_init(amd_psp_register_types);
