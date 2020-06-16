// FIng/CETP - Tecnologo en Informatica
// Redes - Servidor de Autenticacion

using namespace std;

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <arpa/inet.h> // para inet_Addr, etc
#include <netdb.h> // estrucrutas
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h> 
#include <sys/wait.h>
#include <sys/signal.h>
#include <ctype.h> 

#define BACKLOG 2	// El numero de conexiones permitidas


//VARIABLES GLOBALES
int fd,fd2;
//ListaPid listaPid; 			//lista con los pids de los hijos
char * nombre = new (char[75]);		// para acordarse del usuario despues de una orden user

//***** FUNCIONES AUXILIARES **********

void resetString (char * & s)
// Resetea un string.
{
	s[0] = '\0';
}

void leerCedula(char * & buffer, char * & cedula)
// lee una cedula
{
	strncpy (cedula, buffer, strlen(buffer));
	cedula[strlen(buffer)] = '\0';
}

void leerComando(char * & buffer, char * & comando)
// lee un comando
{
	strncpy (comando, buffer, strlen(buffer));
	comando[strlen(buffer)] = '\0';
}

char * agregarCero(char * cad,int num)
// Chequea si el num es < 10 y me devuelve un string con el '0'
// concatenado con num en dicho caso
{
	char * aux = new char[25];
	resetString(aux);
	strcat(aux,"0");
	sprintf(cad,"%d",num);
	if(num<10){
		strcat(aux,cad);
		return aux;
	}
	else{
		delete[] aux;
		return cad;
	}
}

char * getTiempo()
// Obtiene la fecha y hora local y la almacena en un string
// con formato DD/MM/YYYY-hh:mm:ss
{
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	int hh = timeinfo -> tm_hour;
	int mm = timeinfo -> tm_min;
	int ss = timeinfo -> tm_sec;
	int DD = timeinfo -> tm_mday;
	int MM = (timeinfo -> tm_mon) + 1; //xq enero = 0
	int YYYY = (timeinfo -> tm_year) + 1900 ; //xq el año es desde 1900
	char * s = new char[19];
	resetString(s);
	char * cad = new char[25];
	resetString(cad);
	//chequeo si alguna es menor q 10 para concatenarle un '0'
	cad = agregarCero(cad,DD);
	strcat(s,cad);
	strcat(s,"/");
	cad = agregarCero(cad,MM);
	strcat(s,cad);
	strcat(s,"/");
	cad = agregarCero(cad,YYYY);
	strcat(s,cad);
	strcat(s,"-");
	cad = agregarCero(cad,hh);
	strcat(s,cad);
	strcat(s,":");
	cad = agregarCero(cad,mm);
	strcat(s,cad);
	strcat(s,":");
	cad = agregarCero(cad,ss);
	strcat(s,cad);
	return s;
}

void enviar (int descriptor, char * cadena, char * ipport)
// Envia una cadena por un socket, e imprime en la salida estandar.
{
	send(descriptor, cadena, strlen(cadena),0); 
	cout << "\33[31m[\33[00m" << getTiempo() << "\33[31m-\33[00m" << ipport << "\33[36m>" << cadena << "\33[00m";
}

bool alumno(char * alumno, char * & sede)
// Devuelve los datos de un alumno
{
	FILE * archivo;
	bool retorno = false;
	bool termino = false;
	archivo = fopen ("./usuarios.conf", "r");
	char * esta;
	char * linea = new (char[255]);
	linea[0] = '\0';
	char * usps = new (char[255]);
	usps[0] = '\0';
	strcat (usps, alumno);
	strcat (usps, ";");
	if (!(archivo == NULL))
	{
		//cout << "leyendo archivo\n";
		while (!termino)
		{
			fscanf (archivo, "%s[^\n]", linea);
			//cout << linea << "\n";
			esta = strstr (linea, usps);
			termino = feof (archivo);
			if (!(esta == NULL))
			{
				termino = true;
				retorno = true;
				esta = &linea[strlen(usps)];
				char * ptr;
				ptr = strtok(&linea[strlen(usps)], ";");
				strcpy (sede, ptr);
				ptr = strtok(NULL, ";");
			}
			else
			{
//				cout << "no esta en este renglon\n";
			}
		}
	}
	delete [] linea;
	delete [] usps;
	fclose (archivo);
	return retorno;
}

void manejadorHijo(int signal)
 // Manejador de las senhales del hijo.
{
	if (signal == SIGINT){
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGINT CTRL+C recibido\33[00m\n"; 
	}
	if (signal == SIGTERM){
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGTERM Terminacion de programa\33[00m\n";
	}
	if (signal == SIGSEGV){
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGSEGV violacion de segmento\33[00m\n";
	}
	if (signal == SIGPIPE){
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGPIPE \33[00m\n";
	}
	if (signal == SIGALRM){
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGALRM Timeout excedido \33[00m\n";
	}
	close(fd2);
	close(fd);
	exit(1);
}

void manejadorPadre (int signal)
// Manejador de las senhales del padre.
{
	if (signal == SIGINT){
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGINT CTRL+C recibido\33[00m\n"; 
	}
	if (signal == SIGTERM){
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGTERM Terminacion de programa\33[00m\n";
	}
	if (signal == SIGSEGV){
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGSEGV violacion de segmento\33[00m\n";
	}
	if (signal == SIGCHLD){
		int status;
		int pid_hijo = wait(&status);
		cout << "\33[46m\33[31m[" << pid_hijo << "]" << " SIGCHLD \33[00m\n";
		return;
	}
	if (signal == SIGPIPE){
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGPIPE \33[00m\n";
	}
	close(fd2);
	close(fd); 
	exit(1);
	
}

//************ MAIN ***************
//*********************************
//*********************************
int main(int argc, char * argv[])
// argv[1] = puerto
{

 	struct sigaction sa;//////////////////////
	memset (&sa, 0, sizeof (sa));/////////////
	sa.sa_handler = &manejadorPadre;//////////	
	
	struct sigaction sb;//////////////////////
	memset (&sb, 0, sizeof (sb));/////////////
	sb.sa_handler = &manejadorHijo;///////////
	sigaction(SIGINT, &sa, NULL);/////////////
	sigaction(SIGTERM, &sa, NULL);////////////
	sigaction(SIGPIPE, &sa, NULL);////////////
	sigaction(SIGSEGV, &sa, NULL);////////////
	signal(SIGALRM, SIG_IGN);/////////////////

	cout << "\n\n\t>> \33[34mAutenticacion Redes \33[00m<<\n";
	cout << "\t  ---------------------\n\n";
	if (argc < 2)
	{
		cout << "\33[31mERROR:\33[00m Ingrese puerto\n\n";
		exit(-1);
	}
	
	//CREO ESTRUCTURAS Y VARIABLES
	enum estados {esperando, autorizacion, transaccion, actualizacion};
	estados estado = esperando;
	struct sockaddr_in server;	// para la información de la dirección del servidor
	struct sockaddr_in client;	// para la información de la dirección del cliente
	socklen_t sin_size;		// Tamaño del socket, era un int,
					// pero con g++ tuve que poner esta estructura
	char buffer[255];		// buffer para almacenar lo que recibo
	char * todo;
	char * comando = new (char[100]);	// el comando leido
	char * nombre = new (char[100]);	// el nombre leido
	char * sede = new (char[100]);		// la sede leida
	char * palabra = new (char[100]);	// la palabra leida
	char * argumento = new (char[100]);	// argumento del comando, puede ser mas de 1
	char * mensaje = new (char[255]);	// mensaje
	char * envio = new (char[255]);
	char * ipport = new(char[50]);
	char * puerto = new(char[25]);
	buffer[0] = '\0';			// resetString (buffer);
	resetString (comando);
	resetString (argumento);
	resetString (nombre);
	resetString (sede);	
	resetString (palabra);
	resetString (mensaje);
	resetString (envio);
	resetString (ipport);
	resetString (puerto);
	int numbytes;			// cantidad de bytes recibidos en recv()
	int pidHijo;
	int on=1;
	int timeout = 45;	
	// A continuacion la llamada a socket()
	if ((fd=socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		cout << "error en socket()\n";
		exit(-1);
	}
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1])/*PORT*/);	// htons() de la seccion "Conversiones"

	//if ((strcmp(argv[1], "localhost") == 0) || (strcmp(argv[1], "LOCALHOST") == 0))
	//	inet_aton("127.0.0.1", &server.sin_addr);
	//else
	//	inet_aton(argv[1], &server.sin_addr);
	//INADDR_ANY coloca nuestra direccion IP automaticamente
	server.sin_addr.s_addr = INADDR_ANY;		
	bzero(&(server.sin_zero),8); 			// escribimos ceros en el resto de la estructura
 	//libera el socket inmediatamente para ser reutilizado
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));
	// A continuacion la llamada a bind()
	if(bind(fd,(struct sockaddr*)&server, sizeof(struct sockaddr))==-1)
	{
		cout << "error en bind() \n";
		exit(-1);
	}
	if(listen(fd,BACKLOG) == -1)	// llamada a listen()
	{
		cout << "error en listen()\n";
		exit(-1);
	}
	while(1)
	{
		bool transicion = false;
		if (estado == esperando)
		{
			//cout << "Esperando....\n";

//cout << "1\n";
			sin_size = sizeof(struct sockaddr_in);
			// A continuacion la llamada a accept()
			if ((fd2 = accept(fd,(struct sockaddr *)&client, &sin_size))==-1)
			{
				cout << "error en accept()\n";
				exit(-1);
			}
			//cout << "Se obtuvo una conexion desde "<< inet_ntoa(client.sin_addr) << " \n";
			//obtengo [IP:PUERTO]
			resetString(ipport);
//cout << "2\n";			
			sprintf(puerto,"%d", client.sin_port);
//cout << puerto << "\n";
//cout << "3\n";			
			strcat(strcat(strcat(strcat(strcat (ipport, /*"\33[31m[\33[34m"*/""), inet_ntoa(client.sin_addr)), "\33[00m:"), puerto), "\33[31m]\33[00m ");
//cout << "4\n";
//client.sin_addr;
//	strcat(strcat(strcat(strcat(strcat (ipport, "["), inet_ntoa(client.sin_addr)), ":"), argv[2]), "]\t");
//sprintf(cantmensajes,"%d",cantarch);
			enviar (fd2, "Redes 2020 - Taller 3 - Autenticacion de Usuarios\r\n", ipport);
			estado = autorizacion;
			pidHijo = fork(); // ******** FORK *************
	
		}
		if ( pidHijo < 0 )
		{
			cout << "Error en fork() \n" ;
			exit(-1);
		}
		if( pidHijo > 0 )
		{ //PROCESO PADRE
			
			estado = esperando;
//			cout << "Soy Papucho: " << getpid() << " fd: " << fd << "  fd2:" << fd2 << "\n";
//			InsertarListaPid (listaPid, pidHijo);
//			cout << "Lista de hijos: \n";
//			ImprimirListaPid (listaPid);

			//******* manejo de señales *********
			//sigaction(SIGCHLD, &sa, NULL); 
			signal(SIGCHLD,SIG_IGN);
			close(fd2);

		}
		if( pidHijo == 0)
		{//PROCESO HIJO
			//cout << "Soy el hijo " << getpid() << " fd: " << fd << "  fd2:" << fd2 << "\n";
			sigaction(SIGINT, &sb, NULL);
			sigaction(SIGTERM, &sb, NULL);
			sigaction(SIGPIPE, &sb, NULL);
			sigaction(SIGSEGV, &sb, NULL);
			sigaction(SIGALRM, &sb, NULL);
			//signal(SIGCHLD, SIG_IGN); // atrapa señal SIGCHLD y la ignora para no dejar procesos zombies (
			//***ACTIVAR TIMER
			alarm(timeout);
			//***
			if (estado == autorizacion)
			{
				while (!transicion)
				{
					//cout << "Autorizacion....\n";
					buffer[0] = '\0';
					if ((numbytes=recv(fd2,&buffer,100,0)) == -1)
					{
						printf("Error en recv() \n");
						exit(-1);
					}
					buffer[numbytes-2] = '\0';
					//***RESET TIMER
					alarm(0); 
					alarm(timeout);
					//***
					// imprime el comando leido en la salida estandar
					cout << "\33[31m[\33[00m" << getTiempo() << "\33[31m-\33[00m" << ipport << "\33[33m<" << buffer << "\33[00m\n";
					todo = buffer;
					leerCedula(todo, comando);
					resetString(sede);
					//cout << "comando leido: \"" << comando << "\"\n";
					if (alumno (comando, sede))
					{
						resetString (mensaje);
						strcat(mensaje, "SI\r\n");
						enviar (fd2, mensaje, ipport);
						resetString (mensaje);
						sleep(1);
						strcat(strcat(mensaje, sede),"\r\n");
						enviar (fd2, mensaje, ipport);
						//estado = transaccion;
						//transicion = true;
						estado = actualizacion;
						transicion = true;
					}
					else
					{
						enviar (fd2, "NO\r\n", ipport);
						estado = actualizacion;
						transicion = true;
					}
				}
			}

			if (estado == actualizacion)
			{
				//cout << "Actualizacion....\n";
				close(fd2);	
				close(fd);
				exit(0); 		
			}
		}//el del else PROCESO HIJO
	}//el del while
}//el del main
