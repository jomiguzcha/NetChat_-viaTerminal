  /* Server code in C++ */

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
  #include <map>
  #include <utility>

  using namespace std;

  map<string,int> client_list;

  void client_thread(int clientFD){
    int n, etiqueta_1, etiqueta_2, SD_respuesta;
    string nick_recibido;
    string msj_recibido;
    string respuesta;
    
    char buffer[256];

    do{
        bzero(buffer,256);
        n = read(clientFD,buffer,3); // recibe la etiqueta del nick del cliente emisor
        if (n < 0) perror("ERROR reading from socket");
        buffer[3]='\0';
        etiqueta_1 = atoi(buffer);
        
        bzero(buffer,4);
        n = read(clientFD,buffer,etiqueta_1); // recibe el nick del cliente emisor

        nick_recibido = string(buffer); // guarda el nick en un string

        cout<<"El nick que el cliente mando: "<<nick_recibido<<endl;

        bzero(buffer,256);
        n = read(clientFD,buffer,3); // recibe la etiqueta del mensaje del cliente emisor
        if (n < 0) perror("ERROR reading from socket");
        buffer[3]='\0';
        etiqueta_2 = atoi(buffer);

        bzero(buffer,4);
        n = read(clientFD,buffer,etiqueta_2); // recibe el mensaje del cliente emisor

        msj_recibido = string(buffer); // guarda el mensaje en un string

        cout<<"El mensaje que el cliente mando: "<<msj_recibido<<endl;
        
        if(msj_recibido.length() == 0 && nick_recibido.length() != 0){// si no envia mensaje, el nick se guardara en la lista de clientes como el nick del cliente emisor
          client_list.insert(pair<string,int>(nick_recibido,clientFD) );
          respuesta = "Anadido al servidor con exito";
          n = write(clientFD,respuesta.c_str(),respuesta.length());
          if (n < 0) perror("ERROR writing to socket");
          continue;
        }

        else if(msj_recibido == "lista"){// si envia el mensaje lista, se le pasara una lista de los nicknames
          respuesta.clear();
          for (map<string,int>::iterator it = client_list.begin(); it!=client_list.end(); ++it)
            respuesta+= it->first + "  ";
          n = write(clientFD,respuesta.c_str(),respuesta.length());
          if (n < 0) perror("ERROR writing to socket");
          continue;
        }
        

        SD_respuesta = client_list.find(nick_recibido)->second; // se busca el SD del cliente receptor del mensaje

        n = write(SD_respuesta,to_string(etiqueta_1).c_str(),to_string(etiqueta_1).length()); // envia la etiqueta del nick al receptor

        n = write(SD_respuesta,nick_recibido.c_str(),nick_recibido.length()); // envia el nick al receptor
        if (n < 0) perror("ERROR writing to socket");

        n = write(SD_respuesta,to_string(etiqueta_2).c_str(),to_string(etiqueta_2).length()); // envia la etiqueta del mensaje al receptor
        
        n = write(SD_respuesta,msj_recibido.c_str(),msj_recibido.length());
        if (n < 0) perror("ERROR writing to socket");

      }while(msj_recibido.compare("chau") != 0);
    
      shutdown(clientFD, SHUT_RDWR);
      close(clientFD);
  }

 
  int main(void){
    struct sockaddr_in stSockAddr;
    int ServerFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);    

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
    bind(ServerFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
	
	  listen(ServerFD,10);

    for(;;){
      cout<<"Esperando Cliente..."<<endl;
      int ConnectFD = accept(ServerFD, NULL, NULL);

      std::thread(client_thread,ConnectFD).detach();      
    }
 
    close(ServerFD);
    return 0;
  }
