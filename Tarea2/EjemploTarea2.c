#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

/*

	Javier Esteban Peña Reyes
	18.706.047-8

*/


typedef struct coord{ //estructura base de las posiciones, de las multiplicaciones y de el tamaño de las matrices
	int fila;
	int columna;
}info;


struct Matriz{ //estructura de matriz a trabajar
	info total;
	int** matriz;
};

static pthread_mutex_t printf_mutex; //controlador de printf para que la impresion sea en orden

struct Matriz matrices[3]; //arreglo de matrices con memoria compartida


void* calcular(void* argumentos){

	double res = 0;// valor donde se almacenan las multiplicaciones
	int i;

	info* coord = (info*) argumentos; 
	//se hace la transformacion para evitar confuciones y mejor lectura

    pthread_mutex_lock(&printf_mutex);
	// se bloquea el printf para que nada mas imprima antes

	printf("Proceso (%i) calculando elemento [%i,%i]\n", (int)pthread_self(), coord->fila, coord->columna); //se imprime la informacion de el thread
    pthread_mutex_unlock(&printf_mutex);// se desbloquea el printf
	for(i=0 ; i< matrices[0].total.columna ; i++){
		res += matrices[0].matriz[ coord->fila][i] * matrices[1].matriz[i][coord->columna]; //se calcula la multiplicacion de la matriz c con las posiciones de coord
	}
	matrices[2].matriz[coord->fila][coord->columna] = res;//se guarda el valor de la multiplicacion
    free(argumentos);//se libera la memoria usada

	pthread_exit(NULL);// se termina la hebra con resultado NULL

}


void coordinador(){
	int NumHebras,i,j,k;
	FILE* f = fopen("Ejemplo_Entrada_Tarea_2.txt", "r");//se abre el archivo y se revisa si este existe

	if (!f){
		printf("\nEl archivo no fue encontrado\n");
		return;
	}

	for (i=0; i<2; i++){
		//crea las matrices(una despues de otra) leyendo el archivo, si encuentra algun error, el programa aborta el proceso informando donde encontro el error
		fscanf(f, "%i %i", &matrices[i].total.fila, &matrices[i].total.columna);
		matrices[i].matriz = calloc(matrices[i].total.fila, sizeof(int*));
		if( !matrices[i].matriz ){
            printf("\nError al reservar memoria para la Matriz %i\n",i);
            return;
        }
        for(j=0; j < matrices[i].total.fila ; j++){
			matrices[i].matriz[j] = calloc (matrices[i].total.columna, sizeof(int));
			if( !matrices[i].matriz[j] ){
            	printf("\nError al reservar memoria de la fila %i de la matriz %i\n",j,i);
            	return;
        	}
		}
		for(j=0; j < matrices[i].total.fila ; j++){
			for(k=0; k < matrices[i].total.columna ; k++){//puebla la matriz de turno con los valores de el archivo, si no completa la matriz el programa aborta informando

                if(feof(f)){
                    printf("\nFaltan elementos para completar la matriz.\n");
                    return;
                }
				fscanf(f, "%i", &matrices[i].matriz[j][k]);


			}
		}
	}


	for(k=0 ; k<2 ; k++){
        printf("\nMatriz ");//muestra las 2 matrices ordenadas
        (k)? printf("B\n"):printf("A\n");
		for(i=0; i < matrices[k].total.fila ; i++){
			for(j=0; j < matrices[k].total.columna ; j++){
                printf("%i ", matrices[k].matriz[i][j]);
            }
            printf("\n");
        }
	}

	if(matrices[0].total.columna!=matrices[1].total.fila){//este programa no invierte el orden de las matrices en caso de error de ingreso, asi que si las matrices no son de igual columna-fila, no se realiza la multiplicacion
		printf("\nLas matrices A y B no se pueden multiplicar en el orden ingresado (Columna de A Distinta a Fila de B)\n");
		return;
	}
	printf("\n");

	matrices[2].total.fila = matrices[0].total.fila;
	matrices[2].total.columna = matrices[1].total.columna; //se reserva la memoria de la matriz C, informando por cualquier error en el camino

	matrices[2].matriz = calloc(matrices[2].total.fila, sizeof(int*));
    if( !matrices[2].matriz ){
            printf("\nError al reservar memoria para la Matriz C\n");
            return;
    }
	for(i=0 ; i< matrices[2].total.fila; i++){
		matrices[2].matriz[i] = calloc(matrices[2].total.columna, sizeof(int));
        if( !matrices[2].matriz[i] ){
            printf("\nError al reservar memoria de la fila %i de la matriz C\n",i);
            return;
        }
	}
	
	NumHebras = matrices[2].total.fila * matrices[2].total.columna;//se calcula la cantidad de hebras
	pthread_t threadArray[NumHebras]; //se crean las NumHebras hebras
	int id = 0;//se utiliza para avanza en el arreglo de hebras
	pthread_mutex_init(&printf_mutex, NULL); //se inicializa el controlador del printf

	for(i=0 ; i<matrices[2].total.fila ; i++){
		for(j=0 ; j<matrices[2].total.columna;j++){//se recorre la matriz a calcular con respecto a las matrices A y B
			info* inf = calloc(1, sizeof(info));
			inf->fila = i;
			inf->columna = j;
			pthread_create(&threadArray[id], NULL,(void*) &calcular, (void*)inf);
			// se crea la hebra id, con la funcion calcular, y 
			// los valores inf(que corresponden a las cordenadas i y j de la matriz C)
			id++;

		}
	}

	for(i=0 ; i< NumHebras ; i++){ //se espera el termino de todas las hebras con resultado NULL
		pthread_join(threadArray[i], NULL);
	}
	pthread_mutex_lock(&printf_mutex); //se bloquea el printf 

	printf("\nMatriz C\n");
	for (i=0; i<matrices[2].total.fila; i++){ //se imprime el resultado final de la matriz C
		for (j=0; j<matrices[2].total.columna; j++){
			printf("%i ", matrices[2].matriz[i][j]);
		}
		printf("\n");
	}

	pthread_mutex_unlock(&printf_mutex);// se desbloquea el printf

}

int main (){

    coordinador();
	return 0;
}