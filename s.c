#include <stdio.h> 
#include <arpa/inet.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> // read(), write(), close()
#include <pthread.h> // Para utilizar hilos (threads)
#include "encode/Encode.c"
#include "encode/Decode.c"
#include "Packets/packet.c"
#include "tree/Tree.c"

#define MAX 80 
#define CONFIG_FILE "config.txt" // Nombre del archivo de configuración
#define SA struct sockaddr 

// Función diseñada para el chat entre cliente y servidor. 
void *func(void *arg) 
{ 
    int connfd = *((int*)arg);
    TreeNode *root = createTreeNode("/");
    MQTT_Packet *buff = malloc(sizeof(MQTT_Packet)); // Allocate memory for buff
    if (buff == NULL) {
        perror("Error allocating memory for buffer");
        close(connfd);
        return NULL;
    }
    
    bzero(buff, sizeof(MQTT_Packet)); 
    read(connfd, buff, sizeof(MQTT_Packet));
    size_t total_length = buff->remaining_length + sizeof(buff->payload);
    unsigned char *serialized_packet = malloc(total_length);
    memcpy(serialized_packet, buff->variable_header, buff->remaining_length);
    printf("Serialized packet: ");
    for (int i = 0; i < total_length; ++i) {
        printf("%02X ", serialized_packet[i]);
    }
    printf("\n");
    //bzero(buff, sizeof(MQTT_Packet));
    free(buff);

    close(connfd); // Cerrar la conexión con este cliente
    return NULL;
} 

// Función principal 
int main() 
{ 
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	// Leer IP y puerto desde el archivo de configuración
    FILE *config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL)
    {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }

    char ip[MAX];
    int port;
    if (fscanf(config_file, "%s%d", ip, &port) != 2)
    {
        perror("Error reading config file");
        exit(EXIT_FAILURE);
    }
    fclose(config_file);

	// Crear el socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// Asignar IP y PORT
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(ip); 
	servaddr.sin_port = htons(port); 

	// Enlazar el socket al IP y PORT
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Escuchar las conexiones entrantes
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 

	len = sizeof(cli); 

	// Bucle para aceptar conexiones entrantes
	while (1) {
		// Aceptar el paquete de datos del cliente
		connfd = accept(sockfd, (SA*)&cli, &len); 
		if (connfd < 0) { 
			printf("server accept failed...\n"); 
			exit(0); 
		} 
		else
			printf("server accept the client...\n"); 

		printf("PENE");
		
		pthread_t tid; // Identificador del hilo
		pthread_create(&tid, NULL, func, &connfd); // Crear un hilo para manejar esta conexión
	}

	close(sockfd); // Cerrar el socket principal
	return 0;
}
