#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>


/*
Računati proste brojeve te preko signala periodički svake sekunde ispisivati status: zadnji pronađeni prosti broj. Koristiti setitimer sučelje za periodički alarm (vidjeti primjer periodičke obrade ispod). Na signal SIGINT (kombinacija Ctrl+C) privremeno zaustaviti rad na idućim brojevima (programski ostvariti zaustavljanje - umjesto provjere idućih prostih brojeva pozvati pause()), odnosno, nastaviti s radom ako je program prethodno bio zaustavljen. Na signal SIGTERM ispisati zadnji pronađeni prosti broj i završiti s radom. 
*/


int zadnji_prosti;
int pauza = 0;

int je_li_prost(int broj)
{
    int i;
    int korijen = sqrt(broj);
    for(i=2; i<korijen+1; i++)
        if(broj%i == 0)
            return 0;
    return 1;

}

void periodicki_posao ( int sig )
{
    printf ( "Zadnji prosti broj je: %d\n", zadnji_prosti);
}


void pauziraj(int sig)
{
    pauza = (pauza+1)%2;
    if(pauza == 1)
        printf("pauziram ...\n");
    else
        printf("nastavljam ...\n");

}

void terminiraj(int sig)
{
    printf("[SIGTERM] zadnji prosti broj = %d\n", zadnji_prosti);
    exit(0);
}

int main ()
{
    int broj=2;
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

	while (1)
	{
	    while(pauza == 1)
	        sleep(1);
        if(je_li_prost(broj))
            zadnji_prosti = broj;
        broj++;
	}
	return 0;
}


