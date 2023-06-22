#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

/*

	Lucas Enrique Robles Chavez
	21.365.017-3

*/

#define NUM_CLIENTS_MAX 100 // Se asume que el codigo da el numero

typedef struct{
    int num; // Numero del barbero
    bool desocupado; // Estado del barbero
}Barbero;

typedef struct{
    int num; // Numero del cliente
    int entrada; // Tiempo de entrada respecto al inicio del problema.
    int espera; // Tiempo que el cliente esta dispuesto a esperar en la silla de espera.
    int finalizacion; // Tiempo que toma el corte al cliente por parte del barbero
}Cliente;

typedef struct{
	Barbero* barberos;
    int cantBarberos;
    int idBarberos;
    Cliente clientes[NUM_CLIENTS_MAX];
    int cantClientes;
    int idCliente;
    int sillasB;
    int sillasE;
    pthread_mutex_t mutex;
    pthread_cond_t barberoDisponible;
    pthread_cond_t clienteServido;
}Barberia;


Barberia barberia;

void iniBarberia(Barberia* b, int sillasEspera, int barberosCant, int sillasBarbero) {
    b->cantClientes = 0;
    b->sillasE = sillasEspera;
    b->cantBarberos = barberosCant;
    b->sillasB = sillasBarbero;
    pthread_mutex_init(&b->mutex, NULL);
    pthread_cond_init(&b->barberoDisponible, NULL);
    pthread_cond_init(&b->clienteServido, NULL);
}

void destroyBarberia(Barberia* b) {
    pthread_mutex_destroy(&b->mutex);
    pthread_cond_destroy(&b->barberoDisponible);
    pthread_cond_destroy(&b->clienteServido);
}

void* fBarbers(void* argumentos)
{
	pthread_exit(NULL);// se termina la hebra con resultado NULL

}

void* fClients(void* argumentos)
{
	Barberia* barberia = (Barberia*) argumentos; 
	//se hace la transformacion para evitar confuciones y mejor lectura

    pthread_mutex_lock(&barberia->mutex);

    printf("Entra cliente %i a barberia\n", barberia->idCliente);
    pthread_mutex_unlock(&barberia->mutex);

	pthread_exit(NULL);// se termina la hebra con resultado NULL
}

void coordinador(){
    int cantClientes = 5, i, j;

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
        barberia.cantClientes++;
        /*
        printf("Entra cliente %i, %is despues del anterior, esta dispuesto a esperar %is y su corte tarda %is\n", 
                barberia.clientes[i].num, barberia.clientes[i].entrada, 
                barberia.clientes[i].espera, barberia.clientes[i].finalizacion);
        */
    }
    
    
    /*****************************************************************************
    CREACION DE MATRICES DE BARBERIA
    ******************************************************************************/
    barberia.barberos = (Barbero*) malloc( barberia.cantBarberos * sizeof(Barbero));
    if(!barberia.barberos)
    {
        printf("\nError al reservar memoria para los barberos\n");
        return;
    }
    else{
        for(i=0 ; i < barberia.cantBarberos ; i++){
            barberia.barberos[i].desocupado = true;
            barberia.barberos[i].num = i;
        }
    }

    /*****************************************************************************
    LA SALIDA DEL CODIGO
    ******************************************************************************/
    printf("\n");

    pthread_t barbersHilos[barberosCant];
    for (int i = 0; i < barberosCant; i++) {
        pthread_create(&barbersHilos[i], NULL, fBarbers, &barberia);
    }

    pthread_t clientsHilos[NUM_CLIENTS_MAX];
    for (int i = 0; i < barberosCant; i++) {
        barberia.idBarberos = i;
        pthread_create(&barbersHilos[i], NULL, fBarbers, &barberia);
    }

    for (int i = 0; i < barberia.cantClientes; i++) {
        sleep(barberia.clientes[i].entrada);
        barberia.idCliente = i;
        pthread_create(&clientsHilos[i], NULL, fClients, &barberia);
    }


    // Se espera que la salida (return) de los hilos sea NULL
    for(i=0 ; i< barberosCant ; i++){
		pthread_join(barbersHilos[i], NULL);
	}
	for(i=0 ; i< barberia.cantClientes ; i++){
		pthread_join(clientsHilos[i], NULL);
	}
    
    destroyBarberia(&barberia);

    printf("\n");
}

int main (){

    coordinador();
	return 0;
}