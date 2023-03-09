 /* Client code in C++ */
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <iostream>
  #include <string>
  #include <thread>
  #include <mutex>

  using namespace std;

  string msj;
  mutex mtx;

  void escritura_datos(int socketFD){
    string etiqueta;
    int n;

    while(!mtx.try_lock()){
      continue;
    }

    cout<<"Ingresa el nickname: ";
    getline(cin,msj);

    if( (to_string(msj.length()).length()) == 1){ //calcula si la longitud del nickname es de un caracter
      etiqueta = "0";etiqueta+="0"+to_string(msj.length());
    }
    else if( (to_string(msj.length()).length()) == 2){//calcula si la longitud del nickname es de dos caracteres
      etiqueta = "0"+to_string(msj.length());
    }
    else if( (to_string(msj.length()).length()) == 3){//calcula si la longitud del nickname es de tres caracteres
      etiqueta = to_string(msj.length());
    }
    else{
      cout<<"El nickname no tiene los caracteres permitidos"<<endl;
      shutdown(socketFD, SHUT_RDWR);
      close(socketFD);
    }

    n = write(socketFD,etiqueta.c_str(),etiqueta.length());// manda la etiqueta para que sepa la longitud del nick

    n = write(socketFD,msj.c_str(),msj.length());//manda el nick

    msj.clear();etiqueta.clear();
    cout<<"Ingresa tu mensaje: ";
    getline(cin,msj);

    mtx.unlock();

    if( (to_string(msj.length()).length()) == 1){ //calcula si la longitud del nickname es de un caracter
      etiqueta = "0";etiqueta+="0"+to_string(msj.length());
    }
    else if( (to_string(msj.length()).length()) == 2){//calcula si la longitud del nickname es de dos caracteres
      etiqueta = "0"+to_string(msj.length());
    }
    else if( (to_string(msj.length()).length()) == 3){//calcula si la longitud del nickname es de tres caracteres
      etiqueta = to_string(msj.length());
    }
    else{
      cout<<"El mensaje no tiene los caracteres permitidos"<<endl;
      shutdown(socketFD, SHUT_RDWR);
      close(socketFD);
    }

    n = write(socketFD,etiqueta.c_str(),etiqueta.length());// manda la etiqueta para que sepa la longitud del mensaje

    n = write(socketFD,msj.c_str(),msj.length());//manda el mensaje

  }
 
  void lectura_datos(int socketFD){
    char buffer[256];
    int n,etiqueta;
    string nick_recibido,msj_recibido;

    bzero(buffer,256);
    n = read(socketFD,buffer,3); //recibe la etiqueta del nick emisor
    if (n < 0) perror("ERROR reading from socket");
    buffer[3]='\0';
    etiqueta = atoi(buffer);

    bzero(buffer,4);
    n = read(socketFD,buffer,etiqueta); // recibe el nick del cliente emisor

    nick_recibido = string(buffer); // guarda el nick en un string


    bzero(buffer,256);
    n = read(socketFD,buffer,3); //recibe la etiqueta del mensaje emisor
    if (n < 0) perror("ERROR reading from socket");
    buffer[3]='\0';
    etiqueta = atoi(buffer);

    bzero(buffer,4);
    n = read(socketFD,buffer,etiqueta); // recibe el mensaje del cliente emisor

    msj_recibido = string(buffer); // guarda el mensaje en un string

    while(!mtx.try_lock()){
      continue;
    }

    cout<<"El usuario "<<nick_recibido<<" dice: "<<msj_recibido<<"."<<endl;

    mtx.unlock();

  }


  int main(void){
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int Res,n;
    char buff[256];
 
    if (-1 == SocketFD){
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);
    Res = inet_pton(AF_INET, "192.168.0.106", &stSockAddr.sin_addr);
 
    if (0 > Res){
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
	
    else if (0 == Res){
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    if (connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)) == -1){
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    do{
      thread e(escritura_datos,SocketFD); //Lanza la escritura de datos
      e.join();

      if(msj.length() == 0){ // Verifica si el mensaje fue para inscribir en el map
        bzero(buff,256);
        n = read(SocketFD,buff,30);
        printf("Se obtuvo: %s \n",buff);
        continue;
      }

      else if(msj == "lista"){ // Verifica si el mensaje fue para pedir la lista
        bzero(buff,256);
        n = read(SocketFD,buff,256);
        printf("Se obtuvo: %s \n",buff);
        continue;
      }

      thread l(lectura_datos,SocketFD);
      l.detach();

    }while(msj.compare("chau") != 0);

    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);

    return 0;
  }
