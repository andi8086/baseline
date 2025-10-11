#ifndef DRIVER_H
#define DRIVER_H


#include "device.h"


typedef struct {
        int (*probe)(device_t *dev);

} driver_api_t;



#endif
