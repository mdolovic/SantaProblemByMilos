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
static sem_t santaSem;
static sem_t reindeerSem;
static sem_t elfSem;
static sem_t mutex;

void *SantaClaus(void *arg)
{
	printf("Santa Claus: Hoho, here I am\n");
	while (true)
	{
		sem_wait(&santaSem);
		sem_wait(&mutex);
		if (reindeer == MAX_REINDEERS)
		{
			printf("Santa Claus: preparing sleigh\n");
			for (int i = 0; i < MAX_REINDEERS; i++)
				sem_post(&reindeerSem);
			printf("Santa Claus: presents will reach kids\n");
			reindeer = 0;
		}
		else if (elves == 3)
		{
			printf("Santa Claus: helping elves\n");
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
		if (reindeer == MAX_REINDEERS)
			sem_post(&santaSem);
		sem_post(&mutex);
		sem_wait(&reindeerSem);
		printf("Reindeer number %d getting hitched\n", id);
		sleep(20);
	}
	return arg;
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

			printf("Elf number %d will get help from Santa Claus\n", id);
			sleep(10);

			sem_wait(&mutex);
			elves--;
			if (elves == 0)
				sem_post(&elfSem);
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
	sem_init(&santaSem, 0, 0);
	sem_init(&reindeerSem, 0, 0);
	sem_init(&elfSem, 0, 1);
	sem_init(&mutex, 0, 1);

	pthread_t *santa_claus = CreateThread(SantaClaus, 0);

	pthread_t *reindeers[MAX_REINDEERS];
	for (int i = 0; i < MAX_REINDEERS; i++)
		reindeers[i] = CreateThread(Reindeer, (void *)i + 1);

	pthread_t *elves[MAX_ELVES];
	for (int j = 0; j < MAX_ELVES; j++)
		elves[j] = CreateThread(Elf, (void *)j + 1);

	int ret = pthread_join(*santa_claus, NULL);
	assert(ret == 0);
}
