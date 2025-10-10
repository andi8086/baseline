#include "pci.h"
#include "kio.h"

#include <uacpi/uacpi.h>
#include <uacpi/utilities.h>

#include "video/vcon.h"
#include "video/gc.h"


extern vcon_t boot_console;

uacpi_iteration_decision pci_check_acpi_root(
        void *, uacpi_namespace_node *node, uint32_t depth)
{
        uint64_t seg;
        uint64_t bus;

        uacpi_eval_integer(node, "_SEG", NULL, &seg);
        uacpi_eval_integer(node, "_BBN", NULL, &bus);

        vcon_printf(&boot_console, "pci bridge seg %p, bus %p\n",
                seg, bus);
        gc_update_fb(boot_console.gc, 64, 64);

        return UACPI_ITERATION_DECISION_CONTINUE;
}


void pci_init(void)
{
        static const char *pci_root_ids[] = {
                "PNP0A03", "PNP0A08", NULL
        };

        uacpi_find_devices_at(uacpi_namespace_get_predefined(
                UACPI_PREDEFINED_NAMESPACE_SB),
                pci_root_ids,
                pci_check_acpi_root,
                NULL);

}


uint32_t pci_read_32(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn,
                     uint32_t offset)
{
        uint32_t dword_offset = offset & 0xFC;

        uint32_t address = (uint32_t)((bus << 16) | (dev << 11) |
                        (fn << 8) | (dword_offset) |
                        (uint32_t)0x80000000);

        outd(0xCF8, address);

        uint32_t dword_in = ind(0xCFC);

        return dword_in;
}


uint16_t pci_read_16(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn,
                     uint32_t offset)
{
        uint32_t dword_offset = offset & 0xFC;

        uint32_t address = (uint32_t)((bus << 16) | (dev << 11) |
                        (fn << 8) | (dword_offset) |
                        (uint32_t)0x80000000);

        outd(0xCF8, address);

        uint32_t dword_in = ind(0xCFC);

        switch (offset & 2) {
        case 0: return dword_in & 0xFFFF;
        case 2: return (dword_in >> 16) & 0xFFFF;
        }
}


uint16_t pci_read_8(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn,
                    uint32_t offset)
{
        uint32_t dword_offset = offset & 0xFC;

        uint32_t address = (uint32_t)((bus << 16) | (dev << 11) |
                        (fn << 8) | (dword_offset) |
                        (uint32_t)0x80000000);

        outd(0xCF8, address);

        uint32_t dword_in = ind(0xCFC);

        switch (offset & 3) {
        case 0: return dword_in & 0xFF;
        case 1: return (dword_in >> 8) & 0xFF;
        case 2: return (dword_in >> 16) & 0xFF;
        case 3: return (dword_in >> 24) & 0xFF;
        }
}
