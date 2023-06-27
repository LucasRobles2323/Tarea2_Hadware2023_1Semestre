#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_CLIENTES 10 // Número de clientes que llegan a la barbería
#define NUM_SILLAS 4 // Número de sillas disponibles en la sala de espera
#define NUM_BARBEROS 2 // Número de barberos en la barbería

sem_t sem_sillas; // Semáforo para controlar las sillas ocupadas en la sala de espera
sem_t sem_barbero[NUM_BARBEROS]; // Semáforos para indicar si los barberos están disponibles
sem_t sem_cliente; // Semáforo para indicar si hay un cliente en la sala de espera

pthread_mutex_t mutex_sillas; // Mutex para proteger el acceso a la variable sillas_ocupadas

int sillas_ocupadas = 0; // Variable para almacenar el número de sillas ocupadas en la sala de espera

void *cliente(void *num) {
    int cliente_id = *(int *)num;

    // Verificar si hay una silla libre en la sala de espera
    sem_wait(&sem_sillas);
    pthread_mutex_lock(&mutex_sillas);
    sillas_ocupadas++;
    printf("El cliente %d se sienta en una silla de espera.\n", cliente_id);
    fflush(stdout);
    pthread_mutex_unlock(&mutex_sillas);

    // Notificar a uno de los barberos que hay un cliente esperando
    sem_post(&sem_cliente);

    // Esperar a que un barbero esté disponible
    int barbero_id;
    sem_wait(&sem_barbero[barbero_id]);

    printf("El cliente %d está siendo atendido por el barbero %d.\n", cliente_id, barbero_id);
    fflush(stdout);
    sleep(2); // Simulamos el tiempo que tarda el corte de pelo

    printf("El cliente %d ha terminado y se va de la barbería.\n", cliente_id);
    fflush(stdout);

    // Liberar una silla en la sala de espera
    sem_wait(&sem_sillas);
    pthread_mutex_lock(&mutex_sillas);
    sillas_ocupadas--;
    pthread_mutex_unlock(&mutex_sillas);

    sem_post(&sem_sillas);
}

void *barbero(void *num) {
    int barbero_id = *(int *)num;

    while (1) {
        // Esperar a que llegue un cliente
        sem_wait(&sem_cliente);

        // Atender al cliente
        printf("El barbero %d está atendiendo a un cliente.\n", barbero_id);
        fflush(stdout);
        sleep(3); // Simulamos el tiempo que tarda el corte de pelo

        // Notificar al cliente que el corte de pelo ha terminado
        sem_post(&sem_barbero[barbero_id]);
    }
}

int main() {
    pthread_t barberThreads[NUM_BARBEROS];
    pthread_t clientThreads[NUM_CLIENTES];
    int barberos[NUM_BARBEROS];
    int clientes[NUM_CLIENTES];

    // Inicializar semáforos y mutex
    sem_init(&sem_sillas, 0, NUM_SILLAS);
    sem_init(&sem_cliente, 0, 0);
    for (int i = 0; i < NUM_BARBEROS; i++) {
        sem_init(&sem_barbero[i], 0, 0);
    }
    pthread_mutex_init(&mutex_sillas, NULL);

    // Crear los hilos de los barberos
    for (int i = 0; i < NUM_BARBEROS; i++) {
        barberos[i] = i;
        pthread_create(&barberThreads[i], NULL, barbero, (void *)&barberos[i]);
    }

    // Crear los hilos de los clientes
    for (int i = 0; i < NUM_CLIENTES; i++) {
        clientes[i] = i;
        pthread_create(&clientThreads[i], NULL, cliente, (void *)&clientes[i]);
        sleep(1); // Simulamos la llegada escalonada de los clientes
    }

    // Esperar a que todos los clientes terminen
    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_join(clientThreads[i], NULL);
    }

    // Cancelar los hilos de los barberos y liberar recursos
    for (int i = 0; i < NUM_BARBEROS; i++) {
        pthread_cancel(barberThreads[i]);
        pthread_join(barberThreads[i], NULL);
        sem_destroy(&sem_barbero[i]);
    }

    sem_destroy(&sem_sillas);
    sem_destroy(&sem_cliente);
    pthread_mutex_destroy(&mutex_sillas);

    return 0;
}
