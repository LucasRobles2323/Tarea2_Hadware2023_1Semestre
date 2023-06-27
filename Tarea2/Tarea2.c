#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

/*

	Lucas Enrique Robles Chavez   -   21.365.017-3
    Gerald Andrés Espinoza Tapia   -   21.085.069-4
*/

typedef struct{
    int num; // Numero del cliente
    int entrada; // Tiempo de entrada respecto al inicio del problema.
    int espera; // Tiempo que el cliente esta dispuesto a esperar en la silla de espera.
    int finalizacion; // Tiempo que toma el corte al cliente por parte del barbero
}Cliente;

typedef struct{
    int cantBarberos;
    int *id_B; // id Barberos
    Cliente clientes[100]; // maximo acepta 100 clientes en los ejemplos
    int *id_C; // id Clientes
    int sillasB; // Clientes en las sillas de barbero
    int cantSillasB;
    int sillasE; // Clientes en las sillas de espera
    int cantSillasE;
}Barberia;

Barberia *barberia;
pthread_mutex_t mutex;
sem_t *barberoDisponible;
sem_t *barberoAtiende;
sem_t clienteEsperando; // El cliente esperando en la silla barbero
sem_t *clienteAtendido;


void iniBarberia(Barberia* b, 
                 int sillasEspera, int barberosCant, int sillasBarbero, int cantidadCLientes) {
    b->sillasE = 0;
    b->cantSillasE = sillasEspera;
    b->cantBarberos = barberosCant;
    b->cantSillasB = sillasBarbero;
    b->sillasB = 0;
    pthread_mutex_init(&mutex, NULL);

    barberoDisponible = (sem_t*) malloc(barberosCant * sizeof(sem_t));
    for (int i=0; i < barberosCant; i++){
        sem_init(&barberoDisponible[i], 0, 0);
    }
    barberoAtiende = (sem_t*) malloc(barberosCant * sizeof(sem_t));
    for (int i=0; i < barberosCant; i++){
        sem_init(&barberoAtiende[i], 0, 0);
    }
    
    sem_init(&clienteEsperando, 0, 0);
    clienteAtendido = (sem_t*) malloc(cantidadCLientes * sizeof(sem_t));
    for (int i=0; i < cantidadCLientes; i++){
        sem_init(&clienteAtendido[i], 0, 0);
    }
}

void destroy() {
    pthread_mutex_destroy(&mutex);
    sem_destroy(&clienteEsperando);
}

void* fBarbers(void* argumentos)
{
    int barbero_id = *(int *)argumentos;
    //se hace la transformacion para evitar confuciones y mejor lectura

    while (1) {
        // Esperar a que llegue un cliente
        sem_wait(&clienteEsperando);

        // Notificar al cliente que el barbero esta disponible
        sem_post(&barberoDisponible[barbero_id]);

        // Espera a que le digan que atendio al cliente
        sem_wait(&barberoAtiende[barbero_id]);

        pthread_mutex_lock(&mutex);

        int cliente_id = barberia->id_B[barbero_id];
        printf("Barbero %i atiende a cliente %i.\n", barbero_id,  cliente_id);
        fflush(stdout);

        pthread_mutex_unlock(&mutex);

        sleep(barberia->clientes[cliente_id].finalizacion);

        sem_post(&clienteAtendido[cliente_id]);
    }

	pthread_exit(NULL);// se termina la hebra con resultado NULL
}

void* fClients(void* argumentos)
{
    int cliente_id = *(int *)argumentos;
	//se hace la transformacion para evitar confuciones y mejor lectura

    pthread_mutex_lock(&mutex);

    printf("Entra cliente %i a barberia\n", cliente_id);
    fflush(stdout);

    pthread_mutex_unlock(&mutex);
    
    pthread_mutex_lock(&mutex);

    if (barberia->sillasE == barberia->cantSillasE) {
        printf("Sale cliente %d (no encontró sillas de espera)\n", cliente_id);
        fflush(stdout);
        pthread_mutex_unlock(&mutex);
        pthread_exit(NULL);
    }
    printf("Cliente %i usa silla de espera %i.\n", cliente_id, barberia->sillasE);
    fflush(stdout);
    barberia->sillasE++;
    
    pthread_mutex_unlock(&mutex);

    while (1) {
        pthread_mutex_lock(&mutex);

        if (barberia->sillasB < barberia->cantSillasB){
            break;
        }

        if (barberia->clientes[cliente_id].espera == 0){
            printf("Sale cliente %d (espero demasiado)\n", cliente_id);
            fflush(stdout);
            barberia->sillasE--;
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }

        pthread_mutex_unlock(&mutex);
        sleep(1);
        barberia->clientes[cliente_id].espera--;

    }
    printf("Cliente %i usa silla de barbero %i.\n", cliente_id, barberia->sillasB);
    fflush(stdout);
    barberia->sillasE--;
    barberia->sillasB++;

    pthread_mutex_unlock(&mutex);

    // Notificar a uno de los barberos que hay un cliente esperando
    sem_post(&clienteEsperando);

    // Esperar a que un barbero esté disponible
    int barbero_id;
    sem_wait(&barberoDisponible[barbero_id]);

    pthread_mutex_lock(&mutex);

    barberia->id_B[barbero_id] = cliente_id;

    pthread_mutex_unlock(&mutex);

    sem_post(&barberoAtiende[barbero_id]);

    sem_wait(&clienteAtendido[cliente_id]);

    pthread_mutex_lock(&mutex);

    printf("Sale cliente %i (atendido por completo).\n", cliente_id);
    fflush(stdout);
    barberia->sillasB--;

    pthread_mutex_unlock(&mutex);

	pthread_exit(NULL);// se termina la hebra con resultado NULL
}

void coordinador(){
    int cantClientes = 0, i, j;
    barberia = (Barberia*) malloc (sizeof(Barberia));

    /*****************************************************************************
    ABRIR TEXTO EN MODO LECTURA
    ******************************************************************************/
    FILE* f = fopen("InputT2.txt", "r");
    if (!f){
		printf("\nEl archivo no fue encontrado\n");
		exit(1);
	}

    /*****************************************************************************
    GUARDAR LA INFORMACION DEL TEXTO ABIERTO
    ******************************************************************************/
    int sillasEspera, barberosCant, sillasBarbero;
    fscanf(f, "%d %d %d", &sillasEspera, &barberosCant, &sillasBarbero);
    
    /*
    printf("Hay %i sillas de espera, %i barberos y %i sillas de barbero\n\n", 
                sillasEspera, barberosCant, sillasBarbero);
    */

    /*****************************************************************************
    GUARDAR INFORMACION DE LOS CLIENTES
    ******************************************************************************/
    for (int i = 0; i < 100; i++) {
        if (fscanf(f, "%d %d %d", &barberia->clientes[i].entrada, 
                   &barberia->clientes[i].espera, 
                   &barberia->clientes[i].finalizacion) != 3)
        {
            break;
        }
        barberia->clientes[i].num = i;
        cantClientes++;
        
        /*
        printf("Entra cliente %i, %is despues del anterior, esta dispuesto a esperar %is y su corte tarda %is\n", 
                barberia->clientes[i].num, barberia->clientes[i].entrada, 
                barberia->clientes[i].espera, barberia->clientes[i].finalizacion);
        */
    }

    iniBarberia(barberia, sillasEspera, barberosCant, sillasBarbero, cantClientes);

    /*****************************************************************************
    LA SALIDA DEL CODIGO
    ******************************************************************************/
    printf("\n");

    pthread_t barbersHilos[barberosCant];
    barberia->id_B = (int*) calloc (barberosCant, sizeof(int));
    int barberos[barberosCant];
    for (int i = 0; i < barberosCant; i++) {
        barberos[i] = i;
        pthread_create(&barbersHilos[i], NULL, fBarbers, (void *)&barberos[i]);
    }

    pthread_t clientsHilos[cantClientes];
    barberia->id_C = (int*) calloc (cantClientes, sizeof(int));
    int clientes[cantClientes];
    for (int i = 0; i < cantClientes; i++) {
        sleep(barberia->clientes[i].entrada);
        clientes[i] = i;
        pthread_create(&clientsHilos[i], NULL, fClients, (void *)&clientes[i]);
    }


	for(i=0 ; i< cantClientes ; i++){
		pthread_join(clientsHilos[i], NULL);
        sem_destroy(&clienteAtendido[i]);
	}

    for(i=0 ; i< barberosCant ; i++){
        pthread_cancel(barbersHilos[i]);
		pthread_join(barbersHilos[i], NULL);
        sem_destroy(&barberoDisponible[i]);
        sem_destroy(&barberoAtiende[i]);
	}
    
    destroy();

    printf("\n");
}

int main (){

    coordinador();
	return 0;
}
