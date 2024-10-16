#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000

#define IPSERVER "34.246.184.49"
#define PORT 8080

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

// enum to represent various command types
typedef enum {
    CMD_REGISTER,
    CMD_LOGIN,
    CMD_ENTER_LIBRARY,
    CMD_ADD_BOOK,
    CMD_GET_BOOK,
    CMD_GET_BOOKS,
    CMD_DELETE_BOOK,
    CMD_LOGOUT,
    CMD_EXIT,
    CMD_UNKNOWN
} Command;

// handles the command entered by the user
void handle_command(const char *command, char **cookie, char **token);

// parses the command string and returns the corresponding Command enum
Command parse_command(const char *command);

// extracts and returns a JSON list from a server response
char *extract_json_list(char *str);

// finds and returns a value from a JSON string for a given key
char *find_json_value(const char *json_str, const char *key);

// prompts the user for username and password, checking for spaces
int get_credentials(char *username, char *password);

// prepares a JSON payload with the given username and password
char *prepare_payload(const char *username, const char *password);

// sends a POST request to the given endpoint with the provided payload
char *send_request(const char *endpoint, const char *payload);

// checks if a string contains any spaces
int contains_space(const char *str);

char *register_user();

char *login_user(char **cookie);

char *enter_library(char *cookie);

void add_book(char *token);

void get_book(char *token);

void get_books(char *token);

void delete_book(char *token);

void logout(char *cookie);

void logout_user(char **cookie);

#endif
