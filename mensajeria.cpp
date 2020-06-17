/*
CHAT USANDO COMUNICACION ORIENTADA A CONEXION

La idea es que el cliente sea cliente-servidor (reciba y transmita mensajes)
por lo tanto se maneja esto ultimo mediante dos procesos (padre e hijo)
creados con fork()

SERVIDOR
    1 -- socket() https://www.man7.org/linux/man-pages/man2/socket.2.html
    2 -- bind()   https://www.man7.org/linux/man-pages/man2/bind.2.html
    3 -- listen() https://www.man7.org/linux/man-pages/man2/listen.2.html
    4 -- accept() https://man7.org/linux/man-pages/man2/accept.2.html
    5 -- read()  se va a usar recv() que recibe un mensaje de un socket 
                  https://www.man7.org/linux/man-pages/man2/recv.2.html   
    6 -- write() se va a usar send() que envia un mensaje en un socket
                  https://man7.org/linux/man-pages/man2/send.2.html

CLIENTE
    1 -- socket() El mismo que en servidor
    2 -- connect() https://man7.org/linux/man-pages//man2/connect.2.html
    3 -- write()  El mismo que en servidor
    4 -- read()   El mismo que en servidor
    5 -- close()  https://man7.org/linux/man-pages/man2/close.2.html

OTRAS OPERACIONES
    1 -- fork()   https://man7.org/linux/man-pages/man2/fork.2.html

*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h> 
#include <sys/wait.h>
#include <sys/signal.h>
#include "md5.h"
#include <errno.h>

#define MAX_LARGO_MENSAJE 255
#define MAX_USUARIOS_CONECTADOS 5

using namespace std;

// Declaracion de los file descriptores

int fd1_servidor; // File Descriptor Socket del servidor_tcp
int fd2_accept;   // File Descriptor Socket para luego del accept
int fd3_cliente;  // File Descriptor Socket del cliente_tcp
int fd4_auth;     // File Descriptor Socket del cliente AUTENTICACION
int fd5_udp;      // File Descriptor Socket broadcast
int numbytes;     // Para el recv



void resetChar(char*& s) {
    // Resetea un char.
    s[0] = '\0';
}

void resetString(string &s) {
    // Resetea un string
    s = "\0";
}

void resetStringNUM(char*& s) {
    // Resetea un string.
    s[numbytes - 2] = '\0';
}

char* agregarCero(char* cad, int num) {
// Chequea si el num es < 10 y me devuelve un string con el '0'
// concatenado con num en dicho caso
    char* aux = new char[25];
    resetChar(aux);
    strcat(aux, "0");
    sprintf(cad, "%d", num);
    if (num < 10) {
        strcat(aux, cad);
        return aux;
    }
    else {
        delete[] aux;
        return cad;
    }
}

char* getTiempo() {
// Obtiene la fecha y hora local y la almacena en un string
// con formato DD/MM/YYYY-hh:mm:ss
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    int hh = timeinfo->tm_hour;
    int mm = timeinfo->tm_min;
    int ss = timeinfo->tm_sec;
    int DD = timeinfo->tm_mday;
    int MM = (timeinfo->tm_mon) + 1; //xq enero = 0
    int YYYY = (timeinfo->tm_year) + 1900; //xq el año es desde 1900
    char* s = new char[19];
    resetChar(s);
    char* cad = new char[25];
    resetChar(cad);
    //chequeo si alguna es menor q 10 para concatenarle un '0'
    cad = agregarCero(cad, DD);
    strcat(s, cad);
    strcat(s, "/");
    cad = agregarCero(cad, MM);
    strcat(s, cad);
    strcat(s, "/");
    cad = agregarCero(cad, YYYY);
    strcat(s, cad);
    strcat(s, "-");
    cad = agregarCero(cad, hh);
    strcat(s, cad);
    strcat(s, ":");
    cad = agregarCero(cad, mm);
    strcat(s, cad);
    strcat(s, ":");
    cad = agregarCero(cad, ss);
    strcat(s, cad);
    return s;
}

void manejadorHijo(int signal){
// Manejador de las senhales del hijo.
    if (signal == SIGINT) {
        cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGINT CTRL+C recibido\33[00m\n";
    }
    if (signal == SIGTERM) {
        cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGTERM Terminacion de programa\33[00m\n";
    }
    if (signal == SIGSEGV) {
        cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGSEGV violacion de segmento\33[00m\n";
    }
    if (signal == SIGPIPE) {
        cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGPIPE \33[00m\n";
    }
    //if (signal == SIGALRM) {
       // cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGALRM Timeout excedido \33[00m\n";
    //}
    close(fd1_servidor);
    close(fd2_accept);
    close(fd5_udp);
    exit(1);
}

void manejadorPadre(int signal)
// Manejador de las senhales del padre.
{
    if (signal == SIGINT) {
        cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGINT CTRL+C recibido\33[00m\n";
    }
    if (signal == SIGTERM) {
        cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGTERM Terminacion de programa\33[00m\n";
    }
    if (signal == SIGSEGV) {
        cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGSEGV violacion de segmento\33[00m\n";
    }
    if (signal == SIGPIPE) {
        cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGPIPE \33[00m\n";
    }
    close(fd3_cliente);
    exit(1);

}


int main(int argc, char* argv[]) {
    // En argc viene la cantidad de argumentos que se pasan,
    // si se llama solo al programa, el nombre es el argumento 0
    // En nuestro caso: (segun la letra) -> string port ipAuth portAuth
    //      - argv[0] es el string "mensajeria", puede cambiar si se llama con otro path.
    //      - argv[1] es el puerto que escuchara el receptor.
    //      - argv[2] es el ipAuth para conectarse.
    //      - argv[3] es el portAuth para conectarse.

    struct sockaddr_in servidor; // donde se cargan los datos del servidor
    struct sockaddr_in cliente_recibido; // donde se cargan los datos del cliente que se conecte
    struct sockaddr_in cliente_enviado; // donde se guardan los datos del servidor a conectarse.
    struct hostent* he; // Estructura para guardar los datos de una consulta dns (gethosybyname())
    char* buf = new (char[MAX_LARGO_MENSAJE]); // para guardar lo q llega en recv();
    char* mensaje_recibido = new (char[MAX_LARGO_MENSAJE]); //Mensaje recibido del cliente
    char* mensaje_broad_recibido = new(char[MAX_LARGO_MENSAJE]); // Mensaje recibido por broadcast
    string mensaje_boradcast; //Mensaje enviado por broadcast
    string mensaje_enviado; //Mensaje enviado al cliente;
    string dirIp;
    char* ipport = new(char[50]);
    char* puerto = new(char[25]);
    int on = 1;  // Para el setsockopt
    int p;  // Para el fork
    int timeout = 45;
    
    resetChar(ipport);
    resetChar(puerto);

    string user, passmd5;
    cout << "Ingrese usuario: ";
    cin >> user;
    cout << "\nIngrese password: ";
    cin >> passmd5;
    passmd5 = md5(passmd5);  // codificación a md5
    string up = user + '-' + passmd5;   // concadenación de string

    if (argc < 3)
    {
        cout << "[ERROR]:" << " Faltan argumentos: port, ipAuth, portAuth.";
        exit(0);
    }

    // Estructuras para el manejo de Senhales
    // Deberia haber un manejador de senhales para cada hijo si hacen cosas distintas

    // *********************************
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &manejadorPadre;

    struct sigaction sb;
    memset(&sb, 0, sizeof(sb));
    sb.sa_handler = &manejadorHijo;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGKILL, &sa, NULL);
    signal(SIGALRM, SIG_IGN);
    // **********************************

    // Autenticador
    
    cout << "\33[34mRedes de Computadoras\33[39m: Autenticacion\nEscuchando en el puerto " << argv[3] << "\n";


    if ((he = gethostbyname(argv[2])) == NULL) {
        printf("[ERROR]: gethostbyname()\n");
        exit(0);
    }

    if ((fd4_auth = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cout << "[ERROR]: socket()\n";
        exit(-1);
    }

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(atoi(argv[3]));
    servidor.sin_addr.s_addr = INADDR_ANY;
    bzero(&(servidor.sin_zero), 8);

  
    if (connect(fd4_auth, (struct sockaddr*)&servidor, sizeof(struct sockaddr)) == -1) {
        printf("[ERROR]: connect()");
        exit(0);
    }
   
    if ((numbytes = recv(fd4_auth, buf, MAX_LARGO_MENSAJE, 0)) == -1) {
        printf("[ERROR]: recv()\n");
        exit(0);
    }

    resetStringNUM(buf);

    if (strcmp(buf, "Redes 2020 - Taller 3 - Autenticacion de Usuarios") != 0) {
        printf("[ERROR]: Protocolo Incorrecto(1).\n");
        exit(0);
    }

    send(fd4_auth, up.c_str(), 255, 0);
    if ((numbytes = recv(fd4_auth, buf, MAX_LARGO_MENSAJE, 0)) == -1) {
        printf("ERROR]: recv()\n");
        exit(0);
    }
    resetStringNUM(buf);
    cout << buf << endl;

    if (strcmp(buf, "SI") == 0) {
        if ((numbytes = recv(fd4_auth, buf, MAX_LARGO_MENSAJE, 0)) == -1) {
            printf("[ERROR]: recv()\n");
            exit(0);
        }
        resetStringNUM(buf);
        cout << "Bienvenido " << buf << endl;
    }
    else {
        cout << "Usuario incorrecto." << endl;
        exit(0);
    }
    close(fd4_auth);
    // termina autenticador

    cout << "\33[34mRedes de Computadoras\33[39m: Servicio de Mensajeria\nEscuchando en el puerto " << argv[1] << ".\nProceso de pid: " << getpid() << ".\n";
     
    // Bifurcamos el codigo
    p = fork();
    if (p < 0) {
        printf("\33[46m\33[31m[ERROR]: fork()\33[00m\n");
        exit(0);
    }

    if(p > 0){  //proceso padre "cliente tcp"
        while(true){             
            sleep(1);        
            cout<<"Ingrese una ip\n";	
			cin >> dirIp;
            if (dirIp == "*") { // broadcasting UDP
                struct sockaddr_in send_addr, recv_addr;
                int trueflag = 1;

                // socket 
                if ((fd5_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
                    cout << "\33[46m\33[31m[ERROR]: socket(udp)\33[00m\n";

                #ifndef SEND_ONLY
                    // socket opt broadcast
                    if (setsockopt(fd5_udp, SOL_SOCKET, SO_BROADCAST, &trueflag, sizeof trueflag) < 0)
                        cout << "\33[46m\33[31m[ERROR]: socketopt(udp_rcv)\33[00m\n";

                    // Se cargan los datos de la estructura que enviará datos
                    memset(&send_addr, 0, sizeof send_addr);
                    send_addr.sin_family = AF_INET;
                    send_addr.sin_port = htons(atoi(argv[1]));
                    // broadcasting address netmask 24
                    inet_aton("192.168.1.255", &send_addr.sin_addr);
                    //send_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
                #endif // ! SEND_ONLY

                #ifndef RECV_ONLY
                    // libera socket para ser reutilizado
                    if (setsockopt(fd5_udp, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof trueflag) < 0)
                        cout << "\33[46m\33[31m[ERROR]: socketopt(udp_send)\33[00m\n";

                    // Se cargan los datos de la estructura que recibirá datos
                    memset(&recv_addr, 0, sizeof recv_addr);
                    recv_addr.sin_family = AF_INET;
                    recv_addr.sin_port = htons(atoi(argv[1]));
                    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

                    // llamada a bind
                    if (bind(fd5_udp, (struct sockaddr*)&recv_addr, sizeof recv_addr) < 0)
                        cout << "\33[46m\33[31m[ERROR]: bind(udp)\33[00m\n";
                #endif // ! RECV_ONLY

                #ifndef SEND_ONLY
                    sleep(1);
                    cout << "Ingrese el mensaje que será enviado\n";
                    getline(cin >> ws, mensaje_boradcast); // para tomar los espacios del std in
                    
                    // llamada a sendto
                    if (sendto(fd5_udp, mensaje_boradcast.c_str(), MAX_LARGO_MENSAJE, 0, (struct sockaddr*)&send_addr, sizeof send_addr) < 0)
                        cout << "\33[46m\33[31m[ERROR]: send(udp)\33[00m\n";
                    sleep(1);
                #endif // ! SEND_ONLY

                #ifndef RECV_ONLY
                    resetChar(puerto);
                    printf("conexion recibida desde > %s:%d\n", inet_ntoa(recv_addr.sin_addr), ntohs(recv_addr.sin_port));
                    sprintf(puerto, "%d", ntohs(recv_addr.sin_port));  // para guardar los datos del puerto del cliente
                    strcat(strcat(strcat(strcat(strcat(ipport, "\33[31m[\33[34m"), inet_ntoa(recv_addr.sin_addr)), "\33[00m:"), puerto), "\33[31m]\33[00m ");  // concadena ip y puerto
                    
                    //llmada a recv
                    if ((numbytes = recv(fd5_udp, mensaje_broad_recibido, MAX_LARGO_MENSAJE, 0)) < 0)
                        cout << "\33[46m\33[31m[ERROR]: rcv(udp)\33[00m\n";
                    sleep(1);
                    resetStringNUM(mensaje_broad_recibido);
                    cout << "\33[31m[\33[00m" << getTiempo() << "\33[31m-\33[00m" << ipport << "\33[33m<" << " dice: " << mensaje_broad_recibido << "\33[00m\n";
                    resetChar(ipport);
                #endif // ! RECV_ONLY
                close(fd5_udp);
             } else { // recibe una ip o localhost*/
                string ip = dirIp;  // se copia string para liberar buffer colgado

                // socket
                fd3_cliente = socket(AF_INET, SOCK_STREAM, 0);
                if (fd3_cliente < 0) {
                    cout << "\33[46m\33[31m[ERROR]: socket(c)\33[00m\n";
                    exit(-1);
                }    
                if ((he = gethostbyname(ip.c_str())) == NULL) {
                    printf("\33[46m\33[31m[ERROR]: gethostbyname(c)\33[00m\n");
                    exit(-1);
                }
               // Para ver que da el gethostbyname() 
                printf("Nombre del host: %s\n", he->h_name);
                printf("Dirección IP: %s\n", inet_ntoa(*((struct in_addr*)he->h_addr)));
                //resetString(dirIp);
                cliente_enviado.sin_family = AF_INET;
                cliente_enviado.sin_port = htons(atoi(argv[1]));
                cliente_enviado.sin_addr.s_addr = INADDR_ANY;
                bzero(&(cliente_enviado.sin_zero), 8);

                if (connect(fd3_cliente, (struct sockaddr*)&cliente_enviado, sizeof(struct sockaddr)) == -1) {
                    cout << "\33[46m\33[31m[ERROR]: connect(c)\33[00m\n";
                    exit(0);
                }
                sleep(1);
                cout << "Ingrese el mensaje que será enviado\n";
                getline(cin >> ws, mensaje_enviado); // para tomar los espacios del std in
                send(fd3_cliente, mensaje_enviado.c_str(), MAX_LARGO_MENSAJE, 0);
                close(fd3_cliente);
            }            
        }
    }
    if(p == 0){ // proceso hijo "servidor"
        // señales hijo
        sigaction(SIGINT, &sb, NULL);
        sigaction(SIGTERM, &sb, NULL);
        sigaction(SIGPIPE, &sb, NULL);
        sigaction(SIGSEGV, &sb, NULL);

        // socket
        fd1_servidor = socket(AF_INET, SOCK_STREAM, 0);
        if (fd1_servidor < 0) {
            cout << "\33[46m\33[31m[ERROR]: socket(s)\33[00m\n";
            exit(0);
        }
        
        // Se cargan los datos del servidor, ip escucha, puerto.
        servidor.sin_family = AF_INET;
        servidor.sin_port = htons(atoi(argv[1]));
        servidor.sin_addr.s_addr = INADDR_ANY;
        bzero(&(servidor.sin_zero), 8);

        // Libera el socket inmediatamente para ser reutilizado
        setsockopt(fd1_servidor, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));

        // llamada a bind()
        int error = bind(fd1_servidor, (struct sockaddr*)&servidor, sizeof(struct sockaddr));
        if (error < 0) {
            cout << "\33[46m\33[31m[ERROR]: bind(s)\33[00m\n";
            exit(-1);
        }

        //sigaction(SIGALRM, &sb, NULL);
        //llamada a listen
        error = listen(fd1_servidor, MAX_USUARIOS_CONECTADOS);
        if (error < 0) {
            printf("\33[46m\33[31m[ERROR]: listen(s)\33[00m\n");
            exit(-1);
        }
       
        while(true){
            unsigned int sin_size = sizeof(struct sockaddr_in);

            //llamada a accept
            fd2_accept = accept(fd1_servidor, (struct sockaddr*)&cliente_recibido, &sin_size);
            if (fd2_accept < 0) {
                printf("\33[46m\33[31m[ERROR]: accept(s)\33[00m\n");
                exit(0);
            }
            resetStringNUM(mensaje_recibido);
            resetStringNUM(puerto);
            printf("conexion recibida desde > %s:%d\n", inet_ntoa(cliente_recibido.sin_addr), ntohs(cliente_recibido.sin_port));
            sprintf(puerto, "%d", ntohs(cliente_recibido.sin_port));    // para guardar el puerto del cliente
            strcat(strcat(strcat(strcat(strcat(ipport, "\33[31m[\33[34m"), inet_ntoa(cliente_recibido.sin_addr)), "\33[00m:"), puerto), "\33[31m]\33[00m ");
            
            // llamada  a recv
            if ((numbytes = recv(fd2_accept, mensaje_recibido, MAX_LARGO_MENSAJE, 0)) == -1){
                printf("\33[46m\33[31m[ERROR]: recv(s)\33[00m\n");
                exit(-1);
            }
            resetStringNUM(mensaje_recibido);
            alarm(0); 
            alarm(timeout);
            sleep(1);
            cout << "\33[31m[\33[00m" << getTiempo() << "\33[31m-\33[00m" << ipport << "\33[33m<" << " dice: " << mensaje_recibido << "\33[00m\n";
            resetChar(ipport);
            close(fd2_accept);
        }
        
    }
    exit(0);
 }
