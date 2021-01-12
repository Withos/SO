#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>


struct bufor{
	int mtype;
	int mvalue;
};

int *pam; 
#define MAX2 12
#define MAX 10
#define PELNY 2
#define PUSTY 1
#define zapis pam[MAX+1]
#define odczyt pam[MAX]

void waitSemafor(int semID, int number)
{
	printf("semID (kons)= %d\n", semID);
	struct sembuf operacje[1];
	operacje[0].sem_num = number;
	operacje[0].sem_op = -1;
	operacje[0].sem_flg = 0;
 
	if(semop(semID, operacje, 1) == -1)
	{
		printf("Nie udalo mi się zamknac semafora (kons) \n");
		exit(1);
	}
}

void signalSemafor(int semID, int number)
{
	struct sembuf operacje[1];
	operacje[0].sem_num = number;
	operacje[0].sem_op = 1;
	
	if(semop(semID, operacje, 1) == -1)
	{
		printf("Nie udalo mi sie otworzyc semafora (kons) \n");
		exit(1);
	}
}

int main()
{
   key_t klucz, kluczshm, kluczsem;
   int msgID, shmID, semID;
   int i;
   struct bufor komunikat;

   printf("konsument--------------------------------\n");


//uzyskanie dosepu do kolejki komunikatow
if ( (klucz = ftok("/lib/firmware/yamaha", 1)) == -1 )
{
	printf("Blad ftok (kons)\n");
	exit(1);
}
msgID=msgget(klucz,IPC_CREAT|0666); //tworzenie kk
if (msgID==-1)
	{printf("blad kolejki komunikatow\n"); exit(1);}
 
//uzyskanie dosepu do pamieci dzielonej
kluczshm=ftok("/lib/firmware/yamaha",2);
shmID=shmget(kluczshm, MAX2*sizeof(int), IPC_CREAT|0666);//tworzenie pam. dz.

//przylaczenie pam. dzielonej-- shmat   
pam = (int*)shmat(shmID, NULL, 0);

//Uzyskanie dostepu do semaforow
if ( (kluczsem=ftok("/lib/firmware/yamaha",3)) == -1 )
{
	printf("Blad ftok (prod)\n");
	exit(1);
}

printf ("kluczsem kons= %d\n", kluczsem);

if ( (semID = semget(kluczsem, 1, 0666)) == -1)
{
	printf("Blad semget (kons)");
	exit(1);
}


//msgrcv -- odbiór komunikatu 
//odbieranie/wysylanie odpowiednich komunikatow +
msgrcv(msgID, &komunikat, sizeof(komunikat.mvalue), PELNY, 0);
printf("KONSUMENT odebrano: %d\n", komunikat.mtype);

//sekcja krytyczna -- semafor -- operacje na pamięci dzielonej
waitSemafor(semID, 0);

// odczyt z bufora  elementu o  indeksie odczyt (pam. dzielona)
printf("KONSUMENT pam[%d]: %d\n", odczyt, pam[odczyt]);
signalSemafor(semID, 0);

komunikat.mtype=PUSTY;
msgsnd(msgID,&komunikat,sizeof(komunikat.mvalue),0);

//modyfikacja indeksu do odczytu
odczyt = (odczyt+1)%MAX;


exit(0);
}
