//  g++ -o -std=c++11 servidor.exe servidor.cpp -pthread



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>
#include <vector>
#include <iostream>

#define PORT     8080
#define TAM_BUFFER 1024

using namespace std;





struct paquete
{
        string n_sec, n_flujo, sec_flujo, last, size, data, checksum;
        string res;

       int n_sec_, n_flujo_, sec_flujo_, last_, size_, checksum_;

        paquete(int _n_sec, int _n_flujo, int _sec_flujo, int _last, int _size, string _data, int _checksum)
        {       
                this->n_sec_ = _n_sec;
                this->n_flujo_ = _n_flujo;
                this->sec_flujo_ = _sec_flujo;
                this->last_ = _last;
                this->size_ = _size;
                this->checksum_ = _checksum;

                this->n_sec = complete(_n_sec, 5);
                this->n_flujo = complete(_n_flujo, 3);
                this->sec_flujo = complete(_sec_flujo, 2);
                this->last = to_string(_last);
                this->size = complete(_size, 3);
                this->data = _data;
                this->checksum = complete(_checksum, 2);

                string res;

                this->res = n_sec + n_flujo + sec_flujo + last + "A" + size + data + checksum;
        }

         string complete(int n, int c)
        {
                string res;
                res = to_string(n);
                while (res.size() != c)
                        res = "0" + res;
                return res;
        }
};


int checksum(string msj)
{
        int total = 0;
        for (int i = 0; i < msj.size(); i++)
                total = total + (int)msj[i];

        return total % 24;
}

bool test_bloque_completo( vector<paquete> bloque, int & sec_principal ){
        for (int i = 0; i < bloque.size(); i++)
        {
                if(bloque[i].n_sec_ == sec_principal) sec_principal++;
                else return 0;
        }
        return 1;
        
}

void serverMsj(int sockfd, sockaddr_in cliaddr){
        int sec_principal = 0;
        bool bloque_completo = 0;
        vector<paquete> bloque;
        char buffer[TAM_BUFFER];
        int len, n;
        len = sizeof(cliaddr);
        string mensaje;

        while(1){
                bool flag=1;
                while(flag){
                        n = recvfrom(sockfd, (char *)buffer, TAM_BUFFER,
                                                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                                                (socklen_t*)&len);
                        buffer[n] = '\0';
                        
                        mensaje = buffer;

                        cout << "\n\nBLOQUE RECIBIDO: " << mensaje << endl;


                        bloque.push_back(paquete(stoi(mensaje.substr(0,5)),
                                                 stoi(mensaje.substr(5,3)),
                                                 stoi(mensaje.substr(8,2)),
                                                 stoi(mensaje.substr(10,1)),
                                                 stoi(mensaje.substr(12,3)),
                                                 mensaje.substr(15,n-15-2),
                                                 stoi(mensaje.substr(15 + (n-15-2),2))));

                        

                        if (bloque[bloque.size()-1].last_ == 1) flag = 0;

                }

        string ack = "A";
        
        vector<int> aux_secuencias;

        for(int i=0; i<bloque.size();i++){
                ack += bloque[i].n_sec; 
                aux_secuencias.push_back ( bloque[i].n_sec_);
        }


        ack += to_string(checksum(ack)).size()==1 ? "0" + to_string(checksum(ack)) : to_string(checksum(ack));

        char *buff = &ack[0];
        sendto(sockfd, buff, strlen(buff),
                MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                        len);

        bloque_completo = test_bloque_completo( bloque, sec_principal );

        cout << sec_principal << endl;
        if (bloque_completo) {
        

                string mensajeRecibido ="";

                for(int i=0; i<bloque.size();i++){
                        mensajeRecibido += bloque[i].data;
                }

                bloque.clear();

                cout << "\n\n\nMensaje del Cliente: " << mensajeRecibido << endl;


        }
        
        }

}

// Driver code
int main() {

        int sockfd;
        char buffer[TAM_BUFFER];
        struct sockaddr_in servaddr, cliaddr;

        // Creating socket file descriptor
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
        }

        memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        // Filling server information
        servaddr.sin_family = AF_INET; // IPv4
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(PORT);

        // Bind the socket with the server address
        if ( bind(sockfd, (const struct sockaddr *)&servaddr,
                        sizeof(servaddr)) < 0 )
        {
                perror("bind failed");
                exit(EXIT_FAILURE);
        }

        int len, n;

        len = sizeof(cliaddr); //len is value/resuslt


        thread(serverMsj,sockfd,cliaddr).detach();


        
        while(1);


        return 0;
}


