#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <semaphore.h>

/*

	Lucas Enrique Robles Chavez    -   21.365.017-3
    Gerald Andrés Espinoza Tapia   -   21.085.069-4
    
*/


typedef struct{
    int num; // Numero del cliente
    int entrada; // Tiempo de entrada respecto al inicio del problema.
    int espera; // Tiempo que el cliente esta dispuesto a esperar en la silla de espera.
    int finalizacion; // Tiempo que toma el corte al cliente por parte del barbero
}Cliente;

typedef struct{
    sem_t mutex;
    int next; // Cliente a imprimir
    int cantBarberos;
    Cliente clientes[100]; // maximo acepta 100 clientes en los ejemplos
    int sillasB; // Clientes en las sillas de barbero
    int cantSillasB;
    int sillasE; // Clientes en las sillas de espera
    int cantSillasE;
}Barberia;

Barberia *barberia;
bool *bDesocupado;

void iniBarberia(Barberia* b, bool *b_Desocupado, 
                 int sillasEspera, int barberosCant, int sillasBarbero, int cantidadCLientes) {
    b->sillasE = 0;
    b->cantSillasE = sillasEspera;
    b->cantBarberos = barberosCant;
    b->cantSillasB = sillasBarbero;
    b->sillasB = 0;
    b->next = 0;
    sem_init(&b->mutex, 1, 1);

    for (int i=0; i < barberosCant; i++){
        b_Desocupado[i] = true;
    }
}

void fClients(int argumento1)
{
    int cliente_id = argumento1;
	//se hace la transformacion para evitar confuciones y mejor lectura

    while (cliente_id != barberia->next)
    {
        sem_post(&barberia->mutex);
        sleep(0);
        sem_wait(&barberia->mutex);
    }
    

    printf("Entra cliente %i a barberia\n", cliente_id);
    fflush(stdout);
    sem_post(&barberia->mutex);

    sem_wait(&barberia->mutex);

    if (barberia->sillasE == barberia->cantSillasE) {
        printf("Sale cliente %d (no encontró sillas de espera)\n", cliente_id);
        fflush(stdout);
        sem_post(&barberia->mutex);
        return;
    }
    printf("Cliente %i usa silla de espera %i.\n", cliente_id, barberia->sillasE);
    fflush(stdout);
    barberia->sillasE++;
    barberia->next++;

    sem_post(&barberia->mutex);

    while (1) {
        sem_wait(&barberia->mutex);
        if (barberia->sillasB < barberia->cantSillasB){
            sem_post(&barberia->mutex);
            break;
        }

        if (barberia->clientes[cliente_id].espera == 0){
            printf("Sale cliente %d (espero demasiado)\n", cliente_id);
            fflush(stdout);
            barberia->sillasE--;
            sem_post(&barberia->mutex);
            return;
        }
        sem_post(&barberia->mutex);
        sleep(1);
        barberia->clientes[cliente_id].espera--;

    }

    sem_wait(&barberia->mutex);
    printf("Cliente %i usa silla de barbero %i.\n", cliente_id, barberia->sillasB);
    fflush(stdout);
    barberia->sillasE--;
    barberia->sillasB++;

    sem_post(&barberia->mutex);
    sleep(0);

    int barbero_id = -1;
    while (1)
    {
        sem_wait(&barberia->mutex);

        if(barbero_id != -1){
            sem_post(&barberia->mutex);
            break;
        }

        for (int i=0; i < barberia->cantBarberos; i++){
            if(bDesocupado[i]){
                barbero_id = i;
                bDesocupado[i] = false;
                break;
            }
        }
        sem_post(&barberia->mutex);
    }

    printf("Barbero %i atiende a cliente %i.\n", barbero_id,  cliente_id);
    fflush(stdout);
    
    sleep(barberia->clientes[cliente_id].finalizacion);

    printf("Sale cliente %i (atendido por completo).\n", cliente_id);
    fflush(stdout);
    
    sem_wait(&barberia->mutex);
    barberia->sillasB--;
    bDesocupado[barbero_id] = true;
    sem_post(&barberia->mutex);
    return;
}

void coordinador(){
    int cantClientes = 0, i, j;

    /*****************************************************************************
    MMAP PARA LAS HEBRAS NO JERARQUICAS
    ******************************************************************************/
    int fd1 = -1, fd2 = -1;

    // Crear el área de memoria compartida
    fd1 = shm_open("/dataBarberia", O_CREAT | O_RDWR, 0666);
    fd2 = shm_open("/dataBarberos", O_CREAT | O_RDWR, 0666);

    // Fijar el tamaño del área de memoria compartida
    if( ftruncate(fd1, sizeof(Barberia)) == -1 || ftruncate(fd2, sizeof(int)) == -1){
        printf("Error al fijar la memoria del mmap.\n");
    }

    // Mapear el área de memoria compartida
    barberia = (Barberia*) mmap(NULL, sizeof(Barberia), 
                                PROT_READ | PROT_WRITE, 
                                MAP_SHARED, fd1, 0);
    bDesocupado = (bool*) mmap(NULL, sizeof(bool), 
                                PROT_READ | PROT_WRITE, 
                                MAP_SHARED, fd2, 0);
    

    if (barberia == MAP_FAILED || bDesocupado == MAP_FAILED){
        printf("Error al crear mmap.\n");
        exit(1);
    }

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
        if(i > 0){
            barberia->clientes[i].espera+= barberia->clientes[i-1].espera;
        }
        /*
        printf("Entra cliente %i, %is despues del anterior, esta dispuesto a esperar %is y su corte tarda %is\n", 
                barberia->clientes[i].num, barberia->clientes[i].entrada, 
                barberia->clientes[i].espera, barberia->clientes[i].finalizacion);
        */
    }


    iniBarberia(barberia, bDesocupado, sillasEspera, barberosCant, sillasBarbero, cantClientes);

    /*****************************************************************************
    LA SALIDA DEL CODIGO
    ******************************************************************************/
    printf("\n");

    for (int i = 0; i < cantClientes; i++) {
        if(fork() == 0){
            // Hebra cliente
            if(barberia->clientes[i].entrada != 0){
                sleep(barberia->clientes[i].entrada);
            }
            fClients(i);
            exit(0);
        }

    }

    // Esperar a que todos los clientes salgan
    for (int i = 0; i < cantClientes; i++) {
        wait(NULL);
    }

    // Destruir semaforos
    sem_destroy(&barberia->mutex);

    // Desmapear y cerrar el áera de memoria compartida
    munmap(barberia, sizeof(Barberia));
    munmap(bDesocupado, sizeof(bool));
    close(fd1);
    close(fd2);

    // Eliminar el área de memoria compartida
    shm_unlink("/dataBarberia");
    shm_unlink("/dataBarberos");

    printf("\n");
}

int main (){

    coordinador();
	return 0;
}