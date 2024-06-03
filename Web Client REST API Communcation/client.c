#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "buffer.h"
#include "parson.h"

#define HOST "34.246.184.49"
#define SERVER_PORT 8080
#define COMMAND_LEN 1000
#define ANOTHER_LEN 100
#define BUFLEN 4096
#define PAYLOAD_SIZE 2048
#define MESSAGE_SIZE 8192

// variabile pentru stocarea bearer tokenului și cookie-ului
char bearer_token[COMMAND_LEN] = {0};
char session_cookie[COMMAND_LEN] = {0};

void register_user() {
    char username[COMMAND_LEN];
    char password[COMMAND_LEN];
    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);

    // deschidem o conexiune socket
    int sockfd = open_connection(HOST, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    // construim cererea HTTP POST
    char message[BUFLEN];
    snprintf(message, sizeof(message), "POST /api/v1/tema/auth/register HTTP/1.1\r\n");
    strcat(message, "Host: " HOST ":8080\r\n");
    strcat(message, "Content-Type: application/x-www-form-urlencoded\r\n");

    char payload[PAYLOAD_SIZE];
    snprintf(payload, sizeof(payload), "username=%s&password=%s", username, password);
    char content_length_header[50];
    snprintf(content_length_header, sizeof(content_length_header), "Content-Length: %ld\r\n", strlen(payload));

    strcat(message, content_length_header);
    strcat(message, "\r\n");
    strcat(message, payload);
    strcat(message, "\r\n\r\n");

    // se trimite cererea la server
    send_to_server(sockfd, message);

    // primim raspunsul de la server
    char *response = receive_from_server(sockfd);

    // verificam raspunsul pentru a determina daca inregistrarea a fost un succes
    if (response) {
        //  se ia prima linie din răspuns, care ar trebui să fie linia de status
        char *status_line = strtok(response, "\r\n");
        if (strstr(status_line, "201 Created") || strstr(status_line, "200 OK")) {
            printf("200 - OK - SUCCESS , user registered.\n");
        } else {
            printf("ERROR , user already registered\n");
        }
    } else {
        printf("ERROR\n");
    }

    // Inchidem conexiunea
    close_connection(sockfd);
    free(response);
}

void login_user() {
    char username[COMMAND_LEN];
    char password[COMMAND_LEN];
    printf("username= ");
    scanf("%s", username);
    printf("password= ");
    scanf("%s", password);

    int sockfd = open_connection(HOST, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    char message[BUFLEN];
    snprintf(message, sizeof(message), "POST /api/v1/tema/auth/login HTTP/1.1\r\n");
    strcat(message, "Host: " HOST ":8080\r\n");
    strcat(message, "Content-Type: application/json\r\n");

    char payload[PAYLOAD_SIZE];
    snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);
    char content_length_header[50];
    snprintf(content_length_header, sizeof(content_length_header), "Content-Length: %ld\r\n", strlen(payload));

    strcat(message, content_length_header);
    strcat(message, "\r\n");
    strcat(message, payload);
    strcat(message, "\r\n");

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    if (strstr(response, "200 OK")) {
        printf("200 - OK - SUCCESS , logged in.\n");
    } else {
        printf("ERROR , wrong credentials\n");
    }

    // se extrage cookie-ul de sesiune
    char *cookie_start = strstr(response, "Set-Cookie: ");
    if (cookie_start) {
        cookie_start += strlen("Set-Cookie: ");
        char *cookie_end = strstr(cookie_start, ";");
        if (cookie_end) {
            size_t cookie_length = cookie_end - cookie_start;
            strncpy(session_cookie, cookie_start, cookie_length);
            session_cookie[cookie_length] = '\0';
        }
    } else {
        printf("ERROR , no session cookie received, check server response.\n");
    }

    close_connection(sockfd);
    free(response);
}

void enter_library() {
    if (strlen(session_cookie) == 0) {
        printf("ERROR , no session available, log in first.\n");
        return;
    }

    int sockfd = open_connection(HOST, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    char message[BUFLEN];
    snprintf(message, sizeof(message), "GET /api/v1/tema/library/access HTTP/1.1\r\n");
    strcat(message, "Host: " HOST ":8080\r\n");
    strcat(message, "Cookie: ");
    strcat(message, session_cookie);
    strcat(message, "\r\n\r\n");

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    if (strstr(response, "200 OK")) {
        printf("200 - OK - SUCCESS , entered library.\n");
    } else {
        printf("ERROR , couldn't enter library\n");
    }

    // se extrage JWT token
    char *token_start = strstr(response, "\"token\":\"");
    if (token_start) {
        token_start += strlen("\"token\":\"");
        char *token_end = strchr(token_start, '\"');
        if (token_end) {
            size_t token_length = token_end - token_start;
            strncpy(bearer_token, token_start, token_length);
            bearer_token[token_length] = '\0';
        }
    }

    close_connection(sockfd);
    free(response);
}

void get_books() {
    if (strlen(session_cookie) == 0 || strlen(bearer_token) == 0) {
        printf("ERROR , please log in and enter the library first.\n");
        return;
    }

    int sockfd = open_connection(HOST, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    char message[BUFLEN];
    snprintf(message, sizeof(message), "GET /api/v1/tema/library/books HTTP/1.1\r\n");
    strcat(message, "Host: ");
    strcat(message, HOST);
    strcat(message, ":8080\r\n");
    strcat(message, "Cookie: ");
    strcat(message, session_cookie);
    strcat(message, "\r\nAuthorization: Bearer ");
    strcat(message, bearer_token);
    strcat(message, "\r\n\r\n");

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    // partea "JSON" a response-ului
    char *json_start = strstr(response, "[");
    if (json_start) {
        JSON_Value *root_value = json_parse_string(json_start);
        JSON_Array *books = json_value_get_array(root_value);
        printf("ID -- TITLE\n");
        for (size_t i = 0; i < json_array_get_count(books); i++) {
            JSON_Object *book = json_array_get_object(books, i);
            printf("id: %d -- title: %s\n", (int)json_object_get_number(book, "id"), json_object_get_string(book, "title"));
        }
        json_value_free(root_value);
    }

    close_connection(sockfd);
    free(response);
}

void display_book_details(char *response) {
    // cautam inceputul response-ului JSON
    char *json_start = basic_extract_json_response(response);
    if (!json_start) {
        printf("ERROR: Expected a JSON object in response.\n");
        return;
    }

    // se parseaza JSON-ul si se extrag detaliile cartii
    JSON_Value *root_value = json_parse_string(json_start);
    if (!root_value || json_value_get_type(root_value) != JSONObject) {
        printf("ERROR , expected a JSON object.\n");
        json_value_free(root_value);
        return;
    }

    JSON_Object *book = json_value_get_object(root_value);
    if (book) {
        printf("id= %d\n", (int)json_object_get_number(book, "id"));
        printf("title= %s\n", json_object_get_string(book, "title"));
        printf("author= %s\n", json_object_get_string(book, "author"));
        printf("publisher= %s\n", json_object_get_string(book, "publisher"));
        printf("genre= %s\n", json_object_get_string(book, "genre"));
        printf("page_count= %d\n", (int)json_object_get_number(book, "page_count"));
    } else {
        printf("Failed to parse book details.\n");
    }

    // se elibereaza memoria
    json_value_free(root_value);
}

void get_book() {
    if (strlen(session_cookie) == 0 || strlen(bearer_token) == 0) {
        printf("ERROR , please log in and enter the library first.\n");
        return;
    }

    int book_id;
    printf("id=");
    scanf("%d", &book_id);

    int sockfd = open_connection(HOST, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    char message[BUFLEN];

    // se construieste GET requestul
    snprintf(message, sizeof(message), "GET /api/v1/tema/library/books/%d HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Cookie: %s\r\n"
             "Authorization: Bearer %s\r\n\r\n", 
             book_id, HOST, SERVER_PORT, session_cookie, bearer_token);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "200 OK")) {
        display_book_details(response);
    } else if (strstr(response, "404 Not Found")) {
        printf("ERROR: please login and enter the library, or provide a valid id\n");
    } 

    close_connection(sockfd);
    free(response);
}

void add_book() {
    if (strlen(session_cookie) == 0 || strlen(bearer_token) == 0) {
        printf("ERROR , no session or bearer token available, log in and enter the library first.\n");
        return;
    }

    char title[COMMAND_LEN], author[COMMAND_LEN], genre[COMMAND_LEN], publisher[COMMAND_LEN], page_count_str[COMMAND_LEN];
    int page_count;

    // se goleste bufferul pentru citire
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }

    printf("title= ");
    fgets(title, COMMAND_LEN, stdin);
    title[strcspn(title, "\n")] = 0;  // pentru newline

    printf("author= ");
    fgets(author, COMMAND_LEN, stdin);
    author[strcspn(author, "\n")] = 0;

    printf("genre= ");
    fgets(genre, COMMAND_LEN, stdin);
    genre[strcspn(genre, "\n")] = 0;

    // folosim atoi ca sa citim cu fgets, in cazul in care se introduce doar "ENTER"
    printf("page_count= ");
    fgets(page_count_str, COMMAND_LEN, stdin);
    page_count = atoi(page_count_str);

    printf("publisher= ");
    fgets(publisher, COMMAND_LEN, stdin);
    publisher[strcspn(publisher, "\n")] = 0;

    int sockfd = open_connection(HOST, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    char message[MESSAGE_SIZE], data[BUFLEN], content_length[100];

    sprintf(data, "title=%s&author=%s&genre=%s&page_count=%d&publisher=%s", title, author, genre, page_count, publisher);
    sprintf(content_length, "Content-Length: %ld", strlen(data));
    snprintf(message, sizeof(message), 
             "POST /api/v1/tema/library/books HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Content-Type: application/x-www-form-urlencoded\r\n"
             "%s\r\n"
             "Cookie: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "\r\n"
             "%s",
             HOST, SERVER_PORT, content_length, session_cookie, bearer_token, data);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (strstr(response, "200 OK")) {
        printf("200 - OK - Successfully added book.\n");
    } else {
        printf("ERROR - COuldn't to add book\n");
    }

    close_connection(sockfd);
    free(response);
}

void delete_book() {
    if (strlen(session_cookie) == 0 || strlen(bearer_token) == 0) {
        printf("ERROR: No session or bearer token available. Log in and enter the library first.\n");
        return;
    }

    int book_id;
    printf("id=");
    if (scanf("%d", &book_id) != 1) {
        printf("ERROR: Invalid book ID.\n");
        return;
    }

    // se goleste bufferul pentru citire
    while ((getchar()) != '\n');

    int sockfd = open_connection(HOST, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    char message[BUFLEN];
    snprintf(message, sizeof(message), "DELETE /api/v1/tema/library/books/%d HTTP/1.1\r\nHost: %s:%d\r\nCookie: %s\r\nAuthorization: Bearer %s\r\n\r\n", book_id, HOST, SERVER_PORT, session_cookie, bearer_token);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (strstr(response, "200 OK")) {
        printf("200 - OK - Successfully deleted book.\n");
    } else {
        printf("ERROR");
    }

    close_connection(sockfd);
    free(response);
}

void log_out() {
    if (strlen(session_cookie) == 0) {
        printf("ERROR , no session available, please log in first.\n");
        return;
    }

    int sockfd = open_connection(HOST, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    char message[BUFLEN];
    snprintf(message, sizeof(message), "GET /api/v1/tema/auth/logout HTTP/1.1\r\n");
    strcat(message, "Host: ");
    strcat(message, HOST);
    strcat(message, ":8080\r\n");
    strcat(message, "Cookie: ");
    strcat(message, session_cookie);
    strcat(message, "\r\n\r\n");

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    if (!strstr(response, "200 OK")) {
        printf("ERROR , failed to login.\n");
    } else {
        printf("200 - OK - Successfully logged out.\n");
        // se curata session cookie si bearer token
        memset(session_cookie, 0, sizeof(session_cookie));
        memset(bearer_token, 0, sizeof(bearer_token));
    }

    close_connection(sockfd);
    free(response);
}

int main() {
    char command[COMMAND_LEN];

    while (1) {
        if (scanf("%s", command) != 1) {
            printf("ERROR , couldn't read command\n");
            continue;
        }

        if (strcmp(command, "register") == 0) {
            register_user();

        } else if (strcmp(command, "login") == 0) {
            login_user();

        } else if (strcmp(command, "enter_library") == 0) {
            enter_library();

        } else if (strcmp(command, "get_books") == 0) {
            get_books();

        } else if (strcmp(command, "get_book") == 0) {
            get_book();

        } else if (strcmp(command, "add_book") == 0) {
            add_book();

        } else if (strcmp(command, "delete_book") == 0) {
            delete_book();

        } else if (strcmp(command, "logout") == 0) {
            log_out();

        } else if (strcmp(command, "exit") == 0) {
            break;
            
        } else {
            printf("ERROR , unknown command\n");
        }
    }

    return 0;
}
