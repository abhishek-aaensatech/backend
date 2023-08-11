#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int cnt1 = 0, cnt2 = 0, cnt3 = 0;

void *thread_function1(void *arg)
{
    while (1)
    {
        printf("Thread executed - 1 %d\n", cnt1++);
        if(cnt1 == 10000){
            printf("Thread Break - 1\n");
            break;
        }
        if(cnt1==10000 || cnt2==10000 || cnt3==10000)
            sleep(20);
    }
}
void *thread_function2(void *arg)
{
    while (1)
    {
        printf("Thread executed - 2 %d\n", cnt2++);
        if(cnt2 == 10000){
            printf("Thread Break - 2\n");
            break;
        }
        if(cnt1==10000 || cnt2==10000 || cnt3==10000)
            sleep(20);
    }
}
void *thread_function3(void *arg)
{
    while (1)
    {
        printf("Thread executed - 3 %d\n", cnt3++);
        if(cnt3 == 10000){
            printf("Thread Break - 3\n");
            break;
        }
        if(cnt1==10000 || cnt2==10000 || cnt3==10000)
            sleep(20);
    }
}
int main()
{
    pthread_t thread1, thread2, thread3;
    pthread_attr_t attr1, attr2, attr3;

    // Initialize thread attributes
    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);
    pthread_attr_init(&attr3);

    // Set scheduling policy to SCHED_FIFO (real-time)
    pthread_attr_setschedpolicy(&attr1, SCHED_RR);
    pthread_attr_setschedpolicy(&attr2, SCHED_RR);
    pthread_attr_setschedpolicy(&attr3, SCHED_RR);
    // Define priority values
    int priority1 = 1;
    int priority2 = 1;
    int priority3 = 99;

    // Set priority in the attributes
    struct sched_param param1, param2, param3;
    param1.sched_priority = priority1;
    param2.sched_priority = priority2;
    param3.sched_priority = priority3;

    // Modify the priority in the attributes for the last thread
    pthread_attr_setschedparam(&attr3, &param3);
    pthread_create(&thread3, &attr3, thread_function3, NULL);

    
    pthread_join(thread3, NULL);


    // Create threads with different priorities
    pthread_attr_setschedparam(&attr1, &param1);
    pthread_create(&thread1, &attr1, thread_function1, NULL);

    // Modify the priority in the attributes for the next thread
    pthread_attr_setschedparam(&attr2, &param2);
    pthread_create(&thread2, &attr2, thread_function2, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}
