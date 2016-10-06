#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>


/*
Proširiti zadatak iz prve vježbe tako da se prosti brojevi traže korištenjem više dretvi.
Sinkronizaciju dretvi prilikom pristupanja zajedničkim varijablama obaviti Lamportovim algoritmom međusobnog isključivanja.
*/

//Pomocna varijabla koja odlucuje da li radimo cekanje ili ne.
int pauza = 0;
//Zadnji prosti broj kojeg smo nasli.
int zadnji_prosti;
//Broj koji ulazi u test za provjeru je li prost.
int iduci_broj_za_testiranje = 2;
//Niz dretvi.
pthread_t *thr_id;
//Ukupna broj dretvi.
int broj_dretvi;
//Pomocne varijable za Lamportov algoritam.
int *broj, *ulaz, zadnji;

//Pomocna funkcija koja provjerava je li broj dobiven
//kao argument prost. Ako je vraca 1, inace 0.
int je_li_prost(int broj)
{
    int i;
    int korijen = sqrt(broj);
    for(i=2; i<korijen+1; i++)
        if(broj%i == 0)
            return 0;
    return 1;

}

//Definicija SIGALRM-a, ispisuje zadnji pronedeni prosti broj.
void periodicki_posao ( int sig )
{
    printf ( "Zadnji prosti broj je: %d\n", zadnji_prosti);
}

//Definicija SIGINT-a, postavlja varijablu pauza na 1, odnosno 0
//te ispisuje odgovarajucu poruku.
//Ako se pauza prekida svakoj dretvi se salje SIGCHLD da prekine pause()
void pauziraj(int sig)
{
    pauza = 1 - pauza;
    if(pauza == 1)
        printf("pauziram ...\n");
    else
    {
        int i;
        printf("nastavljam ...\n");
        for( i = 0; i < broj_dretvi; ++i )
			pthread_kill(thr_id[i], SIGCHLD);
    }

}

//Definicija SIGTERM-a, ispisuje zadnji pronadeni prosti broj, te ubija program.
void terminiraj(int sig)
{
    printf("[SIGTERM] zadnji prosti broj = %d\n", zadnji_prosti);
    exit(0);
}

//Definicija SIGCHLD signala, sluzi za budenje dretve iz pauze.
void pokreni_se( int sig ){

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

//Funkcija koja pristupa globalnoj varijabli koja ima spremljen
//zadnji pronadeni prosti broj. Ako je njezin prosti broj veci od
//te vrijednosti, postavi novi prosti broj.
//Ova se funkcija u dretvama smije pozivati samo u kriticnom odsjecku
//jer pristupa i mijenja globalne varijable.
void postavi_novi_prosti(int novi)
{
    if(novi > zadnji_prosti)
        zadnji_prosti = novi;
}

//Funkcija koja dohvaca iduci broj za provjeru te povecava vrijednost za
//azurira broj koji slijedeci moramo provjeriti.
//Ova se funkcija u dretvama smije pozivati samo u kriticnom odsjecku
//jer pristupa i mijenja globalne varijable.
int dohvati_i_povecaj()
{
    return iduci_broj_za_testiranje++;
}


//Funkcija koju svaka dretva poziva.
//Funkcija se vrti u beskonacno i samo SIGTERM ili SIGKILL zavrsavaju
//Ako je pomocna varijabla pauza = 1 dolazi do pauziranja izvodenja.
//Kad zelimo prekinuti pauzu dretvi saljemo signal SIGCHLD.
//Funkcija dohvaca iduci broj za obradu, obradi ga (provjeri je li prost),
//te po potrebi azurira globalnu varijablu.
void *funkcija(void *indx)
{
	sigset( SIGCHLD, pokreni_se );
	int indeks = *((int*) indx);

    int kopija_glob_broja;
	while(1)
	{
		if(pauza)
			pause();

		udi_u_KO(indeks);
        kopija_glob_broja = dohvati_i_povecaj();
		izadi_iz_KO(indeks);

        if(je_li_prost(kopija_glob_broja))
        {
            udi_u_KO(indeks);
            postavi_novi_prosti(kopija_glob_broja);
            izadi_iz_KO(indeks);
        }

	}

}


//Main prima jedan argument, broj dretvi.
//Nakon pozivanja main inicijalizira sve vrijednosti potrebne za
//Lamportov algoritam redefinira signale i stvori dretve.
int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("Program %s zahtjeve 1 int koji oznacava broj dretvi.\n", argv[0]);
        exit(1);
    }

    struct itimerval t;

	/* povezivanje obrade signala SIGALRM sa funkcijom "periodicki_posao" */
	sigset ( SIGALRM, periodicki_posao );
	sigset ( SIGINT, pauziraj);
	sigset ( SIGTERM, terminiraj);

	/* definiranje periodičkog slanja signala */
	/* prvi puta nakon: */
	t.it_value.tv_sec = 1;
	t.it_value.tv_usec = 0;
	/* nakon prvog puta, periodicki sa periodom: */
	t.it_interval.tv_sec = 1;
	t.it_interval.tv_usec = 0;


	/* pokretanje sata s pridruženim slanjem signala prema "t" */
	setitimer ( ITIMER_REAL, &t, NULL );
    int i;

    broj_dretvi = atoi(argv[1]);
    int indeks_dretve[broj_dretvi];
    thr_id = (pthread_t*)malloc(broj_dretvi * sizeof(pthread_t));

    ulaz = (int*)malloc(broj_dretvi * sizeof(int));
    broj = (int*)malloc(broj_dretvi * sizeof(int));
    zadnji = 0;



    for(i = 0; i < broj_dretvi; i++)
    {
        indeks_dretve[i] = i;
        if(pthread_create(&thr_id[i], NULL, funkcija, &indeks_dretve[i]) != 0)
        {
            printf("Greska pri stvaranju dretve!\n");
            exit(1);
        }
    }

    //Ovjde ne bi smjelo doci, prog mora umrijeti prije toga.
    for(i = 0; i < broj_dretvi; i++)
    {
        pthread_join(thr_id[i], NULL);
    }

    return 0;
}


