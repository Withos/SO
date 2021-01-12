#include <stdio.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>


#define P 10   // liczba procesow prod i kons
#define MAX 10  //rozmiar buforu
#define MAX2 12 // + dwa pola na indeksy zapis/odczyt
#define PUSTY 1 //typ komunikatu
#define PELNY 2 //typ komunikatu

//struktura komunikatu
struct bufor{
	long mtype;
	int mvalue;
};

int main()
{
   key_t klucz, kluczshm, kluczsem;  //klucze do kk i pam. dz.
   int msgID; //id kolejki kom.
   int shmID;	//id pamieci dzielonej
   int semID; //id zbioru semaforow
   
   int i;
   struct bufor komunikat;
   
   if ( (klucz = ftok("/lib/firmware/yamaha", 1)) == -1 )
   {
      printf("Blad ftok (main)\n");
      exit(1);
   }
   

   msgID=msgget(klucz,IPC_CREAT | 0666); //tworzenie kk
   if (msgID==-1)
	{printf("blad kolejki komunikatow\n"); exit(1);}

   kluczsem=ftok("/lib/firmware/yamaha",3);
   semID= semget(kluczsem, 1, IPC_CREAT | 0666);
   if ( semID == -1)
   {
      perror("Blad semget ");
      exit(1);
   }
   if ( semctl(semID, 0, SETVAL, 1) == -1 )
   {
      perror("Blad semctl ");
      exit(1);
   }

   

   kluczshm=ftok("/lib/firmware/yamaha",2);
   shmID=shmget(kluczshm, MAX2*sizeof(int), IPC_CREAT | 0666);//tworzenie pam. dz.

   komunikat.mtype=PUSTY;
//   komunikat.mvalue=0;   wazny jest typ komunikatu, tresc - dowolna 
  
 for(i=0;i<MAX;i++)
	 {
	if( msgsnd(msgID,&komunikat,sizeof(komunikat.mvalue),0)==-1) //wyslanie komunikatu typu pusty
		{
			printf("blad wyslania kom. pustego\n");
			exit(1);
		};
	   printf("wyslany pusty komunikat %d\n",i);
	}
   	   
   for (i = 0; i < P; i++)
      switch (fork())
      {
         case -1:
            perror("Blad fork (mainprog)");
            exit(2);
         case 0:
            execl("./prod","prod", NULL);
      }

   for(i=0;i<P;i++)
	switch (fork())
	{
	case -1:
	
            printf("Blad fork (mainprog)\n");
	    exit(2);
	case 0:
		execl("./kons","kons",NULL);
	}

for(i=0;i<2*P;i++)
   wait(NULL);

//zwalnianie zasobow
   msgctl(msgID,IPC_RMID,NULL);
   shmctl(shmID,IPC_RMID, NULL);
   semctl(semID,1,IPC_RMID, NULL);
   printf("MAIN: Koniec.\n");
}