#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>


int polje_sa_signalima[] = {SIGQUIT, SIGUSR1, SIGUSR2, SIGTSTP, SIGINT};
int pid;

//Slanjem sigkill signala zavrsavamo s radom.
//Drugom programu vise nista ne saljmo
void obradi_SIGINT( int sig )
{
    kill( pid, SIGKILL);
    exit(0);
}

int main(int argc, char* argv[])
{

    if( argc != 2 ){
        printf("Program %s ocekuje pid od programa kojem salje signale.\n",
                argv[0], argv[1]);
        return -1;
    }

    pid = atoi(argv[1]);

    sigset (SIGINT, obradi_SIGINT);

    int cekanje, signal;
    //program salje trazenom procesu signale dok ne dobije sigint signal
    while(1)
    {
        //cekanje je izmedu 2 i 7 sekundi
        cekanje = rand() % 6 + 2;
        //signal je jedan od 4 prva u globalnom polju
        signal = polje_sa_signalima[rand() % 5];

        //sljanje signala drugom programu
        kill(pid, signal);
        sleep(cekanje);
    }

    return 0;
}
