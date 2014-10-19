#include <stdio.h>
#include <pthread.h>

#include "io/comms.h"

void* publish_run(void *args);
void publish_stdout(container_t *data);
void handle_kill(void *usr, uint16_t id, comms_channel_t channel,
                 const uint8_t *msg, uint16_t len);

#define NUM_THREADS 10

pthread_t publish_thread[NUM_THREADS];
pthread_mutex_t lock;
comms_t *stdout_comms;

int main()
{
    stdout_comms = comms_create(256, 256, NUM_THREADS, publish_stdout);

    comms_subscribe(stdout_comms, CHANNEL_KILL, handle_kill, NULL);

    pthread_mutex_init(&lock, NULL);

    uint32_t i;
    for(i = 0; i < NUM_THREADS; ++i)
    {
        pthread_create(&publish_thread[i], NULL, publish_run, (void*)((uint64_t)i));
    }

    for(i = 0; i < NUM_THREADS; ++i)
    {
        void *ret;
        pthread_join(publish_thread[i], &ret);
    }

    while(1)
    {
        pthread_mutex_lock(&lock);
            if((comms_transmit(stdout_comms) == COMMS_STATUS_DONE) ||
               (comms_transmit(stdout_comms) == COMMS_STATUS_NO_ACTION))
            {
                pthread_mutex_unlock(&lock);
                break;
            }
        pthread_mutex_unlock(&lock);
        usleep(1000);
    }

    pthread_mutex_destroy(&lock);

    comms_destroy(stdout_comms);

    return 0;
}

void* publish_run(void* args)
{
    uint32_t i = (uint64_t)args;

    uint16_t msg_len = 5;
    uint8_t msg[5] = {0,1,2,3,4};

    pthread_mutex_lock(&lock);
        comms_publish_id(stdout_comms, i, CHANNEL_KILL, msg, i, msg_len);
    pthread_mutex_unlock(&lock);
    return NULL;
}

void publish_stdout(container_t *data)
{
    while(comms_cfuncs->size(data) > 0)
    {
        const uint8_t *byte = comms_cfuncs->front(data);
        //printf("Publishing byte: %x\n", *byte);
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
