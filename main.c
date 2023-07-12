/*
Andrés Díaz V-28.285.435
Jose Marquez V-27.675.746

Para el correcto funcionamiento del programa se deben pasar los argumentos
de threads y de archivo de entrada de la siguiente manera
    ./programa entrada.txt 6

Instrucciones de compilacion
  El programa consta de main.c , utils.c y utils.h

  WINDOWS:
    se debe utilizar alguna herramienta que linken las librerias de win32 api
    como visual studio, en este caso se utilizo MinGW  y el comando de
compilacion fue el siguiente

    gcc main.c utils.c -o programa.exe
  Linux:
    Para la compilacion en linux se deben descargar algunas librerias de
criptografia con

    apt-get install libssl-dev

    En linux se utilizo el compilador  GNU gcc y el comando de compilacion
    fue el siguiente

    gcc -o programa main.c utils.c -lssl -lcrypto -pthread

*/

#include <stdio.h>
#include <string.h>

#include "utils.h"

// function declarations
int lookup(FILE *input, char *key, char *value, int value_length);
int generate_password(int idx, char *password);
int process_line(FILE *input, char *name, char *hash);
void organize_output(void);
void worker(void);

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Error: se nececitan 2 argumentos (archivo entrada y hilos).\n");
    return 1;
  }

  Thread *threads;
  inputfile = fopen(argv[1], "r");

  if (inputfile == NULL) {
    printf("El archivo de entrada no existe.\n");
  }

  tempfile = fopen("tempfile.txt", "w");
  char hash[33];

  int thread_num = atoll(argv[2]);
  threads = malloc(sizeof(Thread) * thread_num);
  printf("el programa comenzara a creacker contraseñas, usando %d hilos\n",
         thread_num);

  setup_thread_func(worker);
  create_mutex(&write_mutex);
  create_mutex(&read_mutex);

  for (int i = 0; i < thread_num; i++) {
    start_thread(&threads[i]);
  }
  for (int i = 0; i < thread_num; i++) {
    join_thread(&threads[i]);
  }
  free(threads);

  fclose(inputfile);
  fclose(tempfile);

  organize_output();

  printf("Programa finalizo exitosamente\n");
}

int process_line(FILE *input, char *name, char *hash) {
  char line[96];
  if (fgets(line, 96, input) == NULL || strlen(line) == 0) {
    return 0;
  }

  char *substr = strstr(line, "::");

  if (substr == NULL) {
    return 0;
  }
  int substr_idx = (int)(strstr(line, "::") - line);
  int right_part_length = strlen(line + substr_idx + 2);

  strncpy(name, line, substr_idx);
  name[substr_idx] = '\0';
  strncpy(hash, line + substr_idx + 2, right_part_length);
  if (hash[right_part_length - 1] == '\n') {
    hash[right_part_length - 1] = '\0';
  }
  return 1;
}

int generate_password(int idx, char *password) {
  char *letras = "\0abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int len = strlen(letras + 1) +
            1;  // this skips the end of string char but still counts it
  if (idx >= len * len * len * len) {
    return 0;
  }

  password[0] = letras[idx % len];
  password[1] = letras[(idx / len) % len];
  password[2] = letras[(idx / (len * len)) % len];
  password[3] = letras[(idx / (len * len * len)) % len];
  password[4] = '\0';
  return 1;
}

int lookup(FILE *input, char *key, char *value, int value_length) {
  char name[64];
  char password[33];
  int process_line_result;

  rewind(input);

  while (1) {
    process_line_result = process_line(input, name, password);

    if (process_line_result == 0) {
      return 0;
    }

    if (strcmp(name, key) == 0) {
      strncpy(value, password, value_length - 1);
      value[value_length - 1] = '\0';
      return 1;
    }
  }
  return 0;
}

void worker(void) {
  char name[64];
  char hash[33];
  int process_line_result;
  char password[5];
  char correct_password[5] = {0};
  char digest[33];
  while (1) {
    lock_mutex(&read_mutex);
    process_line_result = process_line(inputfile, name, hash);
    unlock_mutex(&read_mutex);

    if (process_line_result == 0) {
      return;
    }

    for (int i = 0; generate_password(i, password); i++) {
      md5(digest, password);

      if (strcmp(digest, hash) == 0) {
        strcpy(correct_password, password);
        break;
      }
    }

    if (correct_password[0] == '\0') {
      printf("no fue posible cracker la contraseña para %s\n", name);
    } else {
      lock_mutex(&write_mutex);
      printf("el programa ha crackeado una de las contraseñas\n");
      fprintf(tempfile, "%s::%s\n", name, correct_password);
      unlock_mutex(&write_mutex);
    }
  }
}

void organize_output(void) {
  tempfile = fopen("tempfile.txt", "r");
  FILE *salida = fopen("salida.txt", "w");
  FILE *input = fopen("entrada.txt", "r");

  char name[64];
  char hash[33];
  char password[5];
  int process_line_result;
  while (input != NULL && salida != NULL && tempfile != NULL) {
    process_line_result = process_line(input, name, hash);

    if (process_line_result == 0) {
      return;
    }
    if (lookup(tempfile, name, password, 5)) {
      fprintf(salida, "%s:%s\n", name, password);
    } else {
      printf("error al organizar el archivo");
    }
  }
}