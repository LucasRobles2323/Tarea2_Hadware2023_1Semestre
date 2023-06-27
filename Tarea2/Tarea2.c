#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

/*

	Lucas Enrique Robles Chavez
	
    21.365.017-3

*/

#define NUM_CLIENTS_MAX 100 // Se asume que el codigo da el numero

typedef struct{
    int num; // Numero del cliente
    int entrada; // Tiempo de entrada respecto al inicio del problema.
    int espera; // Tiempo que el cliente esta dispuesto a esperar en la silla de espera.
    int finalizacion; // Tiempo que toma el corte al cliente por parte del barbero
}Cliente;

typedef struct{
    int cantBarberos;
    int *id_B; // id Barberos
    int idBarbero;
    Cliente clientes[NUM_CLIENTS_MAX];
    int *id_C; // id Clientes
    int idCliente;
    int sillasB; // Clientes en las sillas de barbero
    int cantSillasB;
    int sillasE; // Clientes en las sillas de espera
    int cantSillasE;
    pthread_mutex_t mutex;
    sem_t *barberoDisponible;
    sem_t clienteEsperando; // El cliente esperando en la silla barbero
}Barberia;

void iniBarberia(Barberia* b, int sillasEspera, int barberosCant, int sillasBarbero) {
    b->sillasE = 0;
    b->cantSillasE = sillasEspera;
    b->cantBarberos = barberosCant;
    b->cantSillasB = sillasBarbero;
    b->sillasB = 0;
    pthread_mutex_init(&b->mutex, NULL);
    b->barberoDisponible = (sem_t*) malloc(barberosCant * sizeof(sem_t));
    for (int i=0; i < barberosCant; i++){
        sem_init(&b->barberoDisponible[i], 0, 0);
    }
    sem_init(&b->clienteEsperando, 0, 0);
}

void destroyBarberia(Barberia* b) {
    pthread_mutex_destroy(&b->mutex);
    sem_destroy(&b->clienteEsperando);
}

void* fBarbers(void* argumentos)
{
    Barberia* barberia = (Barberia *)argumentos;
	int barbero_id = barberia->idBarbero;

    while (1) {
        // Esperar a que llegue un cliente
        sem_wait(&barberia->clienteEsperando);

        // Atender al cliente
        printf("El barbero %d está atendiendo a un cliente.\n", barbero_id);
        fflush(stdout);
        sleep(3); // Simulamos el tiempo que tarda el corte de pelo

        // Notificar al cliente que el corte de pelo ha terminado
        sem_post(&barberia->barberoDisponible[barbero_id]);
    }

	pthread_exit(NULL);// se termina la hebra con resultado NULL

}

void* fClients(void* argumentos)
{
    Barberia* barberia = (Barberia *)argumentos;
	int cliente_id = barberia->idCliente;
	//se hace la transformacion para evitar confuciones y mejor lectura

    pthread_mutex_lock(&barberia->mutex);

    printf("Entra cliente %i a barberia\n", cliente_id);
    fflush(stdout);

    pthread_mutex_unlock(&barberia->mutex);
    
    pthread_mutex_lock(&barberia->mutex);

    if (barberia->sillasE == barberia->cantSillasE) {
        printf("Sale cliente %d (no encontró sillas de espera)\n", cliente_id);
        fflush(stdout);
        pthread_mutex_unlock(&barberia->mutex);
        pthread_exit(NULL);
    }
    printf("Cliente %i usa silla de espera %i.\n", cliente_id, barberia->sillasE);
    fflush(stdout);
    barberia->sillasE++;
    
    pthread_mutex_unlock(&barberia->mutex);

    while (1) {
        pthread_mutex_lock(&barberia->mutex);

        if (barberia->sillasB < barberia->cantSillasB){
            break;
        }

        if (barberia->clientes[cliente_id].espera == 0){
            printf("Sale cliente %d (espero demasiado)\n", cliente_id);
            fflush(stdout);
            barberia->sillasE--;
            pthread_mutex_unlock(&barberia->mutex);
            pthread_exit(NULL);
        }

        pthread_mutex_unlock(&barberia->mutex);
        sleep(1);
        barberia->clientes[cliente_id].espera--;

    }
    printf("Cliente %i usa silla de barbero %i.\n", cliente_id, barberia->sillasB);
    fflush(stdout);
    barberia->sillasE--;
    barberia->sillasB++;

    pthread_mutex_unlock(&barberia->mutex);

	pthread_exit(NULL);// se termina la hebra con resultado NULL
}

void coordinador(){
    int cantClientes = 0, i, j;

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

    Barberia barberia;
    iniBarberia(&barberia, sillasEspera, barberosCant, sillasBarbero);
    
    /*
    printf("Hay %i sillas de espera, %i barberos y %i sillas de barbero\n\n", 
                sillasEspera, barberosCant, sillasBarbero);
    */

    /*****************************************************************************
    GUARDAR INFORMACION DE LOS CLIENTES
    ******************************************************************************/
    for (int i = 0; i < NUM_CLIENTS_MAX; i++) {
        if (fscanf(f, "%d %d %d", &barberia.clientes[i].entrada, 
                   &barberia.clientes[i].espera, 
                   &barberia.clientes[i].finalizacion) != 3)
        {
            break;
        }
        barberia.clientes[i].num = i;
        cantClientes++;
        /*
        printf("Entra cliente %i, %is despues del anterior, esta dispuesto a esperar %is y su corte tarda %is\n", 
                barberia.clientes[i].num, barberia.clientes[i].entrada, 
                barberia.clientes[i].espera, barberia.clientes[i].finalizacion,);
        */
    }

    /*****************************************************************************
    LA SALIDA DEL CODIGO
    ******************************************************************************/
    printf("\n");

    pthread_t barbersHilos[barberosCant];
    barberia.id_B = (int*) malloc (barberosCant * sizeof(int));
    for (int i = 0; i < barberosCant; i++) {
        barberia.id_B[i] = i;
        barberia.idBarbero = i;
        pthread_create(&barbersHilos[i], NULL, fBarbers, (void *)&barberia);
    }

    pthread_t clientsHilos[cantClientes];
    barberia.id_C = (int*) malloc (cantClientes * sizeof(int));
    for (int i = 0; i < cantClientes; i++) {
        sleep(barberia.clientes[i].entrada);
        barberia.id_C[i] = i;
        barberia.idCliente = i;
        pthread_create(&clientsHilos[i], NULL, fClients, (void *)&barberia);
    }


	for(i=0 ; i< cantClientes ; i++){
		pthread_join(clientsHilos[i], NULL);
	}

    for(i=0 ; i< barberosCant ; i++){
        pthread_cancel(barbersHilos[i]);
		pthread_join(barbersHilos[i], NULL);
        sem_destroy(&barberia.barberoDisponible[i]);
	}
    
    destroyBarberia(&barberia);

    printf("\n");
}

int main (){

    coordinador();
	return 0;
}