#include "unistd.h"
#include <iostream>
#include <list>
#include <cstdio>



/*
Simulirati postupak raspoređivanja dretvi u sustavu prema zadanom postupku (FIFO, prioritetno ili RR). Nije potrebno stvarati dretve radi simulacije obavljanja posla, dovoljno je ispisati kojim se redom one obavljaju (primjer ispisa je u nastavku).

Svojstva dretvi mogu se zadati na nekoliko načina (odabrati željeni):

    strukturom podataka (pogledati primjer);
    unositi za vrijeme rada ili
    načiniti generator koji će nasumično stvarati dretve (s određenim parametrima).

Što kad se nešto dogodi "istovremeno"? Npr. kraj jedne jedinice vremena poklapa se s dolaskom nova posla.
Preporuka: najprije "obraditi" kraj, a tek onda nove događaje. Npr. na kraju "sekunde" ažurirati broj dodijeljenih sekundi aktivnoj dretvi, napraviti odgovarajuće radnje, ovisno o raspoređivaču, a tek potom pogledati ima li pojave nove dretve (i akcije koje to nosi).
*/

#define MAX_DRETVI	5
//Vrijeme u sustavu
int vrijeme;


class dretva
{
public:
	int id;
	int preostalo_vrijeme;
	int prioritet;
	int rasporedivanje;

	dretva(int id, int p, int prio, int rasp)
	:id(id), preostalo_vrijeme(p), prioritet(prio), rasporedivanje(rasp) {}
	//~dretva();

};

//Lista pripravnih dretvi, na 0 mjestu je aktivna.
std::list<dretva> lista_dretvi;
std::list<dretva>::iterator it;


/* podaci o događajima pojave novih poslova - dretvi */
#define DRETVI	6
int nove[DRETVI][5] =
{
	/* trenutak dolaska, id, p, prio, rasp (prio i rasp se koriste ovisno o rasporedivacu) */
	{ 1,  3, 5, 3, 1 }, /* rasp = 0 => PRIO+FIFO; 1 => PRIO+RR */
	{ 3,  5, 6, 5, 1 },
	{ 7,  2, 3, 5, 0 },
	{ 12, 1, 5, 3, 0 },
	{ 20, 6, 3, 6, 1 },
	{ 20, 7, 4, 7, 1 },
};

void ubaci_u_listu(dretva nova)
{
	//Dodemo do mjesta na koje ubacujemo novu dretvu u listu pripravnih
	it = lista_dretvi.begin();
	while((*it).prioritet >= nova.prioritet)
		++it;

	lista_dretvi.insert(it, nova);

}

void ispis_zaglavlja()
{
	int i;
	printf ( "  t    AKT" );
		for ( i = 1; i < MAX_DRETVI; i++ )
			printf ( "     PR%d", i );
		printf ( "\n" );
}

void ispis_stanja()
{
	int i;
	it = lista_dretvi.begin();

	printf ( "%3d ", vrijeme );
	for ( i = 0; i < MAX_DRETVI; i++ )
		if ( it != lista_dretvi.end() )
		{
			printf ( "  %d/%d/%d ", (*it).id, (*it).prioritet, (*it).preostalo_vrijeme );
			it++;
		}
		else
			printf ( "  -/-/- " );
	printf ( "\n");
}

void obrada_aktivne()
{
	it = lista_dretvi.begin();
	if(it != lista_dretvi.end())
	{
		(*it).preostalo_vrijeme--;
		if((*it).preostalo_vrijeme == 0)
		{	
			printf("Dretva %d je zavrsila.\n", (*it).id);
			lista_dretvi.pop_front();
		}
	}

}

void dolazak_nove(int *na_redu)
{
	while(vrijeme == nove[(*na_redu)][0])
	{
		dretva nova(nove[(*na_redu)][1], nove[(*na_redu)][2],
			nove[(*na_redu)][3], nove[(*na_redu)][4]);
		ubaci_u_listu(nova);
		
		printf("%3d -- nova dretva id=%d, p=%d, prio=%d\n", vrijeme,
			nove[(*na_redu)][1], nove[(*na_redu)][2], nove[(*na_redu)][3]);
		
		ispis_stanja();

		(*na_redu)++;
	}
}

int main ()
{	//idejno
	vrijeme = 0;
	int posljednja_nova = 0;

	ispis_zaglavlja();
	
	while(1)
	{
		ispis_stanja();
		dolazak_nove(&posljednja_nova);
		obrada_aktivne();

		sleep(1);
		vrijeme++;
	}
}
