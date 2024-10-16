#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "buffer.h"
#include "requests.h"
#include "parson.h"

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void compute_message(char *message, const char *line)
{
    strcat(message, line);
    strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    inet_aton(host_ip, &serv_addr.sin_addr);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    return sockfd;
}

void close_connection(int sockfd)
{
    close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
    int bytes, sent = 0;
    int total = strlen(message);

    do
    {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            error("ERROR writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd)
{
    char response[BUFLEN];
    buffer buffer = buffer_init();
    int header_end = 0;
    int content_length = 0;

    do {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0){
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
        
        header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

        if (header_end >= 0) {
            header_end += HEADER_TERMINATOR_SIZE;
            
            int content_length_start = buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);
            
            if (content_length_start < 0) {
                continue;           
            }

            content_length_start += CONTENT_LENGTH_SIZE;
            content_length = strtol(buffer.data + content_length_start, NULL, 10);
            break;
        }
    } while (1);
    size_t total = content_length + (size_t) header_end;
    
    while (buffer.size < total) {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0) {
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
    }
    buffer_add(&buffer, "", 1);
    return buffer.data;
}

char *basic_extract_json_response(char *str)
{
    return strstr(str, "{\"");
}

char *find_json_value(const char *json_str, const char *key) {
    // locate the position of the key in the JSON string
    char *key_pos = strstr(json_str, key);
    if (!key_pos)
        return NULL;

    // find the colon that follows the key
    char *value_start = strchr(key_pos, ':');
    if (!value_start)
        return NULL;
    value_start += 1;

    // skip any spaces or quotation marks that precede the value
    while (*value_start == ' ' || *value_start == '"') value_start++;

    // find the end of the value, which can be a comma, quote, or closing brace
    char *value_end = strpbrk(value_start, "\",}");
    if (!value_end) 
        return NULL;

    // calculate the length of the value
    size_t value_len = value_end - value_start;
    
    // allocate memory for the value and copy it
    char *value = malloc(value_len + 1);
    if (!value) 
        return NULL;

    strncpy(value, value_start, value_len);
    value[value_len] = '\0';

    return value;
}

char *extract_json_list(char *str) {
    // find the opening bracket of the JSON list
    return strstr(str, "[");
}

// function to parse a command and return the corresponding command type
Command parse_command(const char *command) {
    if (strcmp(command, "register") == 0)
        return CMD_REGISTER;

    if (strcmp(command, "login") == 0)
        return CMD_LOGIN;

    if (strcmp(command, "enter_library") == 0)
        return CMD_ENTER_LIBRARY;

    if (strcmp(command, "add_book") == 0)
        return CMD_ADD_BOOK;

    if (strcmp(command, "delete_book") == 0)
        return CMD_DELETE_BOOK;
    
    if (strcmp(command, "get_book") == 0)
        return CMD_GET_BOOK;
    
    if (strcmp(command, "get_books") == 0)
        return CMD_GET_BOOKS;

    if (strcmp(command, "logout") == 0)
        return CMD_LOGOUT;

    if (strcmp(command, "exit") == 0)
        return CMD_EXIT;

    return CMD_UNKNOWN;
}

// function to handle a command based on the parsed command type
void handle_command(const char *command, char **cookie, char **token) {
    switch (parse_command(command)) {
        case CMD_REGISTER:
            register_user();
            break;

        case CMD_LOGIN:
            *cookie = login_user(cookie);
            break;

        case CMD_ENTER_LIBRARY:
            *token = enter_library(*cookie);
            break;

        case CMD_ADD_BOOK:
            add_book(*token);
            break;

        case CMD_GET_BOOK:
            get_book(*token);
            break;

        case CMD_GET_BOOKS:
            get_books(*token);
            break;
            
        case CMD_DELETE_BOOK:
            delete_book(*token);
            break;

        case CMD_LOGOUT:
            logout_user(cookie);
            break;

        case CMD_EXIT:
            break;

        default:
            printf("Unknown command!\n");
            break;
    }
}

// check space
int contains_space(const char *str) {
    while (*str) {
        if (*str == ' ') return 1;
        str++;
    }
    return 0;
}

// function to get user credentials (username and password)
int get_credentials(char *username, char *password) {
    // prompt for and read the username
    printf("username=");
    fgets(username, BUFLEN - 1, stdin); // remove newline character
    username[strlen(username) - 1] = 0;
    if (contains_space(username)) {
        printf("ERROR! Username should not contain spaces.\n");
        return 0; // indicate failure
    }
    
    // prompt for and read the password
    printf("password=");
    fgets(password, BUFLEN - 1, stdin);
    password[strlen(password) - 1] = 0; // remove newline character
    if (contains_space(password)) {
        printf("ERROR! Password should not contain spaces.\n");
        return 0; // indicate failure
    }

    return 1; // indicate success
}

char *prepare_payload(const char *username, const char *password) {
    // initialize a new JSON object
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    
    // set the username and password fields in the JSON object
    json_object_set_string(obj, "username", username);
    json_object_set_string(obj, "password", password);
    
    // serialize the JSON object to a string
    char *payload = json_serialize_to_string_pretty(val);
    
    // free the JSON value as it's no longer needed
    json_value_free(val);
    
    return payload;
}

// function to send a request to the given endpoint with the provided payload
char *send_request(const char *endpoint, const char *payload) {
    // open a connection to the server
    int sockfd = open_connection(IPSERVER, PORT, AF_INET, SOCK_STREAM, 0);
    char *type = "application/json";
    char *data[1] = {payload};
    
    // create and send the POST request
    char *message = compute_post_request(IPSERVER, endpoint, type, data, 1, NULL, 0);
    send_to_server(sockfd, message);
    
    // receive the server's response
    char *response = receive_from_server(sockfd);
    
    // close the connection
    close_connection(sockfd);
    
    return response;
}

// function to register a new user
char *register_user() {
    char username[BUFLEN], password[BUFLEN];
    
    // get user credentials
    if (!get_credentials(username, password)) {
        return NULL; // exit if credentials are invalid
    }
    
    // prepare the JSON payload
    char *payload = prepare_payload(username, password);
    
    // send the registration request
    char *response = send_request("/api/v1/tema/auth/register", payload);
    
    // free the payload memory
    free(payload);
    
    // check for errors in the response
    if (strchr(response, '{') != NULL) {
        char *res = basic_extract_json_response(response);
        char *error = find_json_value(res, "error");
        if (error != NULL) {
            printf("ERROR! %s\n", error);
            free(error);
            return NULL;
        }
    } else {
        printf("SUCCESS! User registered!\n");
    }
    
    return NULL;
}

// function to log in a user
char *login_user(char **cookie) {
    // check if the user is already logged in
    if (*cookie != NULL) {
        printf("ERROR! User already logged in!\n");
        return *cookie;
    }

    char username[BUFLEN], password[BUFLEN];
    
    // get user credentials
    if (!get_credentials(username, password)) {
        return NULL; // exit if credentials are invalid
    }
    
    // prepare the JSON payload
    char *payload = prepare_payload(username, password);
    
    // send the login request
    char *response = send_request("/api/v1/tema/auth/login", payload);
    
    // free the payload memory
    free(payload);
    
    // check for errors in the response
    if (strchr(response, '{') != NULL) {
        char *res = basic_extract_json_response(response);
        char *error = find_json_value(res, "error");
        if (error != NULL) {
            printf("ERROR! %s\n", error);
            free(error);
            return NULL;
        }
    } else {
        printf("SUCCESS! User logged in!\n");
        
        // extract the cookie from the response
        char *cookie_start = strstr(response, "Set-Cookie: ");
        if (cookie_start) {
            cookie_start += strlen("Set-Cookie: ");
            char *cookie_end = strstr(cookie_start, "\r\n");
            if (cookie_end) {
                size_t cookie_len = cookie_end - cookie_start;
                *cookie = malloc(cookie_len + 1);
                if (*cookie) {
                    strncpy(*cookie, cookie_start, cookie_len);
                    (*cookie)[cookie_len] = '\0';
                }
            }
        }
    }

    return *cookie;
}

// function to enter the library with a given cookie
char *enter_library(char *cookie) {
    int sockfd;
    char *message, *response;
    
    if (cookie == NULL) {
        printf("ERROR! No user logged in!\n");
        return NULL;
    }

    // open a connection to the server
    sockfd = open_connection(IPSERVER, PORT, AF_INET, SOCK_STREAM, 0);
    
    // create and send the GET request to enter the library
    message = compute_get_request(IPSERVER, "/api/v1/tema/library/access", NULL, &cookie, 1);
    send_to_server(sockfd, message);
    
    // receive the server's response
    response = receive_from_server(sockfd);
    
    // close the connection
    close_connection(sockfd);
    
    // extract and check for errors in the response
    char *res = basic_extract_json_response(response);
    char *error = find_json_value(res, "error");
    if (error != NULL) {
        printf("ERROR! %s\n", error);
        free(error);
        return NULL;
    }

    printf("SUCCESS. User entered library!\n");
    
    // extract and return the token from the response
    char *token = find_json_value(res, "token");
    return token;
}

// function to log out a user
void logout(char *cookie) {
    int sockfd;
    char *message, *response;

    if (cookie == NULL) {
        printf("ERROR! No user is currently logged in.\n");
        return;
    }
    
    // open a connection to the server
    sockfd = open_connection(IPSERVER, PORT, AF_INET, SOCK_STREAM, 0);
    
    // create and send the GET request to log out
    message = compute_get_request(IPSERVER, "/api/v1/tema/auth/logout", NULL, &cookie, 1);
    send_to_server(sockfd, message);
    
    // receive the server's response
    response = receive_from_server(sockfd);

    // check for errors in the response
    if (strchr(response, '{') != NULL) {
        char *res = basic_extract_json_response(response);
        char *error = find_json_value(res, "error");
        if (error != NULL) {
            printf("ERROR! %s\n", error);
            free(error);
        }
    } else {
        printf("SUCCESS! User log out!\n");
    }

    // close the connection
    close_connection(sockfd);
}


void logout_user(char **cookie) {
    // log out a user and clear the cookie
    logout(*cookie);
    *cookie = NULL;
}

void get_book(char *token) {
    printf("not implemented\n");
}

void get_books(char *token) {
    printf("not implemented\n");
}

void add_book(char *token) {
    printf("not implemented\n");
}

void delete_book(char *token) {
    printf("not implemented\n");
}
