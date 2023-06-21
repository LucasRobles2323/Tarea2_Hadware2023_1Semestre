#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

/*

	Lucas Enrique Robles Chavez
	21.365.017-3

*/

typedef struct{
    int num; // Numero del barbero
    bool libre; // Señala si el barbero esta libre
}Barbero;

typedef struct{
    int num; // Numero del cliente
    int entrada; // Tiempo de entrada respecto al inicio del problema.
    int espera; // Tiempo que el cliente esta dispuesto a esperar en la silla de espera.
    int finalizacion; // Tiempo que toma el corte al cliente por parte del barbero
}Cliente;

typedef struct{
	bool *barberos;
    int barberosCant;
	bool *sillasB; // Sillas de barbero
    int sillasBCant;
    bool *sillasE; // Sillas de espera
    int sillasECant;
}Barberia;


Barberia barberia;
Cliente clientes[5]; // Esta hecho para el ejemplo del pdf con 5 clientes
static pthread_mutex_t printf_mutex; //controlador de printf para que la impresion sea en orden

void* fBarber(void* argumentos){
    pthread_exit(NULL);// se termina la hebra con resultado NULL
	int i, j;

	Barbero* coordB = (Barbero*) argumentos; 
	//se hace la transformacion para evitar confuciones y mejor lectura

	pthread_exit(NULL);// se termina la hebra con resultado NULL
}

void* fClientes(void* argumentos){
	int i, j;
    int tiempoEspera;

	Cliente* coordC = (Cliente*) argumentos; 
	//se hace la transformacion para evitar confuciones y mejor lectura


    if (coordC->entrada == 0){
        pthread_mutex_lock(&printf_mutex);

        printf("Entra cliente %i a barberia\n", coordC->num);
        
        pthread_mutex_unlock(&printf_mutex);
    }
    else {
        pthread_mutex_lock(&printf_mutex);

        sleep( coordC->entrada );
	    printf("Entra cliente %i a barberia\n", coordC->num);
    
        pthread_mutex_unlock(&printf_mutex);
    }

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
    int aux = 0;
    for(i=0 ; i < 3 ; i++){
        /*****************************************************************************
        GUARDAR INFORMACION DE LA BARBERIA
        ******************************************************************************/
        fscanf(f, "%i", &aux);
        if(i == 0){
            barberia.sillasECant = aux;
        }
        else if(i == 1){
            barberia.barberosCant = aux;
        }
        else{
            barberia.sillasBCant = aux;
        }
    }
    //printf("Hay %i sillas de espera, %i barberos y %i sillas de barbero\n\n", 
    //            barberia.sillasECant, barberia.barberosCant, barberia.sillasBCant);

    /*****************************************************************************
    GUARDAR INFORMACION DE LOS CLIENTES
    ******************************************************************************/
    for(j=0 ; j < cantClientes ; j++){

        for(i=0 ; i < 3 ; i++){
            fscanf(f, "%i", &aux);
        
            if(i == 0){
                if(j == 0){
                    clientes[j].entrada = aux;
                }
                else{
                    clientes[j].entrada = aux + clientes[j-1].entrada;
                }
            }
            else if(i == 1){
                clientes[j].espera = aux;
            }
            else{
                clientes[j].finalizacion = aux;
            }
        }
        clientes[j].num = j;

        //printf("Entra cliente %i, %is despues del anterior, esta dispuesto a esperar %is y su corte tarda %is\n", 
        //        clientes[j].num, clientes[j].entrada, clientes[j].espera, clientes[j].finalizacion);
    }


    /*****************************************************************************
    CREACION DE MATRICES DE BARBERIA
    ******************************************************************************/
    barberia.barberos = (bool*) malloc( barberia.barberosCant * sizeof(bool));
    if(!barberia.barberos)
    {
        printf("\nError al reservar memoria para los barberos\n");
        return;
    }
    else{
        for(i=0 ; i < barberia.barberosCant ; i++){
            barberia.barberos[i] = true; // true = esta disponible 
        }
    }
    barberia.sillasB = (bool*) malloc( barberia.sillasBCant * sizeof(bool));
    if(!barberia.sillasB)
    {
        printf("\nError al reservar memoria para los barberos\n");
        return;
    }
    else{
        for(i=0 ; i < barberia.sillasBCant ; i++){
            barberia.sillasB[i] = true; // true = esta disponible 
        }
    }
    barberia.sillasE = (bool*) malloc( barberia.sillasECant * sizeof(bool));
    if(!barberia.sillasE)
    {
        printf("\nError al reservar memoria para los barberos\n");
        return;
    }
    else{
        for(i=0 ; i < barberia.sillasECant ; i++){
            barberia.sillasE[i] = true; // true = esta disponible 
        }
    }


    /*****************************************************************************
    VARIABLES PARA LA SALIDA DEL CODIGO 
    ******************************************************************************/
    pthread_t barbers[barberia.barberosCant];
    pthread_t clients[cantClientes];
    int idBarber = 0;//se utiliza para avanza en el arreglo de hebras
	int idClient = 0;//se utiliza para avanza en el arreglo de hebras
	pthread_mutex_init(&printf_mutex, NULL); //se inicializa el controlador del printf

    printf("\n");

    /*****************************************************************************
    LA SALIDA DEL CODIGO 
    ******************************************************************************/
    // Crear hilos Barbero
    for(i = 0 ; i < barberia.barberosCant ; i++){
		Barbero* barber = calloc(1, sizeof(Barbero));
		barber->num = i;
        barber->libre = true;

		pthread_create(&barbers[idBarber], NULL,(void*) &fBarber, (void*)barber);
		
        idBarber++;
	}

    // Crear hilos clientes
    for(i = 0 ; i < cantClientes ; i++){
		Cliente* client = calloc(1, sizeof(Cliente));
		client->entrada = clientes[i].entrada;
		client->espera = clientes[i].espera;
        client->finalizacion = clientes[i].finalizacion;
        client->num = clientes[i].num;

		pthread_create(&clients[idClient], NULL,(void*) &fClientes, (void*)client);
		
        idClient++;
	}

    for(i=0 ; i< barberia.barberosCant ; i++){ //se espera el termino de todas las hebras con resultado NULL
		pthread_join(barbers[i], NULL);
	}
	for(i=0 ; i< cantClientes ; i++){ //se espera el termino de todas las hebras con resultado NULL
		pthread_join(clients[i], NULL);
	}

}

int main (){

    coordinador();
	return 0;
}