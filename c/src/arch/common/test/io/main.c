#include "stdio.h"
#include "io/comms.h"

void publish_stdout(container_t *data);
void handle_kill(void *usr, uint16_t id, comms_channel_t channel,
                 const uint8_t *msg, uint16_t len);

comms_t *stdout_comms;

int main()
{
    stdout_comms = comms_create(256, 256, publish_stdout);

    uint16_t msg_len = 5;
    uint8_t msg[5] = {0,1,2,3,4};


    comms_subscribe(stdout_comms, CHANNEL_KILL, handle_kill, NULL);

    comms_publish(stdout_comms, CHANNEL_KILL, msg, msg_len);

    comms_destroy(stdout_comms);

    return 0;
}

void publish_stdout(container_t *data)
{
    while(comms_cfuncs->size(data) > 0)
    {
        const uint8_t *byte = comms_cfuncs->front(data);
        printf("Publishing byte: %x\n", *byte);
        comms_handle(stdout_comms, *byte);
        comms_cfuncs->remove_front(data);
    }
}

void handle_kill(void *usr, uint16_t id, comms_channel_t channel,
                 const uint8_t *msg, uint16_t len)
{
    printf("Received message with id %d on channel kill: ", id);
    uint16_t i;
    for(i = 0; i < len; ++i)
    {
        printf("%x", msg[i]);
    }
    printf("\n");
}
