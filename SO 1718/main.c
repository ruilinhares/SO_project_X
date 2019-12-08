#include "head.h"
//Tiago Gomes e Rui Linhares


void espera_atendimento(int i, int t){
    sem_wait(sem_estatiticas);
    espera_a+=(t-i);
    float espera=espera_a/(estatisticas->atendidos);
    estatisticas->espera_atendimento=espera;
    sem_post(sem_estatiticas);

}

void media_total(int i,int f){
    sem_wait(sem_estatiticas);
    time_total+=(f-i);
    float total=time_total/(estatisticas->atendidos);
    estatisticas->total=total;
    sem_post(sem_estatiticas);
}


void espera_triagem(int i, int t){
    sem_wait(sem_estatiticas);
    espera_t+=(t-i);
    float espera=espera_t/(estatisticas->triados);
    estatisticas->espera_triagem=espera;
    sem_post(sem_estatiticas);

}
void openMMAP(){
    if ((logFD = open(LOG_FILE, O_RDWR| O_CREAT | O_TRUNC,0600))<0){
        perror("Erro abrir o LOG_FILE");
        exit(1);
    }

    if (lseek(logFD, LOG_SIZE-1, SEEK_SET)<0){
        close(logFD);
        perror("Erro lseek");
        exit(1);
    }

    if (write(logFD,"*",1)<0){
        close(logFD);
        perror("Erro write");
        exit(1);
    }

    if ((mapLOG = mmap(NULL, LOG_SIZE, PROT_WRITE, MAP_SHARED, logFD, 0)) ==  MAP_FAILED){
        perror("Erro mmap");
        exit(1);
    }
}

void writeMMAP(char msg[]){
    strcat(mapLOG,msg);
    msync(mapLOG,LOG_SIZE,MS_SYNC);
}

void closeMMAP(){
    if (munmap(mapLOG,LOG_SIZE)<0){
        close(logFD);
        perror("Erro munmap");
        exit(1);
    }
    close(logFD);
}


void creatNamedPipe(){
    unlink(PIPE_NAME);
    // cria o named pipe
    if ((mkfifo(PIPE_NAME, O_CREAT|O_EXCL|0600)<0) && (errno!=EEXIST)){
        perror("Erro ao cria o named pipe! ");
        exit(0);
    }

    // open named pipe para leitura
    if ((fd = open(PIPE_NAME, O_RDWR))<0){
        perror("Erro ao abrir o named pipe para leitura! ");
        exit(0);
    }
}

void closeNamedPipe(){
    close(fd);
    unlink(PIPE_NAME);
}
void create_Message_Queue(){
    signal(SIGINT, catch_ctrlc);
    signal(SIGHUP, catch_ctrlc);
    signal(SIGQUIT, catch_ctrlc);
    signal(SIGTERM, catch_ctrlc);

    /* Cria uma message queue */
    msgID = msgget(IPC_PRIVATE, IPC_CREAT|0777);
    if (msgID < 0)
    {
        perror("Ao criar a message queue");
        exit(0);
    }
}
void clean_mq(){
    if (msgctl(msgID,IPC_RMID,NULL)==-1){
        perror("Ao limpar a message queue");
        exit(0);
    }
}

void clean_Queue(){
    node* current = queue;
    node* next;
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
}

void sigUsr(int sig){
    printStats();
}

void catch_ctrlc(int sig) {
	exit_thread=1;
    while(wait(NULL)>0);
    thread_joining();
    clean_mq();
    printf("\n**Server terminating**\n");
    printStats();
    off();
    closeNamedPipe();
    closeMMAP();
    while (queue->next != NULL) {
        free(queue);
        queue = queue->next;
    }
    semaforosOFF();
    free(config_p);
    kill(0, SIGKILL);
    exit(0);
}

void start_Queue(){
    queue=(struct node *)malloc(sizeof(node));
    queue->next=NULL;
    strcpy(queue->paciente.nome,"head");
    printf("%s",queue->paciente.nome);
}

void cria_doente(char input[]) {  //cria a estrutura do doente e coloca-o na lista
    node *novo = (struct node *) malloc(sizeof(node));
    char *pch;
    pch = strtok(input, "_");
    int n;
    if ((n=atoi(pch))!=0){
        char *triagem;
        char *atendimento;
        char *prioridade;
        int i;
        i=0;
        printf("Grupo de pacientes de tamanho %d\n",n);
        triagem = strtok(NULL, "_");
        atendimento = strtok(NULL, "_");
        prioridade = strtok(NULL, "_");
        for(;i<n;i++){
            node *novo = (struct node *) malloc(sizeof(node));
            int r = 100 + rand() % 900;
            char str[3];
            sprintf(str,"%d",r);
            strcpy(novo->paciente.nome,str);
            novo->paciente.duracao_triagem = atof(triagem);
            novo->paciente.duracao_atendimento = atof(atendimento);
            novo->paciente.prioridade = atof(prioridade);
            novo->paciente.inicio = (int) (time(NULL));
            novo->paciente.atendimento = 0;
            novo->paciente.fim = 0;
            novo->paciente.triagem = 0;
            novo->next = NULL;
            node *current = queue;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = novo;
        }
    }
    else if (strcmp("triagem",pch)==0){
        printf("COMANDO PARA TRIAGEM\n");
        pch = strtok(NULL, "_");
        exit_thread=1;
        thread_joining();
        sem_wait(sem_estatiticas);
        config_p->triage=atoi(pch);
        sem_post(sem_estatiticas);
        threadpool();
    }
    else {
        strcpy(novo->paciente.nome, pch);
        pch = strtok(NULL, "_");
        novo->paciente.duracao_triagem = atof(pch);
        pch = strtok(NULL, "_");
        novo->paciente.duracao_atendimento = atof(pch);
        pch = strtok(NULL, "_");
        novo->paciente.prioridade = atoi(pch);
        novo->paciente.inicio = (int) (time(NULL));
        novo->paciente.atendimento = 0;
        novo->paciente.fim = 0;
        novo->paciente.triagem = 0;
        novo->next = NULL;
        node *current = queue;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = novo;
    }
}



void printStats(){
    printf("\nNº de pacientes triados: %d",estatisticas->triados);
    printf("\nNº de pacientes atendidos: %d",estatisticas->atendidos);
    printf("\nMédia do tempo de espera de atendimento: %.1lf",estatisticas->espera_atendimento);
    printf("\nMédia do tempo de espera de tiagem: %.1lf",estatisticas->espera_triagem);
    printf("\nMédia do tempo total: %.1lf",estatisticas->total);
    time_t finish=time(NULL);
    printf("\n %d\n", (int)(finish)-(int)(start));
}

void semaforosON(){

    sem_unlink("sem_doutores");
    sem_unlink("sem_estatiticas");
    sem_doutores=sem_open("sem_doutores",O_CREAT|O_EXCL,0700,1);
    sem_estatiticas=sem_open("sem_estatiticas",O_CREAT|O_EXCL,0700,1);
    if (pthread_mutex_init(&lock_queue, NULL) != 0)
    {
        perror("\n mutex init failed\n");

    }
}

void semaforosOFF(){

    sem_close(sem_doutores);
    sem_unlink("sem_doutores");

    sem_close(sem_estatiticas);
    sem_unlink("sem_estatiticas");

    pthread_mutex_destroy(&lock_queue);

}

void stats_triados() {
    sem_wait(sem_estatiticas);
    (estatisticas->triados)++;
    sem_post(sem_estatiticas);
}

void stats_atendidos() {
    sem_wait(sem_estatiticas);
    (estatisticas->atendidos)++;
    sem_post(sem_estatiticas);
}

void *workerthread(){ //deverá levar como parâmetro a estrutura do cliente
    while(1){
        pthread_mutex_lock(&lock_queue);
        node *current, *auxFree;
        paciente pc;
        current=queue;
        if (current->next==NULL) {
            pthread_mutex_unlock(&lock_queue);
            sleep(1);
        }
        else{
            pc=current->next->paciente;
            writeMMAP(pc.nome);
            writeMMAP(" a ser triado\n");
            printf("%s a ser triado\n",pc.nome);
            pc.triagem=(int) (time(NULL));
            auxFree = current->next;
            current->next=current->next->next;
            free(auxFree);
            pthread_mutex_unlock(&lock_queue);
            sleep(pc.duracao_triagem);
            pthread_mutex_lock(&lock_queue);
            stats_triados();
            espera_triagem(pc.inicio,pc.triagem);
            pthread_mutex_unlock(&lock_queue);
            writeMMAP(pc.nome);
            writeMMAP(" triado\n");
            if ((msgsnd(msgID, &pc, sizeof(paciente),0))>-1){
                pthread_mutex_lock(&lock_queue);
                sem_wait(sem_estatiticas);
                (estatisticas->mq_atual)++;
                sem_post(sem_estatiticas);
                pthread_mutex_unlock(&lock_queue);
            }
        }
        if (exit_thread==1) {
            pthread_exit(NULL);
        }
    }
}

void memory_shared() {
    if ((memid_stats = shmget(IPC_PRIVATE, sizeof(stats), IPC_CREAT | 0766)) != -1) {
        estatisticas = (stats *) shmat(memid_stats, NULL, 0);
        printf("\nMemória Partilhada de STATS %d \n", memid_stats);
        estatisticas->doutor_temporario=0;
        estatisticas->triados = 0;
        estatisticas->espera_triagem = 0;
        estatisticas->atendidos = 0;
        estatisticas->espera_atendimento = 0;
        estatisticas->total = 0;
        estatisticas->mq_atual = 0;
    } else
        perror("Erro ao criar memória");
}

void thread_joining(){
    int i;
    for (i = 0; i < (config_p->triage); i++) {
        if (pthread_join(thread_list[i], NULL) == 0) {
            printf("Triage thread %d was killed\n", id_list[i]);
        } else
            perror("Error joining threads");
    }
    if (pthread_join(thread_doctE,NULL)==0){
        printf("Doctor extra thread %d was killed\n", id_doctE);
    }else
        perror("Error joining threads");

    free(id_list);
    free(thread_list);
}

int threadpool() {
    exit_thread=0;
    thread_list = (pthread_t*)malloc(sizeof(pthread_t)*config_p->triage);
    id_list = (int *)malloc(sizeof(int)* config_p->triage);
    int i=0;
    for (; i < (config_p->triage); i++) {
        id_list[i]=i+1;
        if ((pthread_create(&thread_list[i], NULL, workerthread,&id_list[i])) != 0) {
            perror("ERROR");
            exit(0);
        }
        printf("Thread %d was created\n", id_list[i]);
    }
    // thread doutor extra
    if ((pthread_create(&thread_doctE, NULL, doctor_extra,&id_doctE)) != 0) {
        perror("ERROR");
        exit(0);
    }
    printf("Thread %d was created\n", id_doctE);
    return 0;
}


int lconfig(){
    config_p=(Configuration *)malloc(sizeof(Configuration));
    FILE *fp = fopen ("config.txt","r");
    int d,t,m;
    float s;
    fscanf(fp,"TRIAGE=%d\n",&t);
    fscanf(fp,"DOCTORS=%d\n",&d);
    fscanf(fp,"SHIFT_LENGTH=%f\n",&s);
    fscanf(fp,"MQ_MAX=%d",&m);
    config_p->doctor=d;
    config_p->triage=t;
    config_p->mq_max=m;
    config_p->shift_size=s;
    fclose(fp);
    return 0;
}

void doctor_exit(){
    printf("Doutor %d terminou o seu turno\n",getpid());
    exit(0);
}

void turnoDoutor(){
    printf("Doutor %d aqui e a atender\n",getpid());
    paciente auxPaciente;
    int v;
    int inicio = (int)(time(NULL));
    int tempo;
    do{
        sem_wait(sem_doutores);
        v = 3;
        while(v>0){
            if (msgrcv(msgID,&auxPaciente,sizeof(paciente),v,IPC_NOWAIT)<0)
                v--;
            else
                v=-1;
        }
        sem_post(sem_doutores);
        if (v!=0){
            writeMMAP(auxPaciente.nome);
            writeMMAP(" a ser atendido\n");
            auxPaciente.atendimento=(int)(time(NULL));
            printf("[%d]%s a ser atendido (prioridade %ld)\n",getpid(),auxPaciente.nome,auxPaciente.prioridade);
            sem_wait(sem_estatiticas);
            (estatisticas->mq_atual)--;
            sem_post(sem_estatiticas);
            sleep(auxPaciente.duracao_atendimento);
            stats_atendidos();
            auxPaciente.fim=(int)(time(NULL));
            media_total(auxPaciente.inicio,auxPaciente.fim);
            #ifdef DEBUG
            printf("Paciente %s atendido\n", auxPaciente.nome);
            #endif 
            writeMMAP(auxPaciente.nome);
            writeMMAP(" atendido\n");
        }else{
            #ifdef DEBUG
            printf("Doutor %d sem pacientes para atendimento\n", getpid());
            #endif
            sleep(1);
        }
    tempo = ((int)(time(NULL))) - inicio;
    }while(tempo < (config_p->shift_size));
    printf("Doutor %d terminou o seu turno\n",getpid());
    exit(0);
}

void * doctor_extra(){
    pid_t pid;
    int status;
    while(exit_thread==0){
        sleep(1);
        sem_wait(sem_estatiticas);
        int mq_a = estatisticas->mq_atual;
        sem_post(sem_estatiticas);
        if (mq_a > config_p->mq_max){
            if ((pid=fork())==0){
                signal(SIGINT,doctor_exit);
                printf("Doutor temporario %d aqui e a atender\n",getpid());
                paciente auxPaciente;
                int v;
                float percento;
                do{
                    sem_wait(sem_doutores);
                    v = 3;
                    while(v>0){
                        if (msgrcv(msgID,&auxPaciente,sizeof(paciente),v,IPC_NOWAIT)<0)
                            v--;
                        else
                            v=-1;
                    }
                    sem_post(sem_doutores);
                    if (v==-1){
                   		writeMMAP(auxPaciente.nome);
            			writeMMAP(" a ser atendido\n");
                        printf("[%d]%s a ser atendido (prioridade %ld)\n",getpid(),auxPaciente.nome,auxPaciente.prioridade);
                        sem_wait(sem_estatiticas);
                        (estatisticas->mq_atual)--;
                        sem_post(sem_estatiticas);
                        sleep(auxPaciente.duracao_atendimento);
                        stats_atendidos();
                        auxPaciente.fim=(int)(time(NULL));
                        media_total(auxPaciente.inicio,auxPaciente.fim);
                        writeMMAP(auxPaciente.nome);
            			writeMMAP(" atendido\n");
                    }
                    sem_wait(sem_estatiticas);
                    percento = (estatisticas->mq_atual)/(config_p->mq_max);
                    sem_post(sem_estatiticas);
                }while(percento>=0.8);
                printf("Doutor temporario %d terminou\n", getpid());
                exit(1) ;
            }else do{
                    sleep(1);
                    pid = waitpid(pid,&status, WNOHANG);
                }while (pid==0);
        }
    }
    pthread_exit(NULL);
    return NULL;
}

void Doctors(){ // processos
    int i;
    int pid;
    if ((pid = fork()) == 0){ // processo para gerar doutores
        int ret;
        for (i=0;i<(config_p->doctor);i++) {
            if ((pid = fork()) < 0) {
                perror("fork");
                abort();
            }
            else if (pid == 0) {
                signal(SIGINT, SIG_IGN);
                turnoDoutor();
                exit(i) ;
            }
        }

        while(wait(&ret)>0){
            ret = WEXITSTATUS(ret);
            if ((pid = fork()) < 0) {
                perror("fork");
                abort();
            }
            else if (pid == 0) {
                signal(SIGINT, SIG_IGN);
                turnoDoutor();
                exit(ret);
            }
        }
    }else if (pid<0){
        perror("fork, erro no gerador de processos");
        exit(0);
    }
}

void off(){
    if (shmdt(estatisticas)==-1){	// detaching da memoria partilhada
        perror("Error at detaching");
    }
    if (shmctl(memid_stats, IPC_RMID, NULL)==-1) {  //limpar memoria partilhada
        perror("Error unmapping shared memory\n");
    }
}

void init(){
    lconfig();
    creatNamedPipe();
    openMMAP();
    printf("Ficheiro config:\nN de Triagem  %d \nN de Doutores  %d \nTurnos  %.1f \nTamanho da MQ  %d",config_p->triage,config_p->doctor,config_p->shift_size,config_p->mq_max);
    memory_shared();
    semaforosON();
    start_Queue();
    create_Message_Queue();
    threadpool();
    Doctors();
}

void le_pipe(){
    while(1) {
        char buffer[1024];
        read(fd, &buffer, sizeof(buffer));
        //printf("%s\n", buffer);
        cria_doente(buffer);
    }
}

int main() {
    signal(SIGINT,catch_ctrlc);
    signal(SIGUSR1,sigUsr);
    start=time(NULL);
    printf("***PROCESSO PRINCIPAL*** \n\t  %d\n",getpid());
    init();
    le_pipe();
    return 0;
}

