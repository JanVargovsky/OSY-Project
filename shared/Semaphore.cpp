#include "Semaphore.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/sem.h>
#include <stdlib.h>

typedef struct sembuf sembuf;

Semaphore::Semaphore(int key, int value) : key(key)
{
    this->id = semget(this->key, 1, IPC_CREAT | 0600);

    if(this->id == -1)
    {
    	printf("Vytvareni semaforu %d selhalo!", key);
    	exit(1);
    }

    if(semctl(id, 0, SETVAL, value) == -1)
    {
    	printf("Nastaveni pocatecni hodnoty %d selhalo!", key);
    	exit(1);
    }

    this->initialized = true;
}

Semaphore::~Semaphore()
{
    semctl(this->id, 0, IPC_RMID);
}

bool Semaphore::Up()
{
	if(!this->initialized)
	{
		printf("Pouzivani neinicializovaneho semaforu %d", this->key);
		return false;
	}

	sembuf sop;
	sop.sem_num = 0;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	
	return semop(this->id, &sop, 1) == 0;
}

bool Semaphore::Down()
{
	if(!this->initialized)
	{
		printf("Pouzivani neinicializovaneho semaforu %d", this->key);
		return false;
	}

	sembuf sop;
	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	
	return semop(this->id, &sop, 1) == 0;
}
