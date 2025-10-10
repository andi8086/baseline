#ifndef PCI_H
#define PCI_H


#include <stdint.h>


void pci_init(void);
uint32_t pci_read_32(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn,
                     uint32_t offset);
uint16_t pci_read_16(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn,
                     uint32_t offset);
uint16_t pci_read_8(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn,
                    uint32_t offset);


#endif
