// Файл: server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void serve_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int read_size;
    char response[BUFFER_SIZE];

    // Читаем запрос от клиента
    read_size = read(client_socket, buffer, BUFFER_SIZE - 1);
    buffer[read_size] = '\0';

    // Печатаем запрос в консоль (для отладки)
    printf("Received request:\n%s\n", buffer);

    // Открываем файл index.html
    int file_fd = open("index.html", O_RDONLY);
    if (file_fd < 0) {
        perror("Failed to open file");
        close(client_socket);
        return;
    }

    // Читаем содержимое файла
    int file_size = read(file_fd, response, BUFFER_SIZE);
    close(file_fd);

    // Формируем HTTP-ответ
    char http_response[BUFFER_SIZE + 128];
    snprintf(http_response, sizeof(http_response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             file_size, response);

    // Отправляем ответ клиенту
    write(client_socket, http_response, strlen(http_response));

    // Закрываем соединение с клиентом
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Создаем сокет
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Настраиваем адрес сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Привязываем сокет к адресу
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Запускаем прослушивание соединений
    if (listen(server_socket, 10) < 0) {
        perror("Failed to listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    // Основной цикл обработки запросов
    while (1) {
        // Принимаем входящее соединение
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Failed to accept connection");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        // Обрабатываем запрос клиента
        serve_client(client_socket);
    }

    // Закрываем серверный сокет
    close(server_socket);
    return 0;
}
