#include "device.h"

#include <stddef.h>
#include "../klib.h"


int num_devices = 0;
device_t devices[KERNEL_DEVICES_MAX];


device_t *device_create(void)
{
        if (num_devices >= KERNEL_DEVICES_MAX) {
                return NULL;
        }

        memset(&devices[num_devices], 0, sizeof(device_t));
        num_devices++;
}


void device_irq_add(device_t *dev, uint8_t irq)
{
        if (dev->num_irqs >= KERNEL_DEV_IRQ_MAX) {
                return;
        }
        dev->irqs[dev->num_irqs] = irq;
        dev->num_irqs++;
}


void device_port_add(device_t *dev, uint16_t port)
{
        if (dev->num_ports >= KERNEL_DEV_IO_MAX) {
                return;
        }
        dev->io_ports[dev->num_ports] = port;
        dev->num_ports++;
}
