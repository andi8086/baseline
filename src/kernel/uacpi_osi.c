#include <uacpi/kernel_api.h>

#include <stdint.h>
#include "pe_mem.h"
#include "klib.h"
#include "kio.h"
#include "kmutex.h"
#include "kint.h"

#include "video/vcon.h"
#include "video/gc.h"


uacpi_i32 uacpi_vsnprintf(
    uacpi_char *buffer, uacpi_size capacity, const uacpi_char *fmt,
    uacpi_va_list vlist
);

uacpi_i32 uacpi_snprintf(
    uacpi_char *buffer, uacpi_size capacity, const uacpi_char *fmt, ...
);

extern vcon_t boot_console;


static void kprintf(char *fmt, ...)
{
        char buffer[80];
        va_list l, l2;
        va_start(l, fmt);
        uacpi_vsnprintf(buffer, 80, fmt, l);
        va_end(l);

        vcon_printf(&boot_console, buffer);
        gc_update_fb(boot_console.gc, 64, 64);
}


/* Returns the physical address of the RSDP structure */
uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address)
{
        extern uint32_t rsdp;

        if (!rsdp) {
                return UACPI_STATUS_NOT_FOUND;
        }

        *out_rsdp_address = (uintptr_t)rsdp;

        return UACPI_STATUS_OK;
}


void *uacpi_kernel_alloc(uacpi_size size)
{
        return (void *)kmalloc_high(size);
}


void *uacpi_kernel_alloc_zeroed(uacpi_size size)
{
        void *p = kmalloc_high(size);
        if (p) {
                memset(p, 0, size);
        }
        return p;
}


void uacpi_kernel_free(void *mem)
{
        kfree(mem);
}


void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len)
{
        /* kernel has identity mapping :) */
        return (void *)(uintptr_t)addr;
}


void uacpi_kernel_unmap(void *addr, uacpi_size len)
{
        /* nothing to do yet */
}


uacpi_status uacpi_kernel_io_map(
    uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle
)
{
        /* not possible on x86, we just store the base as handle */
        *out_handle = (uacpi_handle)(uintptr_t)base;

        return UACPI_STATUS_OK;
}


void uacpi_kernel_io_unmap(uacpi_handle handle)
{
        /* not possible on x86 */
}


uacpi_status uacpi_kernel_io_read8(
    uacpi_handle base, uacpi_size offset, uacpi_u8 *out_value
)
{
        *out_value = inb((uint32_t)base + offset);
        return UACPI_STATUS_OK;
}


uacpi_status uacpi_kernel_io_read16(
    uacpi_handle base, uacpi_size offset, uacpi_u16 *out_value
)
{
        *out_value = inw((uint32_t)base + offset);
        return UACPI_STATUS_OK;
}


uacpi_status uacpi_kernel_io_read32(
    uacpi_handle base, uacpi_size offset, uacpi_u32 *out_value
)
{
        *out_value = ind((uint32_t)base + offset);
        return UACPI_STATUS_OK;
}


uacpi_status uacpi_kernel_io_write8(
    uacpi_handle base, uacpi_size offset, uacpi_u8 in_value
)
{
//        kprintf("outb(%x, %x)\n", (uintptr_t)base + offset, in_value);
        outb((uintptr_t)base + offset, in_value);
        return UACPI_STATUS_OK;
}


uacpi_status uacpi_kernel_io_write16(
    uacpi_handle base, uacpi_size offset, uacpi_u16 in_value
)
{
//        kprintf("outw(%x, %x)\n", (uintptr_t)base + offset, in_value);
        outw((uintptr_t)base + offset, in_value);
        return UACPI_STATUS_OK;

}


uacpi_status uacpi_kernel_io_write32(
    uacpi_handle base, uacpi_size offset, uacpi_u32 in_value
)
{
//        kprintf("outd(%x, %x)\n", (uintptr_t)base + offset, in_value);
        outd((uintptr_t)base + offset, in_value);
        return UACPI_STATUS_OK;
}


uacpi_handle uacpi_kernel_create_mutex(void)
{
        return (uacpi_handle)kmutex_create();
}


void uacpi_kernel_free_mutex(uacpi_handle m)
{
        kmutex_free((kmutex_t *)m);
}


uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle m, uacpi_u16 timeout)
{
        switch (timeout) {
        case 0:
                if (kmutex_lock_try_once((kmutex_t *)m)) {
                        return UACPI_STATUS_OK;
                }
                return UACPI_STATUS_TIMEOUT;
        case 0xFFFF:
        default:
                /* TODO: timeout for < 0xFFFF not implemented */
                kmutex_lock((kmutex_t *)m);
                return UACPI_STATUS_OK;
        }

        return UACPI_STATUS_OK;
}


void uacpi_kernel_release_mutex(uacpi_handle m)
{
        kmutex_unlock((kmutex_t *)m);
}




uacpi_handle uacpi_kernel_create_event(void)
{
        uint32_t *event;

        event = kmalloc_high(sizeof(uint32_t));

        if (event) {
                *event = 0;
        }

        return (uacpi_handle)event;
}


void uacpi_kernel_free_event(uacpi_handle e)
{
        kfree(e);
}



void uacpi_kernel_log(uacpi_log_level lvl, const uacpi_char* msg)
{

        vcon_printf(&boot_console, (char *)msg);
        gc_update_fb(boot_console.gc, 64, 64);
}



void uacpi_kernel_stall(uacpi_u8 usec)
{
        char buffer[32];
        uacpi_snprintf(buffer, 32, "stall_us(%u)\n", usec);
        vcon_printf(&boot_console, (char *)buffer);
        gc_update_fb(boot_console.gc, 64, 64);
}


void uacpi_kernel_sleep(uacpi_u64 msec)
{
        char buffer[32];
        uacpi_snprintf(buffer, 32, "sleep_ms(%u)\n", msec);
        vcon_printf(&boot_console, (char *)buffer);
        gc_update_fb(boot_console.gc, 64, 64);
}


uacpi_status uacpi_kernel_pci_device_open(
    uacpi_pci_address address, uacpi_handle *out_handle
)
{
        memcpy(out_handle, &address, sizeof(uacpi_pci_address));

/*        kprintf("PCI dev open(seg=%u, bus=%u, dev=%u, fn=%u)\n",
                address.segment, address.bus, address.device, address.function);
*/
        return UACPI_STATUS_OK;
}


void uacpi_kernel_pci_device_close(uacpi_handle)
{
        /* NOOP */
}


uacpi_status uacpi_kernel_pci_read8(
    uacpi_handle device, uacpi_size offset, uacpi_u8 *value
)
{
        uacpi_pci_address addr;
        memcpy(&addr, device, sizeof(uacpi_pci_address));
        uint32_t seg = addr.segment;
        uint32_t bus = addr.bus;
        uint32_t dev = addr.device;
        uint32_t fn = addr.function;

        uint32_t dword_offset = offset & 0xFC;

        uint32_t address = (uint32_t)((bus << 16) | (dev << 11) |
                        (fn << 8) | (dword_offset) |
                        (uint32_t)0x80000000);

        outd(0xCF8, address);

        uint32_t dword_in = ind(0xCFC);

        switch (offset & 3) {
        case 0: *value = dword_in & 0xFF;
        case 1: *value = (dword_in >> 8) & 0xFF;
        case 2: *value = (dword_in >> 16) & 0xFF;
        case 3: *value = (dword_in >> 24) & 0xFF;
        }
        return UACPI_STATUS_OK;
}


uacpi_status uacpi_kernel_pci_read16(
    uacpi_handle device, uacpi_size offset, uacpi_u16 *value
)
{
        kprintf("pci_read16");
        while (1);
}


uacpi_status uacpi_kernel_pci_read32(
    uacpi_handle device, uacpi_size offset, uacpi_u32 *value
)
{
        kprintf("pci_read32");
        while (1);
}



uacpi_status uacpi_kernel_pci_write8(
    uacpi_handle device, uacpi_size offset, uacpi_u8 value
)
{
        while (1);
}


uacpi_status uacpi_kernel_pci_write16(
    uacpi_handle device, uacpi_size offset, uacpi_u16 value
)
{
        while (1);
}


uacpi_status uacpi_kernel_pci_write32(
    uacpi_handle device, uacpi_size offset, uacpi_u32 value
)
{
        while (1);
}


uacpi_handle uacpi_kernel_create_spinlock(void)
{
        return (uacpi_handle *)kmutex_create();
}


void uacpi_kernel_free_spinlock(uacpi_handle m)
{
        kmutex_free((kmutex_t *)m);
}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle m)
{
        kmutex_lock((kmutex_t *)m);
}


void uacpi_kernel_unlock_spinlock(uacpi_handle m, uacpi_cpu_flags)
{
        kmutex_unlock((kmutex_t *)m);
}




uacpi_status uacpi_kernel_install_interrupt_handler(
    uacpi_u32 irq, uacpi_interrupt_handler handler, uacpi_handle ctx,
    uacpi_handle *out_irq_handle
)
{
        char buffer[32];
        asm("cli");
        kprintf("Install int handler for irq %u\n", irq);

        if (irq > 15) {
                return UACPI_STATUS_NOT_FOUND;
        }
        bool res = irq_handler_register(irq, (uint32_t)handler);
        asm("sti");
        if (res) {
                return UACPI_STATUS_OK;
        }
        return UACPI_STATUS_NOT_FOUND;
}


uacpi_status uacpi_kernel_uninstall_interrupt_handler(
    uacpi_interrupt_handler, uacpi_handle irq_handle
)
{
        while (1);
}


uacpi_thread_id uacpi_kernel_get_thread_id(void)
{
        /* FIXME */
        return (uacpi_thread_id)1;
}


uacpi_status uacpi_kernel_schedule_work(
    uacpi_work_type, uacpi_work_handler, uacpi_handle ctx
)
{
        while (1);
}


uacpi_status uacpi_kernel_wait_for_work_completion(void)
{
        while (1);
}


uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void)
{
        /* FIXME */
        return 10000000;
        while (1);
}


void uacpi_kernel_reset_event(uacpi_handle e)
{
        __atomic_fetch_add((uint32_t *)e, 0, __ATOMIC_SEQ_CST);
}


void uacpi_kernel_signal_event(uacpi_handle e)
{
        __atomic_fetch_add((uint32_t *)e, 1, __ATOMIC_SEQ_CST);
}


uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle e, uacpi_u16 timeout)
{
        uint32_t *counter = (uint32_t *)e;

        if (timeout == 0xFFFF) {
                while (*counter != 0) {
                        asm("pause");
                }
                return UACPI_TRUE;
        }

        /* FIXME: handle timeout */
        /*
        for (;;) {
                if (*counter == 0) {
                        return UACPI_TRUE;
                }
                usec = tmr.get_time_usec();
                elapsed_msec = (usec - usec_start) / 1000;
                if (elapsed_msec >= timeout) P{
                        break;
                }
                md_pause();
        }
        __atomic_fetch_sub((uint32_t *)e, 1, __ATOMIC_SEQ_CST);
        return UACPI_FALSE;


        */

        for (;;) {
                if (*counter == 0) {
                        return UACPI_TRUE;
                }
                asm("pause");
        }
        __atomic_fetch_sub((uint32_t *)e, 1, __ATOMIC_SEQ_CST);
        return UACPI_FALSE;
}


uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request*)
{
        while (1);

}
