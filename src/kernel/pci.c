#include "pci.h"

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
