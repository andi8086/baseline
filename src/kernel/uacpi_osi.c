#include <uacpi/kernel_api.h>

#include <stdint.h>
#include "pe_mem.h"
#include "klib.h"
#include "kio.h"

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
        *out_handle = (uacpi_handle *)(uintptr_t)base;

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
        outb((uintptr_t)base + offset, in_value);
        return UACPI_STATUS_OK;
}


uacpi_status uacpi_kernel_io_write16(
    uacpi_handle base, uacpi_size offset, uacpi_u16 in_value
)
{
        outw((uintptr_t)base + offset, in_value);
        return UACPI_STATUS_OK;

}


uacpi_status uacpi_kernel_io_write32(
    uacpi_handle base, uacpi_size offset, uacpi_u32 in_value
)
{
        outd((uintptr_t)base + offset, in_value);
        return UACPI_STATUS_OK;
}










uacpi_handle uacpi_kernel_create_mutex(void)
{

}


void uacpi_kernel_free_mutex(uacpi_handle)
{

}


uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle, uacpi_u16)
{

}


void uacpi_kernel_release_mutex(uacpi_handle)
{

}


uacpi_handle uacpi_kernel_create_event(void)
{

}


void uacpi_kernel_free_event(uacpi_handle)
{

}


void uacpi_kernel_log(uacpi_log_level, const uacpi_char*)
{

}



void uacpi_kernel_stall(uacpi_u8 usec)
{

}


void uacpi_kernel_sleep(uacpi_u64 msec)
{

}


uacpi_status uacpi_kernel_pci_device_open(
    uacpi_pci_address address, uacpi_handle *out_handle
)
{

}


void uacpi_kernel_pci_device_close(uacpi_handle)
{

}


uacpi_status uacpi_kernel_pci_read8(
    uacpi_handle device, uacpi_size offset, uacpi_u8 *value
)
{

}


uacpi_status uacpi_kernel_pci_read16(
    uacpi_handle device, uacpi_size offset, uacpi_u16 *value
)
{

}


uacpi_status uacpi_kernel_pci_read32(
    uacpi_handle device, uacpi_size offset, uacpi_u32 *value
)
{

}



uacpi_status uacpi_kernel_pci_write8(
    uacpi_handle device, uacpi_size offset, uacpi_u8 value
)
{

}


uacpi_status uacpi_kernel_pci_write16(
    uacpi_handle device, uacpi_size offset, uacpi_u16 value
)
{

}


uacpi_status uacpi_kernel_pci_write32(
    uacpi_handle device, uacpi_size offset, uacpi_u32 value
)
{

}


uacpi_handle uacpi_kernel_create_spinlock(void)
{

}


void uacpi_kernel_free_spinlock(uacpi_handle)
{

}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle)
{

}


void uacpi_kernel_unlock_spinlock(uacpi_handle, uacpi_cpu_flags)
{

}




uacpi_status uacpi_kernel_install_interrupt_handler(
    uacpi_u32 irq, uacpi_interrupt_handler, uacpi_handle ctx,
    uacpi_handle *out_irq_handle
)
{

}


uacpi_status uacpi_kernel_uninstall_interrupt_handler(
    uacpi_interrupt_handler, uacpi_handle irq_handle
)
{

}


uacpi_thread_id uacpi_kernel_get_thread_id(void)
{

}


uacpi_status uacpi_kernel_schedule_work(
    uacpi_work_type, uacpi_work_handler, uacpi_handle ctx
)
{

}


uacpi_status uacpi_kernel_wait_for_work_completion(void)
{

}


uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void)
{

}


void uacpi_kernel_reset_event(uacpi_handle)
{

}


void uacpi_kernel_signal_event(uacpi_handle)
{

}


uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle, uacpi_u16)
{

}


uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request*)
{

}
