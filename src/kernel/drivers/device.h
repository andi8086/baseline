#ifndef DEVICE_H
#define DEVICE_H


#include <stdint.h>


typedef enum {
        DEVICE_TYPE_ACPI,
        DEVICE_TYPE_PCI
} device_type_t;


/* Device should not utilize more than 8 IRQ lines */
#define KERNEL_DEV_IRQ_MAX 8
#define KERNEL_DEV_IO_MAX 64
#define KERNEL_DEVICES_MAX 256


typedef struct {
        uint32_t seg;
        uint32_t bus;
        uint32_t dev;
        uint32_t fun;
        uint16_t vendor_id;
        uint16_t device_id;
} dev_pci_info_t;


typedef struct {
        uint64_t dev_uid;
        device_type_t type;
        uint8_t irqs[KERNEL_DEV_IRQ_MAX];
        uint8_t num_irqs;
        uint16_t io_ports[KERNEL_DEV_IO_MAX];
        uint8_t num_ports;
        char *acpi_path;
        union {
                dev_pci_info_t pci_info;
        };
} device_t;


device_t *device_create(void);
void device_irq_add(device_t *dev, uint8_t irq);
void device_port_add(device_t *dev, uint16_t port);

#endif
