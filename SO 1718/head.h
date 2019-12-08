//
// //Tiago Gomes e Rui Linhares
//
#ifndef SO_HEAD_H
#define SO_HEAD_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <semaphore.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/msg.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <linux/mman.h>
#define PIPE_NAME "input_pipe"
#define N 4;
#define LOG_FILE "log.txt"
#define LOG_SIZE 5120

typedef struct Configuration{
    int triage;
    int doctor;
    float shift_size;
    int mq_max;

}Configuration;

Configuration *config_p;

typedef struct stats{
    int doutor_temporario;
    int atendidos;
    int triados;
    float espera_triagem;
    float espera_atendimento;
    float total;
    int mq_atual;
}stats;

stats *estatisticas;



typedef struct paciente{
    long int prioridade;
    char nome [50];
    float duracao_triagem;
    float duracao_atendimento;
    int inicio;
    int triagem;
    int atendimento;
    int fim;

}paciente;

paciente *pac;


typedef struct node{
    struct paciente paciente;
    struct node* next;
}node;

node *nd;

stats *estatisticas;
#define BUFF 20
Configuration *configp;
stats *estatisticas;        //confirmar se estando na memoria partilhada precisa de ser declarado
int memid_stats;
sem_t *sem_doutores;
sem_t *sem_estatiticas;
pthread_mutex_t lock_triados;
pthread_mutex_t lock_queue;
time_t start;
node *queue;
pthread_t* thread_list;
int* id_list;
int mqid;
int fd,exit_thread=0;
long int time_total;
int espera_t,espera_a;
int msgID;

//doctor extra
pthread_t thread_doctE;
int id_doctE;
int logFD;
char * mapLOG;

void media_total(int i,int f);
void espera_triagem(int i, int t);
void creatNamedPipe();
void closeNamedPipe();
void create_Message_Queue();
void clean_mq();
void clean_Queue();
void catch_ctrlc(int);
void sigUsr(int sig);
void start_Queue();
void cria_doente(char input[]);
void printStats();
void semaforosON();
void semaforosOFF();
void stats_triados();
void stats_atendidos();
void *workerthread();
void memory_shared();
void thread_joining();
int threadpool();
int leconfig();
void doctor_exit();
void turnoDoutor();
void * doctor_extra();
void Doctors();
void off();
void init();
void le_pipe();
void openMMAP();
void writeMMAP(char msg[]);
void closeMMAP();
#endif //SO_HEAD_H
