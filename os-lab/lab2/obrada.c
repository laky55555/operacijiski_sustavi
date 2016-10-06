#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>


/*
Napisati dva programa:

  1.  program koji omogućava obradu prekida s više razina/prioriteta
  2.  program koji generira signale i šalje ih prvom procesu

Svaki program pokrenuti u svojoj ljusci.
Odabrati pet različitih signala (npr. SIGUSR1, SIGUSR2, SIGQUIT, SIGTSTP, SIGINT) kojima će se simulirati zahtjevi za prekid različitog prioriteta. Neka SIGINT ima najveći prioritet (da se može izravno poslati sa Ctrl+C).

Prvi program treba po primitku signala odrediti prioritet tog signala, te obzirom na njegov prioritet i prioritet trenutna posla ili odmah započeti s njegovom obradom ili samo zabilježiti da se on pojavio (za kasniju obradu) a sada se vratiti (nastaviti) s trenutnim prioritetnijim obradama. Pamtiti i obraditi sve zahtjeve, čak i ako zahtjevi istog tipa još uvijek čekaju na početak obrade.

Drugi program povremeno, svakih nekoliko sekundi (2-7), nasumice odabire jedan od signala (odabranih u prvom programu) te ga šalje prvom programu. Drugi program na primitak signala SIGINT najprije šalje signal SIGKILL prvom programu te potom završava s radom. 
*/



int trenutni_prioritet = -1;
int polje_za_obradit[] = {0, 0, 0, 0, 0};
int polje_sa_signalima[] = {SIGQUIT, SIGUSR1, SIGUSR2, SIGTSTP, SIGINT};
char polje_sa_stringovima[5][15] = {"- %c - - - -\n",
     "- - %c - - -\n", "- - - %c - -\n", "- - - - %c -\n", "- - - - - %c\n"};

int imam_li_nesto_za_obradit(int signal);

void ispis(char* string, int bivsi)
{
    int i;
    printf(string, 'P');
    for(i=0; i<5; i++)
    {
        sleep(1);
        printf(string, i+1+'0');
    }
    printf(string, 'K');
    sleep(1);

    //zavrsli smo s obradom, prioritet se vraca na prijasnji
    trenutni_prioritet = bivsi;
    imam_li_nesto_za_obradit(trenutni_prioritet);
}


int imam_li_nesto_za_obradit(int prioritet)
{
    int i, prijasnji_prioritet, obradujem=0;
    for(i=4; i>prioritet; i--)
    {
        if(polje_za_obradit[i] > 0)
        {
            obradujem++;
            polje_za_obradit[i]--;
            prijasnji_prioritet = trenutni_prioritet;
            trenutni_prioritet = i;
            ispis(polje_sa_stringovima[i], prijasnji_prioritet);
            break;
        }

    }

    return obradujem;
}

void obradi_signal(int sig)
{
    //signal je u rasoponu od 0-4 ovisno o tome koji signal je dosao
    int signal, prijasnji_prioritet;
    for(signal=0; signal<5; signal++)
        if(sig == polje_sa_signalima[signal])
            break;

    printf(polje_sa_stringovima[signal], 'X');
    sigrelse(sig);

    if(trenutni_prioritet < signal)
    {
        prijasnji_prioritet = trenutni_prioritet;
        trenutni_prioritet = signal;
        ispis(polje_sa_stringovima[signal], prijasnji_prioritet);
    }
    else
        polje_za_obradit[signal]++;
}

int main ()
{
    ///prioritet 1
	sigset ( SIGUSR1, obradi_signal);
	///prioritet 2
	sigset ( SIGUSR2, obradi_signal);
	///prioritet 3
	sigset ( SIGQUIT, obradi_signal);
	///prioritet 4
	sigset ( SIGTSTP, obradi_signal);
	///prioritet 5
	sigset ( SIGINT, obradi_signal);

	int j, brojac;
	for(j=0; j<5; j++)
        printf("%d  ", polje_sa_signalima[j]);
    printf("\n");

    int i;
    printf("Proces obrade prekida, PID=%d\n", getpid());
	printf("G 1 2 3 4 5\n-----------\n");
	while (1)
	{
        if(imam_li_nesto_za_obradit(-1) == 0)
            printf("%d - - - - -\n", brojac++);
        sleep(1);
    }
	return 0;
}



