#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>

pthread_t *CreateThread(void *(*f)(void *), void *a)
{
	pthread_t *t = malloc(sizeof(pthread_t));
	assert(t != NULL);
	int ret = pthread_create(t, NULL, f, a);
	assert(ret == 0);
	return t;
}

static const int MAX_ELVES = 10;
static const int MAX_REINDEERS = 9;

static int elves;
static int reindeer;
static int childrenCnt;
static sem_t santaSem;
static sem_t reindeerSem;
static sem_t elfSem;
static sem_t mutex;
static sem_t santaSem2;
static sem_t childSem;
static sem_t childSem2;
static int helpCounter = 0;
static int MAX_HELPED_ELVES = 3;
static int elvesHelped[3];       
// static int children[5]; 
static int elvesHelpedCount = 0;
static int lastReindeer = 0;

void *SantaClaus(void *arg)
{
	printf("Santa Claus: Hoho, here I am\n");
	while (true)
	{
		sem_wait(&santaSem);
		sem_wait(&mutex);
		if (reindeer == MAX_REINDEERS)
		{
      sem_post(&mutex);
      sem_wait(&santaSem2);
      sem_wait(&mutex);
      printf("Santa Claus: preparing sleigh\n");
			for (int i = 0; i < MAX_REINDEERS; i++)
				sem_post(&reindeerSem);
			printf("Santa Claus: presents will reach kids\n");
      
			reindeer = 0;
		}
		else if (elves == 3)
		{
			printf("Santa Claus: helping elves");
      for (int i = 0; i < elvesHelpedCount; i++) {
        printf(" %d", elvesHelped[i]);
      }
      printf(".\n");
      elvesHelpedCount = 0;
		}
		sem_post(&mutex);
	}
	return arg;
}

void *Reindeer(void *arg)
{
	int id = (int)arg;
	printf("This is reindeer %d\n", id);
	while (true)
	{
		sem_wait(&mutex);
		reindeer++;
		if (reindeer == MAX_REINDEERS){
			sem_post(&santaSem);
      lastReindeer = id;
    }
		sem_post(&mutex);
		sem_wait(&reindeerSem);
		printf("Reindeer number %d getting hitched\n", id);
    if (id == lastReindeer){
      for (int i = 0; i < 5; i++){
        sem_post(&childSem2);
      }
    }
		sleep(20);
	}
	return arg;
}

void *Child(void *arg){

  int id = (int)arg; 
  printf("This is child %d\n", id);
  while (true){
      sem_wait(&childSem);
      sem_wait(&mutex);
      childrenCnt++;
      if (childrenCnt == 5){
          printf("Santa shall give gifts\n");
          sem_post(&santaSem2);
      }
      else
          sem_post(&childSem);
      sem_post(&mutex);

      sem_wait(&childSem2);
      printf("Child number %d will get present from Santa Claus\n", id);
      sleep(10);

      sem_wait(&mutex);
      childrenCnt--;
      if(childrenCnt == 0)
        sem_post(&childSem);
      sem_post(&mutex);
    }
}


void *Elf(void *arg)
{
    int id = (int)arg; 
    printf("This is elf %d\n", id);
    while (true)
    {
        bool help = random() % 100 < 10;
        if (help)
        {
            sem_wait(&elfSem);
            sem_wait(&mutex);
            elves++;
            if (elves == 3)
                sem_post(&santaSem);
            else
                sem_post(&elfSem);
            sem_post(&mutex);

            helpCounter++; 
            if (elvesHelpedCount < MAX_HELPED_ELVES)
                elvesHelped[elvesHelpedCount++] = id;

            printf("Elf number %d will get help from Santa Claus, help number %d\n", id, helpCounter);
            sleep(10);

            sem_wait(&mutex);
            elves--;
            if (elves == 0)
            {
                sem_post(&elfSem);
                helpCounter = 0;
            }
            sem_post(&mutex);
        }
        printf("Elf number %d is at work\n", id);
        sleep(2 + random() % 5);
    }
    return arg;
}




int main(int ac, char **av)
{
	elves = 0;
	reindeer = 0;
  childrenCnt = 0;
	sem_init(&santaSem, 0, 0);
	sem_init(&santaSem2, 0, 0);
	sem_init(&reindeerSem, 0, 0);
	sem_init(&elfSem, 0, 1);
	sem_init(&mutex, 0, 1);
	sem_init(&childSem, 0, 1);
	sem_init(&childSem2, 0, 0);


	pthread_t *santa_claus = CreateThread(SantaClaus, 0);

	pthread_t *reindeers[MAX_REINDEERS];
	for (int i = 0; i < MAX_REINDEERS; i++)
		reindeers[i] = CreateThread(Reindeer, (void *)i + 1);

	pthread_t *elves[MAX_ELVES];
	for (int j = 0; j < MAX_ELVES; j++)
		elves[j] = CreateThread(Elf, (void *)j + 1);


  pthread_t *children[10];
	for (int j = 0; j < 10; j++)
		children[j] = CreateThread(Child, (void *)j + 1);

	int ret = pthread_join(*santa_claus, NULL);
	assert(ret == 0);
}