#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>


/*
Ostvariti komunikaciju između dretvi/procesa proizvođača i potrošača korištenjem međuspremnika ograničene duljine. Na osnovu broja parametara komandne linije, početna dretva/proces stvara odgovarajući broj dretvi/procesa proizvođača te jednog potrošača. Svaki proizvođač preuzima jedan niz znakova zadan iz komandne linije i šalje preko međuspremnika znak po znak potrošaču (prema primjeru pokretanja programa pri dnu stranice). Potrošač prima znak po znak te kad je primio sve znakove (uključujući i znakove za kraj niza znakova '\0' koje su mu poslali proizvođači) ispisuje sve primljene znakove te završava s radom.

Riješiti problem proizvođača i potrošača uporabom semafora (i dretvi).
*/


//Pomocne varijable koje nam sluze za to da znamo na koje mjesto u zajednicki niz pisati/citati.
int ULAZ = 0, IZLAZ = 0;
//Zajednicki niz u koji sve dretve pisu/citaju.
char m[5];
//3 semafora za kontrolu izvrsavanja dretvi.
sem_t PISI, PUN, PRAZAN;

//Struktura podataka koju saljemo dretvama da znaju koji string moraju citati i koje su po redu.
typedef struct _thr_struct{
	int indeks;
	char *string;
} thr_struct;

//Funkcija koju izvrsavaju dretve koje salju znak po znak iz dobivenog stringa.
void* proizvodac(void *arg)
{
    int indeks = ( (thr_struct*) arg )->indeks;
    char *moj_string = ( (thr_struct*) arg )->string;

    int i=0;
    while(1)
    {
    	sleep(1);
    	sem_wait(&PUN);
    	sem_wait(&PISI);
    	printf("PORIZVODAC%d -> %c\n", indeks, moj_string[i]);
    	m[ULAZ] = moj_string[i];
    	ULAZ = (ULAZ+1)%5;
    	sem_post(&PISI);
    	sem_post(&PRAZAN);

    	if(moj_string[i] == '\0')
    		break;
    	i++;
    }
}

//Funkcija koju pokrece dretva koja cita znakove u zajednickom polju.
//Funkcija cita sve znakove ta zavrsni ispis ispisuje znak po znak da pazi na oznaku
//kraja stringa kojeg ima spremljenog.
void* potrosac(void *broj)
{
	int broj_proizvodaca = *((int*) broj) - 1;
	int i=0;
	char* s;
	int velicina_polja = broj_proizvodaca*2;
	s = (char*)malloc(velicina_polja * sizeof(char));
	
	while(broj_proizvodaca!=0)
	{
		if(velicina_polja == i)
		{
			velicina_polja *= 2;
			s = (char*)realloc(s, velicina_polja * sizeof(char));
		}
		sem_wait(&PRAZAN);
		s[i] = m[IZLAZ];
		IZLAZ = (IZLAZ+1)%5;
		printf("POTROSAC <- %c\n", s[i]);
		sem_post(&PUN);
		if(s[i] == '\0')
			broj_proizvodaca--;
	
		i++;
	}

	printf("Primljeno je: ");
	int j;
	for(j=0; j<i; j++)
		if(s[j] != '\0')
			printf("%c", s[j]);
	printf("\n");
	free(s);
}

//Inicijaliziramo semafore
void inicijalizacija_semafora()
{
	//Prazan na 0 jer on oznacava koliko ima novih znakova u zajednickom polju.
	sem_init(&PRAZAN, 0, 0);
	//Pun na 5 je on oznacava da li ima slobodnih mjesta u zajednickom polju.
	sem_init(&PUN, 0, 5);
	//Pisi na 1 jer on oznacava kriticni odsjecak za pisanje u zajednicko polje.
	sem_init(&PISI, 0, 1);
}


//Konstruira dretvu za svaki dobiveni niz te joj pridruzi string za obradu i indeks.
//Konstruira dretvu koja ce citati iz zajednickog niza.
//Ceka zavrsetak izvodenja svih dretvi.
int main(int argc, char **argv)
{
	
	inicijalizacija_semafora();

	pthread_t thr_id[argc];
	thr_struct parametri_za_slanje[argc-1];

	int i;
    for(i = 0; i < argc-1; i++)
    {
    	parametri_za_slanje[i].indeks = i+1;
    	parametri_za_slanje[i].string = argv[i+1];

        if(pthread_create(&thr_id[i], NULL, proizvodac, (void*)&parametri_za_slanje[i]) != 0)
        {
            printf("Greska pri stvaranju dretve!\n");
            exit(1);
        }
    }

    if(pthread_create(&thr_id[argc-1], NULL, potrosac, &argc) != 0)
        {
            printf("Greska pri stvaranju dretve!\n");
            exit(1);
        }

    for(i = 0; i < argc; i++)
    {
        pthread_join(thr_id[i], NULL);
    }

	return 0;
}


