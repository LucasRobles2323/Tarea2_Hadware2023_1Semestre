#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_CLIENTS 100

typedef struct {
    int arrival_time;
    int waiting_time;
    int haircut_time;
    int id;
} Client;

typedef struct {
    Client clients[MAX_CLIENTS];
    int num_clients;
    int num_waiting_chairs;
    int num_barbers;
    int num_barber_chairs;
    int next_barber_id;
    int next_client_id;
    pthread_mutex_t mutex;
    pthread_cond_t barberoDisponible;
    pthread_cond_t client_served;
} BarberShop;

void initBarberShop(BarberShop* shop, int num_waiting_chairs, int num_barbers, int num_barber_chairs) {
    shop->num_clients = 0;
    shop->num_waiting_chairs = num_waiting_chairs;
    shop->num_barbers = num_barbers;
    shop->num_barber_chairs = num_barber_chairs;
    shop->next_client_id = 1;
    shop->next_barber_id = 1;
    pthread_mutex_init(&shop->mutex, NULL);
    pthread_cond_init(&shop->barber_available, NULL);
    pthread_cond_init(&shop->client_served, NULL);
}

void destroyBarberShop(BarberShop* shop) {
    pthread_mutex_destroy(&shop->mutex);
    pthread_cond_destroy(&shop->barber_available);
    pthread_cond_destroy(&shop->client_served);
}

void* barberThread(void* arg) {
    BarberShop* shop = (BarberShop*)arg;
    while (1) {
        pthread_mutex_lock(&shop->mutex);
        while (shop->num_clients == 0) {
            pthread_cond_wait(&shop->barber_available, &shop->mutex);
        }
        int client_id = shop->clients[0].id;
        Client client = shop->clients[0];
        for (int i = 1; i < shop->num_clients; i++) {
            shop->clients[i - 1] = shop->clients[i];
        }
        shop->num_clients--;
        pthread_mutex_unlock(&shop->mutex);
        
        printf("El barbero %d está cortando el pelo del cliente %d\n", shop->next_barber_id, client_id);
        sleep(client.haircut_time);
        
        printf("El barbero %d ha terminado de cortar el pelo del cliente %d\n", shop->next_barber_id, client_id);
        pthread_cond_signal(&shop->client_served);
    }
    return NULL;
}

void* clientThread(void* arg) {
    BarberShop* shop = (BarberShop*)arg;
    Client client = shop->clients[shop->num_clients];
    
    pthread_mutex_lock(&shop->mutex);
    printf("El cliente %d ha entrado a la barbería.\n", client.id);
    
    if (shop->num_clients == shop->num_waiting_chairs) {
        printf("El cliente %d no encontró sillas de espera y se fue.\n", client.id);
        pthread_mutex_unlock(&shop->mutex);
        pthread_exit(NULL);
    }
    
    shop->num_clients++;
    int client_id = shop->next_client_id;
    shop->next_client_id++;
    
    pthread_cond_signal(&shop->barber_available);
    pthread_mutex_unlock(&shop->mutex);
    
    pthread_mutex_lock(&shop->mutex);
    while (client_id != shop->clients[0].id) {
        pthread_cond_wait(&shop->client_served, &shop->mutex);
    }
    pthread_mutex_unlock(&shop->mutex);
    
    printf("El cliente %d se ha sentado en una silla de espera.\n", client.id);
    sleep(client.waiting_time);
    
    pthread_mutex_lock(&shop->mutex);
    while (client_id != shop->clients[0].id) {
        pthread_cond_wait(&shop->client_served, &shop->mutex);
    }
    pthread_mutex_unlock(&shop->mutex);
    
    printf("El cliente %d ha terminado de cortarse el pelo y se va.\n", client.id);
    pthread_exit(NULL);
}

int main() {
    int num_waiting_chairs, num_barbers, num_barber_chairs;
    scanf("%d %d %d", &num_waiting_chairs, &num_barbers, &num_barber_chairs);
    
    BarberShop shop;
    initBarberShop(&shop, num_waiting_chairs, num_barbers, num_barber_chairs);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (scanf("%d %d %d", &shop.clients[i].arrival_time, &shop.clients[i].waiting_time, &shop.clients[i].haircut_time) != 3) {
            break;
        }
        shop.clients[i].id = i + 1;
        shop.num_clients++;
    }
    
    pthread_t barberThreads[num_barbers];
    for (int i = 0; i < num_barbers; i++) {
        pthread_create(&barberThreads[i], NULL, barberThread, &shop);
    }
    
    pthread_t clientThreads[MAX_CLIENTS];
    for (int i = 0; i < shop.num_clients; i++) {
        sleep(shop.clients[i].arrival_time);
        pthread_create(&clientThreads[i], NULL, clientThread, &shop);
    }
    
    for (int i = 0; i < num_barbers; i++) {
        pthread_join(barberThreads[i], NULL);
    }
    for (int i = 0; i < shop.num_clients; i++) {
        pthread_join(clientThreads[i], NULL);
    }
    
    destroyBarberShop(&shop);
    
    return 0;
}
