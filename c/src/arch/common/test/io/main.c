#include "stdio.h"
#include "io/comms.h"

bool publish_stdout(uint8_t *data, uint16_t datalen);
void handle_kill(void *usr, uint16_t id, comms_channel_t channel,
                 uint8_t *msg, uint16_t msg_len);

comms_t *stdout_comms;

int main()
{
    stdout_comms = comms_create(256, 256);
    comms_add_publisher(stdout_comms, publish_stdout);

    uint16_t msg_len = 5;
    uint8_t msg[5] = {0,1,2,3,4};


    comms_subscribe(stdout_comms, CHANNEL_KILL, handle_kill, NULL);

    comms_publish(stdout_comms, CHANNEL_KILL, msg, msg_len);

    comms_destroy(stdout_comms);

    return 0;
}

bool publish_stdout(uint8_t *data, uint16_t data_len)
{
    uint16_t i;
    for(i = 0; i < data_len; ++i)
    {
        printf("Publishing byte: %x\n", data[i]);
        comms_handle(stdout_comms, data[i]);
    }
    return true;
}

void handle_kill(void *usr, uint16_t id, comms_channel_t channel,
                 uint8_t *msg, uint16_t msg_len)
{
    printf("Received message with id %d on channel kill: ", id);
    uint8_t i;
    for(i = 0; i < msg_len; ++i)
    {
        printf("%x", msg[i]);
    }
    printf("\n");
}
