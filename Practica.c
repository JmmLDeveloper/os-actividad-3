/*
Andrés Díaz V-28.285.435
Jose Marquez
    Cosas que faltaron:
    1.-    La opción de extraer el codigo de una línea (no leer el txt sino hacer un
        split de la linea despues del ::)
    2.-    Terminar de desarrollar la idea de un productor de claves con muchos consumidores
        que verifican las contreseñas generadas (solo se tiene la opción de un solo hilo
        con un semaforo para multiplexar las tareas que se estan realizando)
    3.-    Implementarlo en windows
    4.-    Algunas validaciones
    5.-    Una posible petición para poder terminar como tal el ejercicio

    Consideraciones:
s
        OJO. Como se utilizó openssl para hacer el algoritmo DM5 se tiene que meter la siguiente 
        linea (ademas de instalarlo)

        gcc -o Practica.out Practica.c -lssl -lcrypto -pthread
*/

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>

pthread_mutex_t read_lock;
pthread_mutex_t write_lock;

int generate_password(int idx,char* password){
    char* letras ="\0abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int len = strlen(letras + 1) + 1; // this skips the end of string char but still counts it
    if ( idx >= len * len *len *len ) {
      return 0;
    }

    password[0]=letras[idx % len];
    password[1]=letras[ (idx / len) %  len ];  
    password[2]=letras[ (idx / (len * len) ) %  len ];
    password[3]=letras[ (idx / (len * len *len ) ) %  len ];
    password[4]='\0';
    return 1;
}


int process_line(FILE* input,char* name,char* hash) {
    char line[96];
    if(fgets(line,96,input) == NULL || strlen(line) == 0 ) {
        return 0;
    }


    char* substr = strstr(line,"::");

    if (substr == NULL) {
      return 0;
    }
    int substr_idx = (int)(strstr(line,"::") -  line);
    int right_part_length = strlen(line + substr_idx + 2);

    strncpy(name,line, substr_idx );
    name[substr_idx] = '\0';
    strncpy(hash,line + substr_idx + 2 , right_part_length );
    if ( hash[right_part_length - 1] == '\n' ) {
      hash[right_part_length - 1] = '\0';
    }
    return 1;
}

void md5(char* digest,char* input) {
    unsigned char md5_result[EVP_MAX_MD_SIZE];
    unsigned int md5_length;
    EVP_MD_CTX *mdctx;
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit(mdctx, EVP_md5());
    EVP_DigestUpdate(mdctx, input, strlen(input));
    EVP_DigestFinal(mdctx, md5_result, &md5_length);
    EVP_MD_CTX_free(mdctx);
    char result_str[md5_length*2 + 1];
    for(int i=0;i<md5_length;i++) {
        sprintf(&result_str[i*2], "%02x", md5_result[i]);
    }

    strncpy(digest,result_str,33);
}
 
FILE* inputfile;
FILE* tempfile;

int lookup(FILE* input,char* key,char* value,int value_length) {
  char name[64];
  char password[33];
  int process_line_result;
  
  rewind(input);

  while ( 1 ) {
      process_line_result = process_line(input,name,password); 

      if ( process_line_result == 0 ) {
	return 0;
      }
      
      if( strcmp(name,key) == 0 ) {
	strncpy(value,password,value_length - 1);
	value[value_length - 1] = '\0';
	return 1;
      }
	      
  }
  return 0;
}



void organize_output() {

    tempfile = fopen("tempfile.txt", "r");  
    FILE *salida = fopen("salida.txt", "w");  
    FILE *input = fopen("entrada.txt", "r");  

    char name[64];
    char hash[33];
    char password[5];
    int process_line_result;
    while ( input != NULL && salida != NULL && tempfile != NULL ) {
	process_line_result = process_line(input,name,hash); 

	if ( process_line_result == 0 ) {
	  return;
	}
	if( lookup(tempfile,name,password,5)  ) {
	  fprintf(salida,"%s \t %s\n",name,password);  
	} else {
	  printf( "errocito de bolistio");
	}
    }
}
int cracked_passwords_counter = 0;
void* worker(void* args) {
    char name[64];
    char hash[33];
    int process_line_result;
    char password[5];
    char correct_password[5] = {0} ;
    char digest[33];
    while(1) {
	pthread_mutex_lock(&read_lock);		
	  process_line_result = process_line(inputfile,name,hash); 
	pthread_mutex_unlock(&read_lock);		

	if ( process_line_result == 0 ) {
	  return NULL;
	}

	for(int i =0 ; generate_password(i,password) ; i++) {
	  md5(digest,password);
	    
	  if( strcmp(digest,hash) == 0 ) {
	      strcpy(correct_password,password);
	      break;
	  }
  	}

	if (correct_password[0] == '\0') {
	    printf("no fue posible cracker la contraseña para %s\n", name);

	} else {
	  pthread_mutex_lock(&write_lock);		
	    cracked_passwords_counter++;
	    printf("el programa ha crackeado %d contraseñas\n",cracked_passwords_counter);
	    fprintf(tempfile,"%s::%s\n",name,correct_password);
	  pthread_mutex_unlock(&write_lock);		
	}

    }
}

int read_num() {
    char buffer[16];
    fgets(buffer,15,stdin);
    return atoll(buffer);
}



int main(int argc, char *argv[]) {
    pthread_t* threads;
    FILE *input = fopen(argv[1], "r");  
    tempfile = fopen("tempfile.txt", "w");  
    inputfile = input;
    int thread_num = atoll(argv[2]); // falta validar 
    threads = malloc( sizeof(pthread_t) * thread_num );
    printf("el programa comenzara a creacker contraseñas, usando %d hilos\n",thread_num);

    for(int i = 0; i < thread_num; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }
    for(int i = 0; i < thread_num; i++) {
        pthread_join(threads[i],NULL);
    }
    free(threads);
    fclose(input);
    fclose(tempfile);
    organize_output();
    printf("el programa a finalizado correctamente\n");
    return 0;

}
