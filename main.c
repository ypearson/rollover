#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>

#define ROLLOVER_MODE 0 // set to 1 to switch modes

#define MAX_VALUE 1000U

unsigned int tick = 0;
unsigned int done = 0;
unsigned int start_move = 0;
unsigned int move_timeout = 100;

unsigned int HAL_GetTick(void)
{
    return tick;
}

void *Systick_Handler(void *vargp)
{
    while(!done)
    {
        usleep(4000);
        tick++;
        tick %= MAX_VALUE;
    }
}

void move_function(void)
{
    printf("\nMOVE\n");
    start_move = 0;
}

void doing_some_work(void)
{
    unsigned int work_time = ((unsigned int)rand() % 40000) + 4000;
    usleep(work_time);
}

void *delay_thread(void *vargp)
{
    while(!done)
    {

        doing_some_work();

        if(!start_move)
        {
            start_move = HAL_GetTick();
            continue;
        }

        unsigned int check_time = HAL_GetTick();

        printf("HAL_GetTick()=[%u]||||"
                "start_move=[%u]||||"
                "HAL_GetTick() - start_move=[%u]||||"
                "MAX_VALUE - start_move + check_time [%u]\n",
            HAL_GetTick(), start_move, HAL_GetTick() - start_move, MAX_VALUE - start_move + check_time);

        #if ROLLOVER_MODE
        if(HAL_GetTick() - start_move > move_timeout)
        {
            move_function();
        }
        #else
        if(check_time > start_move)
        {
            if(check_time - start_move > move_timeout)
            {
                move_function();
                printf("+++\n");
            }
        }
        else
        {
            if(MAX_VALUE - start_move + check_time > move_timeout)
            {
                move_function();
                printf("---\n");
            }
        }
        #endif
    }
}


int main(int argc, char const *argv[])
{
    printf("size of unsigned int is %ld bytes\n",sizeof(tick) );
    pthread_t thread_id1;
    pthread_t thread_id2;
    pthread_create(&thread_id1, NULL, Systick_Handler, NULL);
    pthread_create(&thread_id2, NULL, delay_thread, NULL);
    sleep(12);
    done = 1;
    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);
    printf("Done running test.\n");

    return 0;
}
