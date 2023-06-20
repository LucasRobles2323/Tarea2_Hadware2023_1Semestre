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
	bool *barberos;
    int barberosCant;
	bool *sillasB; // Sillas de barbero
    int sillasBCant;
    bool *sillasE; // Sillas de espera
    int sillasECant;
}Barberia;

typedef struct{
    int numCliente;
    int tiempoEntrada; // Tiempo de entrada respecto a la entrada del cliente anterior.
    int tiempoEspera; // Tiempo que el cliente esta dispuesto a esperar en la silla de espera.
    int tiempoFinalizacion; // Tiempo que toma el corte al cliente por parte del barbero
}Cliente;


Barberia barberia;
Cliente clientes[5]; // Esta hecho para el ejemplo del pdf con 5 clientes
static pthread_mutex_t printf_mutex; //controlador de printf para que la impresion sea en orden

void* mostrar(void* argumentos){

	double res = 0;// valor donde se almacenan las multiplicaciones
	int i, j;
    int tiempoEspera;

	Cliente* coord = (Cliente*) argumentos; 
	//se hace la transformacion para evitar confuciones y mejor lectura

    sleep(coord->tiempoEntrada);

    pthread_mutex_lock(&printf_mutex);
	// se bloquea el printf para que nada mas imprima antes

	printf("Entra cliente %i a barberia\n", coord->numCliente); //se imprime la informacion de el thread
    
    pthread_mutex_unlock(&printf_mutex);// se desbloquea el printf

    int vacioSE = -1; // Si hay sillas de espera disponible
    for(i=0 ; i < barberia.sillasECant ; i++){
        if(barberia.sillasE[i]){
            barberia.sillasE[i] = false;
            vacioSE = i;
            break;
        }
    }

    pthread_mutex_lock(&printf_mutex);
	// se bloquea el printf para que nada mas imprima antes
	if( vacioSE != -1 ){
        printf("Cliente %i usa silla de espera %i\n", coord->numCliente, vacioSE); //se imprime la informacion de el thread
    }
    else{
        printf("Sale cliente %i porque no hay sillas de espera disponibles. \n", coord->tiempoEntrada); 
    }
    pthread_mutex_unlock(&printf_mutex);// se desbloquea el printf

    int vacioSB = -1; // Para saber si hay silla barbero disponible
    while (tiempoEspera < coord->tiempoEspera)
    {
        
        for(i=0 ; i < barberia.sillasBCant ; i++){
            
            if(barberia.sillasB[i]){
                vacioSB = i;
                barberia.sillasE[vacioSE] = true;
                barberia.sillasE[vacioSB] = false;
                break;
            }
        }

        if( vacioSB != -1){
            break;
        }
        sleep(1);
        tiempoEspera++;
    }

    pthread_mutex_lock(&printf_mutex);
	// se bloquea el printf para que nada mas imprima antes

    if( tiempoEspera < coord->tiempoEspera){
        printf("Cliente %i usa silla de barbero %i\n", coord->numCliente, vacioSB);
        //se imprime la informacion de el thread
    }
    else{
        printf("Sale cliente %i porque espero demasiado\n", coord->numCliente);
    }

    pthread_mutex_unlock(&printf_mutex);// se desbloquea el printf

    int vacioB = -1; // Para saber si hay barbero disponible
    while (vacioB == -1)
    {
        for(i=0 ; i < barberia.barberosCant ; i++){
            if(barberia.barberos[i]){
                vacioB = i;
                barberia.barberos[i] = false;
            }
        }
    }

    pthread_mutex_lock(&printf_mutex);
	// se bloquea el printf para que nada mas imprima antes

    printf("Barbero %i atiende a cliente %i\n", vacioB, coord->numCliente);

    pthread_mutex_unlock(&printf_mutex);// se desbloquea el printf

    sleep(coord->tiempoFinalizacion);
    barberia.barberos[vacioB] = false;
    barberia.sillasE[vacioSB] = false;

    pthread_mutex_lock(&printf_mutex);
	// se bloquea el printf para que nada mas imprima antes
    
    printf("Sale cliente %i (atendido por completo)\n", coord->numCliente);

    pthread_mutex_unlock(&printf_mutex);// se desbloquea el printf

	pthread_exit(NULL);// se termina la hebra con resultado NULL

}

void coordinador(){
    int NumHebras, i, j;

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
    for(j=0 ; j < 5 ; j++){

        for(i=0 ; i < 3 ; i++){
            fscanf(f, "%i", &aux);
        
            if(i == 0){
                clientes[j].tiempoEntrada = aux;
            }
            else if(i == 1){
                clientes[j].tiempoEspera = aux;
            }
            else{
                clientes[j].tiempoFinalizacion = aux;
            }
        }
        clientes[j].numCliente = j;

        //printf("Entra cliente %i, %is despues del anterior, esta dispuesto a esperar %is y su corte tarda %is\n", 
        //        clientes[j].numCliente, clientes[j].tiempoEntrada, clientes[j].tiempoEspera, clientes[j].tiempoFinalizacion);
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
    NumHebras = 5;
    pthread_t threadArray[NumHebras]; //se crean las NumHebras hebras
	int id = 0;//se utiliza para avanza en el arreglo de hebras
	pthread_mutex_init(&printf_mutex, NULL); //se inicializa el controlador del printf

    printf("\n\n");

    /*****************************************************************************
    LA SALIDA DEL CODIGO 
    ******************************************************************************/
    for(i = 0 ; i < 5 ; i++){
		Cliente* client = calloc(1, sizeof(Cliente));
		client->tiempoEntrada = clientes[i].tiempoEntrada;
		client->tiempoEspera = clientes[i].tiempoEspera;
        client->tiempoFinalizacion = clientes[i].tiempoFinalizacion;
        client->numCliente = clientes[i].numCliente;
		pthread_create(&threadArray[id], NULL,(void*) &mostrar, (void*)client);
		// se crea la hebra id, con la funcion mostrar, y 
		// los valores iCliente(que corresponde a la informaciòn de cada cliente)
		id++;
	}

	for(i=0 ; i< NumHebras ; i++){ //se espera el termino de todas las hebras con resultado NULL
		pthread_join(threadArray[i], NULL);
	}

}

int main (){

    coordinador();
	return 0;
}