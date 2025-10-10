#include "kdev.h"
#include "kprintf.h"
#include "klib.h"
#include "pci.h"
#include <uacpi/uacpi.h>
#include <uacpi/namespace.h>
#include <uacpi/utilities.h>
#include <uacpi/resources.h>


typedef struct {
        bool is_pci_bus;
        uint16_t pci_bus;
        int pci_slot_node_depth;
} dev_enum_ctx_t;


static uacpi_iteration_decision dev_map_resources(void *ctx, uacpi_resource *res)
{
        switch (res->type) {
        case UACPI_RESOURCE_TYPE_VENDOR_SMALL:
                kprintf("%.*s ", res->vendor.length, res->vendor.data);
                break;
        case UACPI_RESOURCE_TYPE_IRQ:
                kprintf("IRQs: ");
                for (int i = 0; i < res->irq.num_irqs; i++) {
                        kprintf("%u ", res->irq.irqs[i]);
                }
                break;
        case UACPI_RESOURCE_TYPE_EXTENDED_IRQ:
                kprintf("eIRQs: ");
                for (int i = 0; i < res->extended_irq.num_irqs; i++) {
                        kprintf("%u ", res->extended_irq.irqs[i]);
                }
                break;
        case UACPI_RESOURCE_TYPE_FIXED_IO:
                kprintf("f-io: %x-%x ", res->fixed_io.address,
                        res->fixed_io.address + res->fixed_io.length);
                break;
        case UACPI_RESOURCE_TYPE_IO:
                kprintf("io: %x-%x ", res->io.minimum, res->io.maximum);
                break;
        }
        return UACPI_ITERATION_DECISION_CONTINUE;
}


static uacpi_iteration_decision acpi_init_one_device(
        void *ctx, uacpi_namespace_node *node,
        uacpi_u32 node_depth)
{
        uacpi_namespace_node_info *info;
        (void)node_depth;

        dev_enum_ctx_t *enum_info = (dev_enum_ctx_t *)ctx;

        uacpi_status ret;

        ret = uacpi_get_namespace_node_info(node, &info);
        if (uacpi_unlikely_error(ret)) {
                return UACPI_ITERATION_DECISION_CONTINUE;
        }

        const char *path = uacpi_namespace_node_generate_absolute_path(node);
        // kprintf("device %s: ", path);

        if (info->flags & UACPI_NS_NODE_INFO_HAS_HID) {
                /* check if it is a PCI root bridge */
                if (memcmp(info->hid.value, "PNP0A08", info->hid.size) == 0 ||
                    memcmp(info->hid.value, "PNP0A03", info->hid.size) == 0) {
                        enum_info->is_pci_bus = true;
                        enum_info->pci_bus = 0;
                        enum_info->pci_slot_node_depth =
                                uacpi_namespace_node_depth(node) + 1;
                }
        }
        int nd = uacpi_namespace_node_depth(node);

        if (info->flags & UACPI_NS_NODE_INFO_HAS_ADR) {
                if (nd == enum_info->pci_slot_node_depth) {
                        enum_info->is_pci_bus = true;
                }
                if (enum_info->is_pci_bus) {
                        /* check if device exists */
                        uint16_t dev, fn;
                        dev = info->adr >> 16;
                        fn = info->adr & 0xFFFF;
                        uint32_t data = pci_read_32(0, enum_info->pci_bus, dev,
                                                    fn, 0);
                        if (data == UINT32_MAX) {
                                return UACPI_ITERATION_DECISION_CONTINUE;
                        }
                        kprintf("%x.%x ", dev, fn);
//                        kprintf("%x.%x ", dev, fn);
                        kprintf("%04X:%04X ", data & 0xFFFF, data >> 16);
                }


        } else {
                /* no address, this means we are on a device */
                kprintf("device %s: ", path);
                enum_info->is_pci_bus = false;
        }

        uacpi_for_each_device_resource(node, "_CRS", dev_map_resources, ctx);

        if (info->flags & UACPI_NS_NODE_INFO_HAS_UID) {
                kprintf("UID %.*s ", info->uid.size, info->uid.value);
        }


        if (info->flags & UACPI_NS_NODE_INFO_HAS_CLS) {
                  kprintf("Class %.*s ", info->cls.size, info->cls.value);
        }

        if (info->flags & UACPI_NS_NODE_INFO_HAS_HID) {
                /* TODO: match HID against pnp id */
                kprintf("%.*s\n", info->hid.size, info->hid.value);
        } else {
                kprintf("\n");
        }

        /* in case nothing was found above: */
        if (info->flags & UACPI_NS_NODE_INFO_HAS_CID) {
                for (int i = 0; i < info->cid.num_ids; i++) {
                        kprintf("%.*s ", info->cid.ids[i].size,
                                info->cid.ids[i].value);
                }
                kprintf("\n");
                /* TODO: match CID against pnp id */
        }

        /* TODO: if we found a matching driver, probe the device */

        uacpi_free_namespace_node_info(info);

        return UACPI_ITERATION_DECISION_CONTINUE;
}


void uacpi_devices_enumerate(void)
{
        /* tell ACPI we want APIC mode informations */
/*

        uacpi_object *arg1 = uacpi_object_create_integer(1);

        uacpi_object_array args = {
                .count = 1,
                .objects = &arg1
        };

        res = uacpi_execute(UACPI_NULL, "\\\\_PIC", &args);
        if (res == UACPI_STATUS_OK) {
                kprintf("\\_PIC(1) successful\n");
        } else {
                kprintf("\\_PIC(1) failed: %s\n", uacpi_status_to_string(res));
        }
*/
        uacpi_status res;
        res = uacpi_set_interrupt_model(UACPI_INTERRUPT_MODEL_IOAPIC);
        if (res == UACPI_STATUS_OK) {
                kprintf("ACPI: int model is IOAPIC\n");
        }

        dev_enum_ctx_t enum_ctx;
        enum_ctx.is_pci_bus = false;
        enum_ctx.pci_slot_node_depth = 0;

        uacpi_namespace_for_each_child(
                uacpi_namespace_root(),
                acpi_init_one_device,
                UACPI_NULL,
                UACPI_OBJECT_DEVICE_BIT,
                UACPI_MAX_DEPTH_ANY,
                &enum_ctx);
}
