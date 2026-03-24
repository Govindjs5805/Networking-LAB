#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void get_unique_filename(char *filename) {
    struct stat buffer;
    if (stat(filename, &buffer) != 0)
        return;

    char name[256], ext[256];
    char newname[512];
    int count = 1;

    char *dot = strrchr(filename, '.');
    if (dot) {
        strncpy(name, filename, dot - filename);
        name[dot - filename] = '\0';
        strcpy(ext, dot);
    } else {
        strcpy(name, filename);
        ext[0] = '\0';
    }

    do {
        sprintf(newname, "%s(%d)%s", name, count++, ext);
    } while (stat(newname, &buffer) == 0);

    strcpy(filename, newname);
}

void send_list(int client_fd) {
    FILE *fp = popen("ls", "r");
    char buffer[BUFFER_SIZE];
    char data[10000] = "";

    while (fgets(buffer, sizeof(buffer), fp)) {
        strcat(data, buffer);
    }
    pclose(fp);

    long size = strlen(data);
    send(client_fd, &size, sizeof(size), 0);
    send(client_fd, data, size, 0);
}

void send_file(int client_fd, char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        long size = -1;
        send(client_fd, &size, sizeof(size), 0);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    send(client_fd, &size, sizeof(size), 0);

    char buffer[BUFFER_SIZE];
    while (!feof(fp)) {
        int n = fread(buffer, 1, BUFFER_SIZE, fp);
        send(client_fd, buffer, n, 0);
    }

    fclose(fp);
}

void receive_file(int client_fd, char *filename) {
    long size;
    recv(client_fd, &size, sizeof(size), 0);
    if (size <= 0) return;

    get_unique_filename(filename);

    FILE *fp = fopen(filename, "wb");
    char buffer[BUFFER_SIZE];
    long received = 0;

    while (received < size) {
        int n = recv(client_fd, buffer, BUFFER_SIZE, 0);
        fwrite(buffer, 1, n, fp);
        received += n;
    }

    fclose(fp);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char command[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);

    printf("Server started...\n");
    client_fd = accept(server_fd, NULL, NULL);

    while (1) {
        memset(command, 0, sizeof(command));
        recv(client_fd, command, sizeof(command), 0);

        if (strncmp(command, "LIST", 4) == 0) {
            send_list(client_fd);
        } else if (strncmp(command, "GET", 3) == 0) {
            send_file(client_fd, command + 4);
        } else if (strncmp(command, "PUT", 3) == 0) {
            receive_file(client_fd, command + 4);
        } else if (strncmp(command, "EXIT", 4) == 0) {
            break;
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
