#include "stdio.h"
#include "comms.h"

bool publish_stdout(uint8_t byte);
void handle_kill(uint8_t *msg, uint16_t msg_len);

comms_t *stdout_comms;

int main()
{
    stdout_comms = comms_create(publish_stdout, 256);

    uint16_t msg_len = 5;
    uint8_t msg[5] = {0,1,2,3,4};


    comms_subscribe(stdout_comms, CHANNEL_KILL, handle_kill);

    comms_publish_blocking(stdout_comms, CHANNEL_KILL, msg, msg_len);

    comms_destroy(stdout_comms);

    return 0;
}

bool publish_stdout(uint8_t byte)
{
    printf("Publishing byte: %x\n", byte);
    comms_handle(stdout_comms, byte);
    return true;
}

void handle_kill(uint8_t *msg, uint16_t msg_len)
{
    printf("Received message on channel kill: ");
    uint8_t i;
    for(i = 0; i < msg_len; ++i)
    {
        printf("%x", msg[i]);
    }
    printf("\n");
}
