#include "qemu/osdep.h"
#include "qemu/module.h"
#include "qemu/log.h"
#include "qapi/error.h"

#include "hw/arm/psp-smn-misc.h"

#define PSP_SMN_MISC_LOG "[PSP SMN MISC]"

/* TODO make this class dependant */
#define PSP_SMN_MMIO_SIZE 0x44000000

static uint64_t psp_smn_misc_read(void *opaque, hwaddr offset,
                           unsigned int size)
{
    uint64_t val;
    switch(offset) {
        // PSPEMU verified
        case 0x5a078:
            val = 0x10;
            break;

        case 0x5a088:
            val =0x1;
            break;

        case 0x5a870:
            val =0x1;
            break;

        case 0x5d0cc:
            val =0x20;
            break;

        case 0x5e000:
            val =0x1;
            break;

        // PSPEMU verified
        case 0x1003034:
            val =0x1e112;
            break;

        // PSPEMU verified
        case 0x1004034:
            val =0x1e112;
            break;

        // PSPEMU verified
        case 0x1018034:
            val =0x1e113;
            break;

        // PSPEMU verified
        case 0x1025034:
            val =0x1e113;
            break;

        // PSPEMU verified
        case 0x102e034:
            val =0x1e312;
            break;

        // PSPEMU verified
        case 0x1030034:
            val =0x1e312;
            break;

        case 0x1046034:
            val = 0x1e103;
            break;

        case 0x1047034:
            val = 0x1e103;
            break;

        // PSPEMU verified
        case 0x106c034:
            val = 0x1e113;
            break;

        // PSPEMU verified
        case 0x106d034:
            val = 0x1e113;
            break;

        // PSPEMU verified
        case 0x106e034:
            val = 0x1e312;
            break;

        // PSPEMU verified
        case 0x1080034:
            val = 0x1e113;
            break;

        // PSPEMU verified
        case 0x1081034:
            val = 0x1e113;
            break;

        // PSPEMU verified
        case 0x1096034:
            val = 0x1e312;
            break;

        // PSPEMU verified
        case 0x1097034:
            val = 0x1e312;
            break;

        // PSPEMU verified
        case 0x10a8034:
            val = 0x1e312;
            break;

        // PSPEMU verified
        case 0x10d8034:
            val = 0x1e312;
            break;

        case 0x18080064:
            val =0x600;
            break;

        case 0x18480064:
            val =0x600;
            break;

        default:
            qemu_log_mask(LOG_TRACE, "%s: unimplemented device read at offset: 0x%"
                  HWADDR_PRIx " (size: %d, value: TODO)\n",
                  PSP_SMN_MISC_LOG, offset, size);
            return 0;
    }

    qemu_log_mask(LOG_TRACE, "%s: read at offset: 0x%"
                  HWADDR_PRIx " (size: %d, value: 0x%lu)\n",
                  PSP_SMN_MISC_LOG, offset, size, val);
    return val;
}

static void psp_smn_misc_write(void *opaque, hwaddr offset, uint64_t value,
                           unsigned int size)
{
    qemu_log_mask(LOG_TRACE, "%s: unimplemented device write at offset: 0x%"
                  HWADDR_PRIx " (size: %d, value: 0x%lx)\n",
                  PSP_SMN_MISC_LOG, offset, size, value);
}

static const MemoryRegionOps misc_mem_ops = {
    .read = psp_smn_misc_read,
    .write = psp_smn_misc_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl.min_access_size = 1,
    .impl.max_access_size = 4,
};

static void psp_misc_realize(DeviceState *dev, Error **errp) {
    PSPSmnMiscState * s = PSP_SMN_MISC(dev);

    /* Init mmio */
    /* TODO make size class based. Or over full range with lower prio */
    memory_region_init_io(&s->iomem, OBJECT(dev), &misc_mem_ops, s,
                          "psp-smn-misc", PSP_SMN_MMIO_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void psp_smn_misc_init(Object *o) {
}

static void psp_smn_misc_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);
    dc->realize = psp_misc_realize;
}

static const TypeInfo psp_smn_misc_info = {
    .name = TYPE_PSP_SMN_MISC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_init = psp_smn_misc_init,
    .instance_size = sizeof(PSPSmnMiscState),
    .class_init = psp_smn_misc_class_init,
};

static void psp_misc_register_types(void)
{
    type_register_static(&psp_smn_misc_info);
}

type_init(psp_misc_register_types);
