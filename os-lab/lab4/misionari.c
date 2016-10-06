#include <pthread.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

/*
Rješiti problem prijevoza kanibala i misionara. Na obali neke široke rijeke postoji čamac koji prevozi kanibale i misionare na drugu stranu obale. Kapacitet čamca je 7 putnika. U čamcu moraju biti najmanje 3 putnika da on može krenuti. U čamcu ne smije biti više kanibala od misionara, dok su sve ostale kombinacije putnika dozvoljene. Kanibali i misionari dolaze sa obje strane rijeke. Jedan misionar dolazi svake dvije sekunde, a kanibal svake sekunde (odabir obale je slučajan). Nakon što pređu preko rijeke putnici odlaze dalje (nema ih više u sustavu). U sustavu neka postoji samo jedan čamac, a svaki misionar i kanibal predstavljaju po jednu dretvu/proces. Čamac je također jedna dretva/proces koji pri svakom prijelazu ispisuje koga prevozi (npr. "Prevezeni: misionar, kanibal, misionar, misionar"). Pretpostaviti da je čamac u početku na desnoj obali. Nakon što se u čamcu popune tri (ili više) mjesta, čamac pričeka još sekundu i kreće preko rijeke što traje dvije sekunde. Dretve/procese misionare i kanibale stvara pomoćna dretva/proces (npr. početna dretva). Ispravno sinkronizirati dretve/procese kanibale, misionare i čamac korištenjem semafora ili monitora. 
*/


//Struktura koja na pomaze prlikom definranja dretve.
struct Covjek{
	int id;
	//0 ako je desno, 1 ako je lijevo
	int obala;
	//prehrana je 0 ako je misionar, 1 ako je kanibal
	int prehrana;
};

//Inicijalizacija monitora i redova za cekanje.
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
//Red za cekanje na desnoj obali.
pthread_cond_t desna = PTHREAD_COND_INITIALIZER;
//Red za cekanje na lijevoj obali.
pthread_cond_t lijeva = PTHREAD_COND_INITIALIZER;
//Camac ceka 1s prije kretanja.
pthread_cond_t camac = PTHREAD_COND_INITIALIZER;
//Pomocna varijabla za dohvacanje pravog reda za cekanje.
pthread_cond_t *red_a[2] = { &desna, &lijeva };


///Pomocne varijable, sluze za provjeru koliko je ukrcavanje dobro.
/*//Broj misionara na desnoj, pa lijevoj obali.
int misionari[2] = {0,0};
//Broj kanibala na desnoj, pa lijevoj obali.
int kanibali[2] = {0,0};*/

//Brod 0 je ako je brod na desnoj strani, 1 ako je na lijevoj.
int brod_na_obali = 0;
//Broj misionara, pa broj kanibala u brodu.
int putnici[2] = {0,0};
//0 je desna obala, 1 lijeva.
char *strana_obale[2] = {"desnoj", "lijevoj"};


//Funkcija koja oponasa jednu osobu. Svaka osoba se pokusa sto 
//prije ukrcati na camac ako joj to uvijeti dopustaju, te nakon 
//ukrcavanja mijenja uvijete na camcu te daje mogucnost ukrcavanja
//svim dretvama na cekanju. Nakon toga dretva nestaje.
void *nova_osoba (void *p) 
{
	struct Covjek *cov = p;
	
	pthread_mutex_lock(&m);
	
	/*if(cov->prehrana==0)
		misionari[cov->obala]++;
	else
		kanibali[cov->obala]++;*/
	
	//Staje ako: dretva je na krivoj obali ili je brod pun
	while (cov->obala != brod_na_obali || (putnici[0] + putnici[1] >= 7) || 
		   		//ako su se neki misionari vec ukrcali i ima jednako kanibala, dretva je kanibal                  
			  	(putnici[0] != 0 && putnici[0] <= putnici[1] && cov->prehrana == 1)  ||
			  	//ako nema ukrcanih misionara, a kanibala ima vise od 1, a dretva je misionar
		  		(putnici[0] == 0 && putnici[1] > 1 && cov->prehrana == 0))
		pthread_cond_wait(red_a[cov->obala], &m);

	/*if(cov->prehrana==0)
		misionari[cov->obala]--;
	else
		kanibali[cov->obala]--;*/

	putnici[cov->prehrana]++;
	pthread_cond_broadcast(red_a[cov->obala]);
	pthread_cond_signal(&camac);

	pthread_mutex_unlock(&m);
	
	free(cov);
	
	return NULL;
}

//Funkcija koja sluzi za prijevoz kanibala i misionara.
//Nakon sto se u brod ukrca bar 3 putnika da mogucnost 
//ukrcavanja jos jednu sekundu, te onda krece.
//Funkcija se vrti beskonacno puta.
void *novi_brod()
{

	while(1)
	{
		pthread_mutex_lock(&m);
		
		while (putnici[0] + putnici[1] < 3)
			pthread_cond_wait(&camac, &m);

		pthread_mutex_unlock(&m);
		sleep(1);
		pthread_mutex_lock(&m);
		
		printf("Brod je na %s obali i prevozi %d misionara i %d kanibala.\n",strana_obale[brod_na_obali], putnici[0], putnici[1]);
		brod_na_obali = 1 - brod_na_obali;
		//printf("Na %s obali ima %d misionara i %d kanibala.\n", strana_obale[brod_na_obali], misionari[brod_na_obali], kanibali[brod_na_obali]);
		
		sleep(2);
		putnici[0] = putnici[1] = 0;
		pthread_mutex_unlock(&m);
	}
}

//Main na pocetku stvori dretvu koja ce oponasati camac.
//Main stvara svake sekunde jednog kainbala i svake 2 sekunde jednog misionara.
//Kanibali i misionari simbolizirani su pomocu dretve.
int main () 
{
	pthread_t thr_id;
	pthread_t thr_id_brod;
	pthread_attr_t attr;
	int id_osobe = 0;
	struct Covjek *kanibal1, *kanibal2, *misionar;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_create(&thr_id_brod, &attr, novi_brod, NULL);

	//period od 2 sekunde u kojim se stvori 2 kanibala i 1 misionar.
	while (1) 
	{
		kanibal1 = malloc(sizeof(struct Covjek));
		kanibal1->id = ++id_osobe;
		kanibal1->obala = rand() & 1;
		kanibal1->prehrana = 1;
		
		//printf("Novi kanibal na obali %d\n", kanibal1->obala);
		
		pthread_create(&thr_id, &attr, nova_osoba, (void *) kanibal1);

		misionar = malloc(sizeof(struct Covjek));
		misionar->id = ++id_osobe;
		misionar->obala = rand() & 1;
		misionar->prehrana = 0;
		
		//printf("Novi misionar na obali %d\n", misionar->obala);
		
		pthread_create(&thr_id, &attr, nova_osoba, (void *) misionar);
		
		sleep(1);

		kanibal2 = malloc(sizeof(struct Covjek));
		kanibal2->id = ++id_osobe;
		kanibal2->obala = rand() & 1;
		kanibal2->prehrana = 1;
		
		//printf("Novi kanibal na obali %d\n", kanibal2->obala);
		
		pthread_create(&thr_id, &attr, nova_osoba, (void *) kanibal2);

		sleep(1);
	}

	return 0;
}
