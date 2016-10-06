#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*
Ostvariti program koji simulira tijek rezervacije stolova u nekom restoranu.
Program na početku treba stvoriti određeni broj dretvi koji se zadaje u naredbenom retku.
Svaka dretva nakon isteka jedne sekunde (van kritična odsječka) provjerava ima li slobodnih stolova te slučajno odabire jedan od njih. Nakon odabira, dretva ulazi u kritični odsječak te ponovo provjerava je li odabrani stol slobodan. Ako jest, označava stol zauzetim i izlazi iz kritičnog odsječka. U oba slučaja, nakon obavljene operacije (još uvijek unutar kritična odsječka) ispisuje trenutno stanje svih stolova te podatke o obavljenoj rezervaciji. Prilikom ispisa za svaki stol mora biti vidljivo je li slobodan ili broj dretvea koja je taj stol rezervirala.
Broj stolova se također zadaje u naredbenom retku.
Svaka dretva ponavlja isti postupak sve dok više nema slobodnih stolova. Program završava kada sve dretve završe.
*/





//Varijabla koja govori koliko jos ima slobodnih mjesta na stolu.
//Kad vrijednos dode na 0 dretve prestaju s radom.
int broj_slobodnih_mjesta;
//Varijabla koja sadrzi broj stolova koje treba rezervirati.
int broj_stolova;
//Varijabla koja oznacava koliko imamo dretvi.
int broj_dretvi;
//Niz brojeva koji oznacava trenutno zauzece stolova.
//Ako stol nije zauzet ima znak - inace je broj dretve koja ga je zauzela.
char *stolovi;
//Pomocne varijable za Lamportov algoritam.
int *broj, *ulaz, zadnji;


//Funkcija koja ispisuje trenutno stanje na stolovima.
void ispisi_stolove()
{
    /*
    int i;
    for(i = 0; i < broj_stolova; i++)
        printf("%c", stolovi[i]);
    printf("\n");*/
    printf("%s\n", stolovi);
}


//Funkcija koja vraca broj koji oznacava jedan nasumican ne rezerviran stol.
int dodjeli_random_stol()
{
    int i, j=0;
    int slobodni[broj_slobodnih_mjesta];
    for(i = 0; i < broj_stolova; i++)
        if(stolovi[i] == '-')
            slobodni[j++] = i;

    return slobodni[rand() % broj_slobodnih_mjesta];
}

//Funkcija ulaska u kriticni odsjecak, koristi se Lamportov (pekarski) algoritam
void udi_u_KO(int indeks)
{
    int j;
    ulaz[indeks] = 1;
    zadnji++;
    broj[indeks] = zadnji;
    ulaz[indeks] = 0;

    for(j = 0; j < broj_dretvi; j++)
    {
        while(ulaz[j] == 1);
        while(broj[j] != 0 && ( broj[j] < broj[indeks] ||
                                ( broj[j] == broj[indeks] && j < indeks ) ) );
    }

}

//Funkcija za izlazak iz kriticnog odsjecka.
void izadi_iz_KO(int indeks)
{
    broj[indeks]= 0;
}

//Funkcija koja provjerava je li stol na kojem zelimo napravit slobodan.
//Ako je stol slobodan, rezervira ga te ispise poruku o rezervaciji, inace
//napise poruku da je stol vec zauzet. Na kraju funkcije ispisuje se stanje stolova.
//Funkcija prima 2 parametra, broj dretve koja zeli rezervirati stol i broj stola
//koji funkcija zeli rezervirati.
void izvrsi_rezervaciju(int indeks, int zeljeni_stol)
{
    if(stolovi[zeljeni_stol] == '-') {
        stolovi[zeljeni_stol] = indeks + 48;
        broj_slobodnih_mjesta--;
        printf("Dretva %d: rezerviram stol %d, stanje:\n", indeks, zeljeni_stol);
    }
    else
        printf("Dretva %d: neuspjela rezervacija stola %d, stanje:\n", indeks, zeljeni_stol);

    ispisi_stolove();
}


//Funkcija koju vrti svaka dretva zasebno dok nisu sva mjesta na stolu zauzeta.
//Funkcija se brini o tome koja dretva zeli rezervirati koji stol, te pazi da
//ne dode do kolizije.
void* funkcija(void *indeks)
{
    int stol_koji_zelim_rezervirati;
    int moj_broj = *((int*) indeks);
    while(1)
    {
        sleep(1);
        if(broj_slobodnih_mjesta < 1)
            break;
        stol_koji_zelim_rezervirati = dodjeli_random_stol();
        printf("Dretva %d: odabirem stol %d.\n", moj_broj, stol_koji_zelim_rezervirati);
        udi_u_KO(moj_broj);
        izvrsi_rezervaciju(moj_broj, stol_koji_zelim_rezervirati);
        izadi_iz_KO(moj_broj);

    }

}

//Main prima dva argumenta, prvi je broj dretvi, a drugi broj stolova.
//Nakon pozivanja main inicijalizira sve vrijednosti i stvori dretve.
//Main ceka da sve dretve prestanu s radom, te onda i on zavrsava.
//Program radi ispravno samo do ukljucivo 10 dretvi. Vise od toga nema smisla jer
//onda ne bi znali koja je dretva rezervirala koji stol.
int main(int argc, char** argv)
{
    if(argc != 3)
    {
        printf("Program %s zahtjeve 2 int kao argumente. ", argv[0]);
        printf("Prvi je broj dretvi, a drugi broj stolova\n");
        exit(1);
    }

    int i;
    broj_dretvi = atoi(argv[1]);
    int indeks_dretve[broj_dretvi];

    broj_slobodnih_mjesta = broj_stolova = atoi(argv[2]);

    stolovi = (char*)malloc(broj_stolova * sizeof(char));
    for(i = 0; i < broj_stolova; i++)
        stolovi[i] = '-';

    ulaz = (int*)malloc(broj_dretvi * sizeof(int));
    broj = (int*)malloc(broj_dretvi * sizeof(int));
    zadnji = 0;


    pthread_t thr_id[broj_dretvi];

    for(i = 0; i < broj_dretvi; i++)
    {
        indeks_dretve[i] = i;
        if(pthread_create(&thr_id[i], NULL, funkcija, &indeks_dretve[i]) != 0)
        {
            printf("Greska pri stvaranju dretve!\n");
            exit(1);
        }
    }

    for(i = 0; i < broj_dretvi; i++)
    {
        pthread_join(thr_id[i], NULL);
    }

    return 0;
}

