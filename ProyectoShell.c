/*------------------------------------------------------------------------------
Proyecto Shell de UNIX. Sistemas Operativos
Grados I. Inform�tica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Algunas secciones est�n inspiradas en ejercicios publicados en el libro
"Fundamentos de Sistemas Operativos", Silberschatz et al.

Para compilar este programa: gcc ProyectoShell.c ApoyoTareas.c -o MiShell
Para ejecutar este programa: ./MiShell
Para salir del programa en ejecuci�n, pulsar Control+D
------------------------------------------------------------------------------*/

#include "ApoyoTareas.h" // Cabecera del m�dulo de apoyo ApoyoTareas.c
 
#define MAX_LINE 256 // 256 caracteres por l�nea para cada comando es suficiente
#include <string.h>  // Para comparar cadenas de cars. (a partir de la tarea 2)

// --------------------------------------------
//                     MAIN          
// --------------------------------------------

job* myList; //declaracion de la lista de tareas(myList), siendo un puntero para poder recorrerla

void manejador(int signal){ //nos permite controlar las señales de los procesos conforme se van ejeutando de modo que ningun quede en estado zombie
  job * trabajo;
  pid_t pid_wait;
  int status, info;
  enum status status_res;

  while((pid_wait = waitpid(-1, &status, WUNTRACED | WNOHANG | WCONTINUED)) > 0){
    trabajo = get_item_bypid(myList, pid_wait); //busca y devuelve un elemento de la lista que coincida con el pid
    if(trabajo){ //si el elemento existe entonces:
      status_res = analyze_status (status, &info); //analizamos el estado en el que se encuentra el proceso
      printf("Comando %s ejecutado en segundo plano con pid %i. estado %s. Info: %i.\n",
        trabajo -> command, trabajo ->pgid, status_strings[status_res], info);
      

      if(status_res == SENALIZADO || status_res == FINALIZADO){ //cuando llega a nuestro manejador un proceso con uno de estos casos, se analiza y en este caso se elimina
        delete_job(myList, trabajo);
      }else if (status_res == REANUDADO){ //verifica que el estado ha sido reanudado y si es asi lo hace en segundo plano
        trabajo->ground = SEGUNDOPLANO;
      }else if (status_res == SUSPENDIDO){ 
        trabajo->ground = DETENIDO;
      }
    }
  }
}

int main(void)
{
  char inputBuffer[MAX_LINE]; // B�fer que alberga el comando introducido
  int background;         // Vale 1 si el comando introducido finaliza con '&'
  char *args[MAX_LINE/2]; // La l�nea de comandos (de 256 cars.) tiene 128 argumentos como m�x
                              // Variables de utilidad:
  int pid_fork, pid_wait; // pid para el proceso creado y esperado
  int status;             // Estado que devuelve la funci�n wait
  enum status status_res; // Estado procesado por analyze_status()
  int info;		      // Informaci�n procesada por analyze_status()
  ignore_terminal_signals();
  signal(SIGCHLD, manejador); //asociamos manejador a la señal SIGCHLD
                  //SIGCHLD lanza la señal al padre cuando el hijo finaliza o se suspende
  myList = new_job(0, "Job List", PRIMERPLANO); ///creamos lista de tareas
  int primerPlano = 0;

  while (1) // El programa termina cuando se pulsa Control+D dentro de get_command()
  {   		
    printf("COMANDO->");
    fflush(stdout);
    get_command(inputBuffer, MAX_LINE, args, &background); // Obtener el pr�ximo comando
    if (args[0]==NULL) continue; // Si se introduce un comando vacio, no hacemos nada

    //Comandos internos
    if (strcmp(args[0], "cd") == 0){ //FASE 2
      if(chdir(args[1]) == -1){ //si el comando no existe sale un mensaje de error
        printf("\nError. Directorio no encontrado\n");
      }
      continue;
    }

    if (strcmp(args[0], "logout") == 0){ //FASE 2
      printf("\nSaliendo del Shell...\n");
      exit(0);
    }

    pid_fork = fork(); //Creamos proceso hijo

    if(pid_fork > 0){//Creamos proceso padre
    new_process_group(pid_fork);
      if(!background){ //primer plano
        set_terminal(pid_fork);
        pid_wait = waitpid(pid_fork, &status, WUNTRACED); //espera al proceso hijo
        set_terminal(getpid()); //el padre una vez el hijo finalice lo que esté haciendo recupera la terminal
        
        if(pid_fork = pid_wait){ //cuando el proceso que el pare espera es igual al hijo que esta en la terminal
          status_res = analyze_status(status, &info); //analizamos el estado del hijo

          if(status_res == SUSPENDIDO){ //si el estado del hijo es suspendido
            printf("\nComando %s ejecutado en primer plano con pid %i. Estado %s. Info: %i\n", args[0], pid_fork,status_strings[status_res], info);
            job* aux = new_job(pid_fork,args[0],DETENIDO);
            add_job(myList, aux);
          }else if(status_res == FINALIZADO){ //si el estado del hijo es finalizado, es decir, que ha finalizado su ejecucion
            if(info != 255){
              printf("\nComando %s ejecutado en primer plano con pid %i. Estado %s. Info: %i\n", args[0], pid_fork,status_strings[status_res], info);
            }
          }
        }
        primerPlano = 0;
      }else{ //segundo plano
        printf("\nComando %s ejecutado en segundo plano con pid %i.\n", args[0], pid_fork);
        job* aux = new_job(pid_fork,args[0],SEGUNDOPLANO);
        add_job(myList, aux);
      }   
    }else if (pid_fork == 0){ //proceso hijo
      new_process_group(getpid()); //Asignamos grupo propio
      if(!background){
        set_terminal(getpid()); //el hijo toma la terminal
      }
      restore_terminal_signals(); //una vez finalizado el hijo, restablecemos las señales de la terminal
      execvp(args[0], args); //sustituye todo el codigo por el comando que introduzcamos
      printf("\nError. Comando %s no encontrado\n", args[0]); //si no se encuntra la funcion lanzamos un error
      exit(-1); //y salimos
    }else{
      perror("\nError en el fork\n");
      continue;
    }
  }
}


