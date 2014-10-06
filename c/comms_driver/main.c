#include "stdio.h"
#include "comms.h"

comms_t *serial_comms;

int main()
{
    serial_comms = comms_create(NULL, 1000);

    comms_destroy(serial_comms);

    return 0;
}
