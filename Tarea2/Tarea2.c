#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

/*

	Lucas Enrique Robles Chavez
	21.365.017-3

*/

struct Barberia{
	int barberos;
	int sillasB; // Sillas de barbero
    int sillasE; // Sillas de espera
};

struct Cliente{
    int tiempoEntrada; // Tiempo de entrada respecto a la entrada del cliente anterior.
    int tiempoEspera; // Tiempo que el cliente esta dispuesto a esperar en la silla de espera.
    int tiempoFinalizacion; // Tiempo que toma el corte al cliente por parte del barbero
};


struct Barberia barberia;
struct Cliente clientes[5]; // Esta hecho para el ejemplo del pdf con 5 clientes

void coordinador(){
    FILE* f = fopen("InputT2.txt", "r");
    
    if (!f){
		printf("\nEl archivo no fue encontrado\n");
		exit(1);
	}

    int aux = 0;
    for(int i=0 ; i < 3 ; i++){
        fscanf(f, "%i", &aux);
        
        if(i == 0){
            barberia.sillasE = aux;
        }
        else if(i == 1){
            barberia.barberos = aux;
        }
        else{
            barberia.sillasB = aux;
        }
    }
    printf("Hay %i sillas de espera, %i barberos y %i sillas de barbero\n\n", 
                barberia.sillasE, barberia.barberos, barberia.sillasB);

    for(int j=0 ; j < 5 ; j++){

        for(int i=0 ; i < 3 ; i++){
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

        printf("Entra %is despues del anterior, esta dispuesto a esperar %is y su corte tarda %is\n", 
                clientes[j].tiempoEntrada, clientes[j].tiempoEspera, clientes[j].tiempoFinalizacion);
    }

    /*
    char linea[245];
    int i=0; // NumCliente
    int flag = 1; // 1 = Barbero 2 = Cliente
    char *token;

    while (fgets(linea, sizeof(linea), f) != NULL){

        if(strlen(linea) <= 1){
            flag = 2;
            continue;
        }

        if (flag == 1){
            fscanf(linea, "%i %i %i", 
                barberia.sillasE, barberia.barberos, barberia.sillasB);
		    printf("Hay %i sillas de espera, %i barberos y %i sillas de barbero\n\n", 
                barberia.sillasE, barberia.barberos, barberia.sillasB);
	    }
        else{
            fscanf(linea, "%i %i %i", 
                clientes[i].tiempoEntrada, clientes[i].tiempoEspera, clientes[i].tiempoFinalizacion);
		    printf("Entra %is despues del anterior, esta dispuesto a esperar %is y su corte tarda %is\n", 
                clientes[i].tiempoEntrada, clientes[i].tiempoEspera, clientes[i].tiempoFinalizacion);
            i++;
	    }
	}
    */
}

int main (){

    coordinador();
	return 0;
}