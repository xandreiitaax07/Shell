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

  while (1) // El programa termina cuando se pulsa Control+D dentro de get_command()
  {   		
    printf("COMANDO->");
    fflush(stdout);
    get_command(inputBuffer, MAX_LINE, args, &background); // Obtener el pr�ximo comando
    if (args[0]==NULL) continue; // Si se introduce un comando vac�o, no hacemos nada
/* Los pasos a seguir a partir de aqu�, son:
   (1) Genera un proceso hijo con fork()
   (2) El proceso hijo invocar� a execvp()
   (3) El proceso padre esperar� si background es 0; de lo contrario, "continue" 
   (4) El Shell muestra el mensaje de estado del comando procesado
   // (5) El bucle while regresa a la funci�n get_command()
*/

  pid_fork = fork(); //Creamos proceso hijo

  if(pid_fork > 0){//Creamos proceso padre
    if(!background){ //primer plano
      pid_wait = waitpid(pid_fork, &status, 0); //espera al proceso hijo
      status_res = analyze_status(status, &info); //
      printf("\nComando %s ejecutado en primer plano con pid %i. Estado finalizado. Info: %d\n", args[0], pid_fork, info);
    }else{ //segundo plano
      printf("\nComando %s ejecutado en segundo plano con pid %i.\n", args[0], pid_fork);
    }   
  }else if (pid_fork == 0){ //proceso hijo
    execvp(args[0], args); //sustituye todo el codigo por el comando que introduzcamos
    printf("\nError. Comando %s no encontrado\n", args[0]); //si no se encuntra la funcion lanzamos un error
    exit(-1); //y salimos
  }
}


