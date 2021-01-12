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

#define MAX 10
#define MAX2 12
#define PELNY 2
#define PUSTY 1
#define odczyt pam[MAX]
#define zapis pam[MAX+1]

void waitSemafor(int semID, int number)
{
	printf("semID (prod)= %d\n", semID);
	struct sembuf operacje[1];
	operacje[0].sem_num = number;
	operacje[0].sem_op = -1;
	operacje[0].sem_flg = 0;
 
	if(semop(semID, operacje, 1) == -1)
	{
		printf("Nie udalo mi się zamknac semafora (prod)\n");
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
		printf("Nie udalo mi sie otworzyc semafora (prod)\n");
		exit(1);
	}
}

int main()
{
   key_t klucz, kluczshm, kluczsem;
   int msgID;
   int shmID, semID;
   int i; // to, co ma trafic do bufora 
   time_t czas;   
   struct bufor komunikat;
   pid_t pidNr;

printf("producent--------------------------------\n");

//uzyskanie dostepu do kolejki komunikatow
if ( (klucz = ftok("/lib/firmware/yamaha", 1)) == -1 )
{
	printf("Blad ftok (prod)\n");
	exit(1);
}
msgID=msgget(klucz,IPC_CREAT|0666); //tworzenie kk
if (msgID==-1)
	{printf("blad kolejki komunikatow (prod)\n"); exit(1);}

//uzyskanie dostepu do pamieci dzielonej
if ( (kluczshm=ftok("/lib/firmware/yamaha",2)) == -1 )
{
	printf("Blad ftok (prod)\n");
	exit(1);
}
shmID=shmget(kluczshm, MAX2*sizeof(int), IPC_CREAT|0666);//tworzenie pam. dz.

//przylaczenie pamieci dzielonej
pam = (int*)shmat(shmID, NULL, 0);

//Uzyskanie dostepu do semaforow
if ( (kluczsem=ftok("/lib/firmware/yamaha",3)) == -1 )
{
	printf("Blad ftok (prod)\n");
	exit(1);
}
if ( (semID = semget(kluczsem, 1, IPC_CREAT | 0666)) == -1)
{
	printf("Blad semget (prod)");
	exit(1);
}

//wysylanie/odbieranie odpowiednich komunikatow
msgrcv(msgID, &komunikat, sizeof(komunikat.mvalue), PUSTY, 0);

//operacje na pamieci dzielonej w sekcji krytycznej -- semafory
waitSemafor(semID, 0);
	
//produkcja - dodanie rekordu do puli buforow  pod indeks - zapis  -- getpid()
pidNr=getpid();
pam[zapis] = pidNr;
printf("PRODUCENT pam[%d]: %d\n", zapis, pam[zapis]);
signalSemafor(semID, 0);

//wyslanie odpowiedniego komunikatu
komunikat.mtype=PELNY;
msgsnd(msgID,&komunikat,sizeof(komunikat.mvalue),0);
 
//modyfikacja indeksu do zapisu
zapis = (zapis+1)%MAX;


exit(0);
}