//  g++ -o -std=c++11 cliente.exe cliente.cpp -pthread


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <thread>
#include <vector>
#include <iostream>

#define PORT 8080
#define TAM_BUFFER 1024

using namespace std;


vector<int> ack_secuencias;

int n_paquetes = 0;
int secuencia = 0;

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

vector<paquete> bloque;
vector<paquete> bloque_aux;

void enviarMsj(int sockfd, sockaddr_in servaddr)
{
        while (1)
        {
                usleep(500 * 1000);
                
                for (int i = 0; i < bloque.size(); i++)
                {       
                        char *buff = &bloque[i].res[0];
                        sendto(sockfd, buff, strlen(buff),
                               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                               sizeof(servaddr));
                }
                
        }
}

int checksum(string msj)
{
        int total = 0;
        for (int i = 0; i < msj.size(); i++)
                total = total + (int)msj[i];

        return total % 24;
}

bool test_checksum(string msj, int cs)
{
        int total = 0;
        for (int i = 0; i < msj.size(); i++)
                total = total + (int)msj[i];

        if (total % 24 == cs)
                return 1;
        else
                return 0;
}

bool check_secuencias(vector<int> sec)
{

        bool res = 0;

        for (int i = 0; i < ack_secuencias.size(); i++)
        {
                for (int e = 0; e < sec.size(); e++)
                {       
                        if(ack_secuencias[i] == sec[e]) break;
                        else if (e == sec.size() - 1 && ack_secuencias[i] != sec[e])
                        {       
                                for(int x=0; x<bloque_aux.size(); x++){
                                        if(bloque_aux[x].n_sec_ == ack_secuencias[i])
                                                bloque.push_back(bloque_aux[x]);
                                }
                                res = 1;
                        }
                }
        }

        return res;
}

// 5 +3 +2 +1 + 1 +3 + DATA + 2 = 17
// 512 - 17 = 495 -1 = 494

void clientMsj(int sockfd, sockaddr_in servaddr)
{

        char buffer[TAM_BUFFER];
        string mensaje;
        int n, len, n_pack;
        vector<int> sec_ack;

        while (1)
        {

                printf("\n MENSAJE :  ");
                gets(buffer);
                mensaje=buffer;
                if(mensaje == "test")
                        mensaje = "Puede que la tarea que me he impuesto de escribir una historia completa del pueblo romano desde el comienzo mismo de su existencia me recompense por el trabajo invertido en ella, no lo sé con certeza, ni creo que pueda aventurarlo. Porque veo que esta es una práctica común y antiguamente establecida, cada nuevo escritor está siempre persuadido de que ni lograrán mayor certidumbre en las materias de su narración, ni superarán la rudeza de la antigüedad en la excelencia de su estilo. Aunque esto sea así, seguirá siendo una gran satisfacción para mí haber tenido mi parte también en investigar, hasta el máximo de mis capacidades, los anales de la nación más importante del mundo, con un interés más profundo; y si en tal conjunto de escritores mi propia reputación resulta ocultada, me consuelo con la fama y la grandeza de aquellos que eclipsen mi fama. El asunto, además, es uno que exige un inmenso trabajo. Se remonta a más de 700 años atrás y, después de un comienzo modesto y humilde, ha crecido a tal magnitud que empieza a ser abrumador por su grandeza. No me cabe duda, tampoco, que para la mayoría de mis lectores los primeros tiempos y los inmediatamente siguientes, tienen poco atractivo; Se apresurarán a estos tiempos modernos en los que el poderío de una nación principal es desgastado por el deterioro interno. Yo, en cambio, buscaré una mayor recompensa a mis trabajos en poder cerrar los ojos ante los males de que nuestra generación ha sido testigo durante tantos años; tanto tiempo, al menos, como estoy dedicando todo mi pensamiento a reproducir los claros registros, libre de toda la ansiedad que pueden perturbar el historiador de su época, aunque no le puedan deformar la verdad.";

                n_pack = mensaje.size() / 494;
                n_pack = mensaje.size() % 494 == 0 ? n_pack : n_pack + 1;

                int flujo = 0;
                int sec = 0;

                for (int i = 0; i < n_pack; i++)
                {
                        string parte = mensaje.substr(494 * i, 494);
                        bloque.push_back(paquete(secuencia,flujo,sec,i==n_pack-1 ? 1 : 0, parte.size(), parte, checksum(parte)));
                        ack_secuencias.push_back(secuencia);
                        secuencia++;
                        sec ++;
                }

                flujo ++;
                bloque_aux = bloque;
                sec = 0;

                bool flag = 1;
                while (flag)
                {
                        n = recvfrom(sockfd, (char *)buffer, TAM_BUFFER,
                                     MSG_WAITALL, (struct sockaddr *)&servaddr,
                                     (socklen_t *)&len);
                        buffer[n] = '\0';

                        mensaje = buffer;
                        
                        cout << "ack " << mensaje << endl;

                        vector<int> ack_secuencias_aux;
                        
                        if (mensaje[0] == 'A')
                        {
                                mensaje = mensaje.substr(1, n - 3);
                                //cout << mensaje << endl;
                                for (int i = 0; i < mensaje.size()/5; i++)
                                {
                                        ack_secuencias_aux.push_back(stoi(mensaje.substr(i * 5, 5)));
                                }

                                for (int i = 0; i < ack_secuencias.size(); i++)
                                {
                                        cout << "ack_secuencias " << ack_secuencias[i] << endl;
                                }

                                for (int i = 0; i < ack_secuencias_aux.size(); i++)
                                {
                                        cout << "ack_secuencias_aux " << ack_secuencias_aux[i] << endl;
                                }
                                

                                //cout << "pase el for" << endl;

                                flag = check_secuencias(ack_secuencias_aux);

                                
                        }

                        ack_secuencias_aux.clear();
                }

                ack_secuencias.clear();
                bloque_aux.clear();
                bloque.clear();
                
        }
}

// Driver code
int main()
{
        int sockfd;
        char buffer[TAM_BUFFER];
        struct hostent *host;
        struct sockaddr_in servaddr;

        host = (struct hostent *)gethostbyname((char *)"127.0.0.1");

        // Creating socket file descriptor
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
        }

        memset(&servaddr, 0, sizeof(servaddr));

        // Filling server information
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr = *((struct in_addr *)host->h_addr);

        int n, len;

        thread(clientMsj, sockfd, servaddr).detach();
        thread(enviarMsj, sockfd, servaddr).detach();

        while (1);

        close(sockfd);
        return 0;
}
