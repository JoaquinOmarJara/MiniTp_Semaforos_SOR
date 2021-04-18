#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>

#define LIMITE 50
#define LIMITE_ACCIONES 9
#define LIMITE_INGREDIENTES 10

//creo estructura de semaforos 
struct semaforos {
	sem_t sem_picar_vegetales;
    sem_t sem_mezclar;
	//sem_t sem_sazonar_mezcla;
	sem_t sem_agregar_carne;
	sem_t sem_empanar_carne;
	//sem_t sem_freir_milanesa;
	//sem_t sem_hornear_pan;
	sem_t sem_cortar_vegetales;
	sem_t sem_armar_sandwich;
	//poner demas semaforos aqui
};

struct semaforos_compartidos {
	sem_t sem_sazonar_mezcla;
	sem_t sem_freir_milanesa;
	sem_t sem_hornear_pan;
	//poner demas semaforos aqui
};

struct semaforos_compartidos *semaforos_c = malloc(sizeof(struct semaforos_compartidos));

//creo los pasos con los ingredientes
struct ingrediente {
	char nombre_ingrediente[LIMITE];
};


struct accion {
	char nombre_accion[LIMITE];
    struct ingrediente ingrediente[LIMITE_INGREDIENTES];
};

//creo los parametros de los hilos 
struct parametro {
 int equipo_param;
 struct semaforos semaforos_param[LIMITE_ACCIONES];
 struct accion pasos_param[LIMITE_ACCIONES];
};

void inicializarStructAcciones(struct accion *acciones){
	int j;
	int x;
	for(j = 0; j < LIMITE_ACCIONES; j++){
		strcpy(acciones[j].nombre_accion, "");
		for(x = 0; x < LIMITE_INGREDIENTES; x++){		
			strcpy(acciones[j].ingrediente[x].nombre_ingrediente, "");
		}		
	}
}

void obtenerReceta(struct accion *acciones){
	inicializarStructAcciones(acciones);
	FILE *origen;
	char linea[50];	

	origen= fopen("receta.txt","r");
	if (origen == NULL) {
		printf( "Problemas con la apertura de los ficheros.\n" );
		exit( 1 );
	}
	
	int j;
	int x;
	
	int indiceAccion = 0;
	while (feof(origen) == 0) {
		fgets(linea,50,origen);
		char nombre_accion[50];
		if(strcmp("Accion:\n",linea) == 0){
			fgets(nombre_accion,50,origen);
			strcpy(acciones[indiceAccion].nombre_accion, nombre_accion);
			fgets(linea,50,origen);
			int indiceIngrediente = 0;
			char nombre_ingrediente[50];
			fgets(nombre_ingrediente,50,origen);
			while(strcmp(nombre_ingrediente, "\n") != 0 & feof(origen) == 0){	
				strcpy(acciones[indiceAccion].ingrediente[indiceIngrediente].nombre_ingrediente, nombre_ingrediente);	
				fgets(nombre_ingrediente,50,origen);
				indiceIngrediente++;
			}
		}
		indiceAccion++;
	}
	
	if (fclose(origen)!= 0)
		printf( "Problemas al cerrar el fichero origen.txt\n" );
	
}

//funcion para imprimir las acciones y los ingredientes de la accion
void* imprimirAccion(void *data, char *accionIn) {
	struct parametro *mydata = data;
	//calculo la longitud del array de pasos 
	int sizeArray = (int)( sizeof(mydata->pasos_param) / sizeof(mydata->pasos_param[0]));
	//indice para recorrer array de pasos 
	int i;
	for(i = 0; i < sizeArray; i ++){
		//pregunto si la accion del array es igual a la pasada por parametro (si es igual la funcion strcmp devuelve cero)
		if(strcmp(mydata->pasos_param[i].accion, accionIn) == 0){
		printf("\tEquipo %d - accion %s \n " , mydata->equipo_param, mydata->pasos_param[i].accion);
		//calculo la longitud del array de ingredientes
		int sizeArrayIngredientes = (int)( sizeof(mydata->pasos_param[i].ingredientes) / sizeof(mydata->pasos_param[i].ingredientes[0]) );
		//indice para recorrer array de ingredientes
		int h;
		printf("\tEquipo %d -----------ingredientes : ----------\n",mydata->equipo_param); 
			for(h = 0; h < sizeArrayIngredientes; h++) {
				//consulto si la posicion tiene valor porque no se cuantos ingredientes tengo por accion 
				if(strlen(mydata->pasos_param[i].ingredientes[h]) != 0) {
							printf("\tEquipo %d ingrediente  %d : %s \n",mydata->equipo_param,h,mydata->pasos_param[i].ingredientes[h]);
				}
			}
		}
	}
}

//funcion para tomar de ejemplo
void* picarVegetales(void *data) {
	//creo el nombre de la accion de la funcion 
	char *accion = "Picar vegetales";
	//creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	//llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//uso sleep para simular que que pasa tiempo
	usleep( 20000 );
	//doy la señal a la siguiente accion (cortar me habilita mezclar)
    sem_post(&mydata->semaforos_param.sem_mezclar);
	
    pthread_exit(NULL);
}

void* mezclar(void *data) {
	sem_wait(&mydata->semaforos_param.sem_picar_vegetales);
	//creo el nombre de la accion de la funcion 
	char *accion = "Mezclar";
	//creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	//llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//uso sleep para simular que que pasa tiempo
	usleep( 20000 );
	//doy la señal a la siguiente accion (cortar me habilita mezclar)
    sem_post(&mydata->semaforos_param.sem_sazonar_mezcla);
	
    pthread_exit(NULL);
}

void* ejecutarReceta(void *i) {
	
	//variables semaforos
	sem_t sem_picar_vegetales;
	sem_t sem_mezclar;
	//sem_t sem_sazonar_mezcla;
	sem_t sem_agregar_carne;
	sem_t sem_empanar_carne;
	//sem_t sem_freir_milanesa;
	//sem_t sem_hornear_pan;
	sem_t sem_cortar_vegetales;
	sem_t sem_armar_sandwich;
	
	//crear variables semaforos aqui
	
	//variables hilos
	pthread_t p1;
	pthread_t p1;
	pthread_t p1;
	pthread_t p1;
	//crear variables hilos aqui
	
	//numero del equipo (casteo el puntero a un int)
	int p = *((int *) i);
	
	printf("Ejecutando equipo %d \n", p);

	//reservo memoria para el struct
	struct parametro *pthread_data = malloc(sizeof(struct parametro));

	//seteo los valores al struct
	
	//seteo numero de grupo
	pthread_data->equipo_param = p;

	//seteo semaforos
	pthread_data->semaforos_param.sem_picar_vegetales = sem_picar_vegetales;
	pthread_data->semaforos_param.sem_mezclar = sem_mezclar;	
	//pthread_data->semaforos_param.sem_sazonar_mezcla = sem_sazonar_mezcla;
	pthread_data->semaforos_param.sem_agregar_carne = sem_agregar_carne;
	pthread_data->semaforos_param.sem_empanar_carne = sem_empanar_carne;
	//pthread_data->semaforos_param.sem_freir_milanesa = sem_freir_milanesa;
	//pthread_data->semaforos_param.sem_hornear_pan = sem_hornear_pan;
	pthread_data->semaforos_param.sem_cortar_vegetales = sem_cortar_vegetales;
	pthread_data->semaforos_param.sem_armar_sandwich = sem_armar_sandwich;
	//setear demas semaforos al struct aqui
	
	//FILE *origen;
	//char[] linea;

	//recetaTXT= fopen("receta.txt","r");
	//if (origen == NULL || destino == NULL) {
	//	printf( "Problemas con la apertura de los ficheros.\n" );
	//	exit( 1 );
	//}

	//linea = gets(origen);
	//while (feof(origen) == 0) {
	//	puts(letra, destino);
	//	printf( "%c",letra );
	//	letra = getc(origen);
	//}
	
	struct accion acciones[LIMITE_ACCIONES];
	obtenerReceta(acciones);
	
	for(j = 0; j < LIMITE_ACCIONES; j++){
		strcpy(pthread_data->pasos_param[j].accion, acciones[j].nombre_accion);	
		for(x = 0; x < LIMITE_INGREDIENTES; x++){		
			strcpy(pthread_data->pasos_param[j].ingredientes[x], acciones[j].ingrediente[x].nombre_ingrediente);
		}		
	}	
	

	//seteo las acciones y los ingredientes (Faltan acciones e ingredientes) ¿Se ve hardcodeado no? ¿Les parece bien?
    //strcpy(pthread_data->pasos_param[0].accion, "cortar");
	//strcpy(pthread_data->pasos_param[0].ingredientes[0], "ajo");
    //strcpy(pthread_data->pasos_param[0].ingredientes[1], "perejil");


	//strcpy(pthread_data->pasos_param[1].accion, "mezclar");
	//strcpy(pthread_data->pasos_param[1].ingredientes[0], "ajo");
    //strcpy(pthread_data->pasos_param[1].ingredientes[1], "perejil");
 	//strcpy(pthread_data->pasos_param[1].ingredientes[2], "huevo");
	//strcpy(pthread_data->pasos_param[1].ingredientes[3], "carne");
	
	
	//inicializo los semaforos
	sem_init(&(pthread_data->semaforos_param.sem_picar_vegetales),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_mezclar),0,0);
	//sem_init(&(pthread_data->semaforos_param.sem_sazonar_mezcla),0,0);   //sem compartido
	sem_init(&(pthread_data->semaforos_param.sem_agregar_carne),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_empanar_carne),0,0);
	//sem_init(&(pthread_data->semaforos_param.sem_freir_milanesa),0,0);   //sem compartido
	//sem_init(&(pthread_data->semaforos_param.sem_hornear_pan),0,0);      //sem compartido
	sem_init(&(pthread_data->semaforos_param.sem_cortar_vegetales),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armar_sandwich),0,0);
	//inicializar demas semaforos aqui


	//creo los hilos a todos les paso el struct creado (el mismo a todos los hilos) ya que todos comparten los semaforos 
    int rc;
    rc = pthread_create(&p1,                           //identificador unico
                            NULL,                          //atributos del thread
                                cortar,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	//crear demas hilos aqui
	
	
	//join de todos los hilos
	pthread_join (p1,NULL);
	//crear join de demas hilos


	//valido que el hilo se alla creado bien 
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
     }

	 
	//destruccion de los semaforos 
	sem_destroy(&sem_mezclar);
	//destruir demas semaforos 
	
	//salida del hilo
	 pthread_exit(NULL);
}


int main ()
{
	sem_t sem_sazonar_mezcla;
	sem_t sem_freir_milanesa;
	sem_t sem_hornear_pan;
	semaforos_c->sem_sazonar_mezcla = sem_sazonar_mezcla;
	semaforos_c->sem_freir_milanesa = sem_freir_milanesa;
	semaforos_c->sem_hornear_pan = sem_hornear_pan;
	
	sem_init(&(semaforos_c->sem_sazonar_mezcla),0,1);   //sem compartido
	sem_init(&(semaforos_c->sem_freir_milanesa),0,1);   //sem compartido
	sem_init(&(semaforos_c->sem_hornear_pan),0,2);      //sem compartido
	
	//creo los nombres de los equipos 
	int rc;
	int *equipoNombre1 =malloc(sizeof(*equipoNombre1));
	int *equipoNombre2 =malloc(sizeof(*equipoNombre2));
//faltan equipos
  
	*equipoNombre1 = 1;
	*equipoNombre2 = 2;

	//creo las variables los hilos de los equipos
	pthread_t equipo1; 
	pthread_t equipo2;
//faltan hilos
  
	//inicializo los hilos de los equipos
    rc = pthread_create(&equipo1,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre1); 

    rc = pthread_create(&equipo2,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre2);
  //faltn inicializaciones


   if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
     } 

	//join de todos los hilos
	pthread_join (equipo1,NULL);
	pthread_join (equipo2,NULL);
//.. faltan joins


    pthread_exit(NULL);
}


//Para compilar:   gcc subwayArgento.c -o ejecutable -lpthread
//Para ejecutar:   ./ejecutable
