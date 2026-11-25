#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>


#define WRITE_BUFFER_SIZE 1024

typedef struct{
	int index;
	char write_buffer[WRITE_BUFFER_SIZE];
}stdout_writer;

void write_to_output(stdout_writer *std_writer, char byte);
void flush_writer(stdout_writer *std_writer);
void read_file_number(FILE *file, char *buf, int buf_size, int fortune_to_read);

//---- batch parte
void fill_fortunes_index_from_source(FILE *file, char *read_buf, int read_buf_size, int *fortunes_index_buf, int fortunes_index_buf_size, int total_file_size);
void read_batch_file(char *buf, int buf_size, FILE *fortune_file, int *fortunes_index_buf, int fortunes_index_buf_size, int file_size);
void write_to_output_batch(char *write_buffer, int write_buffer_size, FILE *fortune_file, int small_limit, int big_limit);
int big_limit_noninclusive(int small_limit, int *fortunes_index_buf, int fortunes_index_buf_size, int file_size);

char *file_name, *batch_name;
int number_passed;

typedef enum{
	FILE_FLAG,
	BATCH_FLAG,
	NUMBER_FLAG,
	FLAGS_ENUM_SIZE,
	UNKNOWN_FLAG

}flag;


const char *flag_arr[FLAGS_ENUM_SIZE] = {"-f", "-b", "-n"};

flag check_flag(char *value);
int flag_filler(flag flag_type, char *flag_value);
int parse_flags(char *argv[]);

int main(int argc, char *argv[]){

	if (argc!=5){
		printf("Invalid number of arguments\n");
		return 1;
	}


	if (parse_flags(argv)==1){
		return 1;
	}



	printf("fileName: %s \n", file_name);
	printf("batch_name: %s \n", batch_name);
	printf("number: %d \n", number_passed);


        FILE *file = fopen(file_name, "r");

	if (file==NULL){
		printf("file not opsible to open, check spelling \n");
		return 1;
	}
	int stat(const char *restrict path, struct stat *restrict buf);


	struct stat stat_struct;
	stat(file_name, &stat_struct);



	//asumimso que el file esta bien y sus dos primeros valores son como indican. 
	//<valor de cuantos fortunes hay>
	//<maxSize pero este no lo vamos a usar yo creo>
	
	int fortunes_count;
	int fortunes_max_size;
	if (fscanf(file, "%d", &fortunes_count)!=1){
		printf("scanning of the file not posible \n");
		return 1;
	};

	if (fscanf(file, "%d", &fortunes_max_size)!=1){
		printf("scanning of the file not posible \n");
		return 1;
	};


	if(number_passed > fortunes_count){
		printf("number passed is bigger than fortune count \n");
		return 1;
	}
	printf("fortunes count: %d \n", fortunes_count);
	printf("max fortuen size: %d \n \n", fortunes_max_size);



	char *buffer= malloc(sizeof(char) * fortunes_max_size);
	if (buffer==NULL){
		return 1;
	}
	//A APRTIR DE ACA, nada de dynamic memory allocation
	





	if (number_passed>0){
		read_file_number(file,  buffer, fortunes_max_size, number_passed);
		//read by number logi
	}else{
		int *percentage_indexes_buffer= malloc(sizeof(int) * fortunes_count);


		struct stat batch_stat_struct;
		stat(batch_name, &batch_stat_struct);

		if ((int)batch_stat_struct.st_size <=0){
			printf("error, file batch empty");
			return 1; 
		}
		printf("size de batch: %zu \n", batch_stat_struct.st_size);

 		fill_fortunes_index_from_source(file, buffer, fortunes_max_size, percentage_indexes_buffer, fortunes_count, (int)stat_struct.st_size);
		read_batch_file(buffer, fortunes_max_size, file, percentage_indexes_buffer, fortunes_count, (int)stat_struct.st_size);


		/* Leer el archivo, determinar si hay algun valor que no coincide. el check del size queda en todo.  
		 *Leo el archivo y puedo tomar

3. Como lo podriamos hacer con static memory allocation. 

- Recibimos el file
- leemos el file  de source para guardar en un array acorde al index y el %, cada starting point de cada fortune 
- guardamos eso en el array que creamos dinamicamente. Lo creamos sin ningun realloc ni nada, proqeu sabemos cuantos cosos hay
- por cada numero del batch, vamos leyendo. Podriamos leerlo tambien en un buffer. 

- Leemos ese buffer con los bytes creados 
- por cada numero que leemos. 


POdriamos hacer un fscan para probar, pero seria mas costoso. 

- 




		 * hago un ordenamiento del array, y voy imprimiendo. POrque? porque si no lo rodeno debo subir y bajar y asi. ordenarlo es mas simple
		 * */

		// prepare to read the file
	}

	free(buffer);
	fclose(file);
	return 0;
}

int parse_flags(char *argv[]){


	flag flag_1 = check_flag(argv[1]);
	flag flag_2 = check_flag(argv[3]);

	if (flag_1 == UNKNOWN_FLAG || flag_2 == UNKNOWN_FLAG){
		printf("invalid flags passed \n");
		return 1;
	}

	if (flag_2 == flag_1){
		printf("error same flags provided \n");
		return 1;
	}



	if (flag_filler(flag_1, argv[2]) ==1 ||  flag_filler(flag_2, argv[4]) ==1 ) {
		printf("flag filler failed \n");
		return 1;

	}


	return 0;

}

flag check_flag(char *value){
	for (int i=0; i<FLAGS_ENUM_SIZE+1; i++) {
		if (strcmp(flag_arr[i], value)==0) {
			return i;
		
		}
	
	}

	return UNKNOWN_FLAG;
}



int flag_filler(flag flag_type, char *flag_value){

	if (flag_value==NULL){
		printf("flag_value is NULL \n");
		return 1;
	}

	switch (flag_type) {
		case FILE_FLAG:
			if (file_name!=NULL){
				printf("ASSERTION - file name, already passed \n");
				return 1;
			}

			file_name = flag_value;
			break;

		case BATCH_FLAG:
			if (batch_name!=NULL){
				printf("ASSERTION - batch name, already passed \n");
				return 1;
			}

			if (number_passed>0){
				printf("Already using -n flag, you can not use the -b flag at the same time \n");
				return 1;
			}

			batch_name = flag_value;
			break;

		case NUMBER_FLAG:
			if (number_passed!=0){
				printf("ASSERTION - numer should be always cero here, its %d  \n", number_passed);
				return 1;
			}
			if (batch_name!=NULL){
				printf("Already using -b flag, you can NOT use the -n flag at the same time \n");
			}
			
			number_passed = atoi(flag_value);

			if (number_passed<=0){
				printf("invalid number, should be greater than cero \n");
				return 1;
			}
			break;

		default:

			printf("this is an assertoin, this should never reach this function");
			return 1;
	
	}


	return 0;


}





void read_file_number(FILE *file, char *buf, int buf_size, int fortune_to_read){

	int percentage_counter=0;
	int read_bytes;
	stdout_writer std_writer = {.index = 0, .write_buffer = {0}};


	while ((read_bytes = fread(buf, sizeof(char), buf_size, file))> 0 ) {
		

		if (percentage_counter>fortune_to_read){
			flush_writer(&std_writer);
			return;
		}
		//recorremos el buffer 
		for (int i=0; i<read_bytes; i++){
			if (buf[i]=='%'){
				percentage_counter++;
			}

			if (percentage_counter>fortune_to_read){
				flush_writer(&std_writer);
				return;
			}



			if (percentage_counter==fortune_to_read){
				write_to_output(&std_writer, buf[i]);
			}



		}

	
	}
}


void write_to_output(stdout_writer *std_writer, char byte){
	assert(std_writer->index < WRITE_BUFFER_SIZE-1);
	assert(std_writer->index >=0);

	if (std_writer->index == WRITE_BUFFER_SIZE){
		flush_writer(std_writer);
	}


	std_writer->write_buffer[std_writer->index] = byte;
	std_writer->index++;

}

void flush_writer(stdout_writer *std_writer){
	fwrite(std_writer->write_buffer, sizeof(char), (std_writer->index), stdout);
	fflush(stdout);
	std_writer->index=0;
}





//---cosas de batch


void fill_fortunes_index_from_source(FILE *file, char *read_buf, int read_buf_size, int *fortunes_index_buf, int fortunes_index_buf_size, int total_file_size){

	int bytes_read;
	int curr_fortune_index = 0;
       	int file_byte_index=0;

	// int posiblePercentage=0;

	int se = fseek(file, 0, SEEK_SET);

	char percentage_pattern[3]="\n%\n";
	int checkIndex=0;






	/*                           -
	 *  [ A, A, \n, %, A, \n, \n, %, \n]      
	 *
	 *  checkIndex=0;
	 *
	 *
	 *
	 *
	 * */
	printf("seek starts en: %d \n", se);
	while ((bytes_read = fread(read_buf, sizeof(char), read_buf_size, file)) > 0 ) {
		for (int i=0; i<bytes_read; i++) {

			if (read_buf[i] == percentage_pattern[checkIndex]){
				checkIndex++;
			}else{
				checkIndex=0;
				//weird edge case cuando tenemos por ejmeplo
	 			// *  [ \n, \n, %, \n]      
				if (read_buf[i] == percentage_pattern[checkIndex]){
					checkIndex++;
				}
			}

			if (checkIndex==3){
				fortunes_index_buf[curr_fortune_index] = file_byte_index;
				curr_fortune_index++;
			}

       			file_byte_index++;

		}




	}

	assert(file_byte_index==total_file_size);
	assert(curr_fortune_index == fortunes_index_buf_size);

}





void read_batch_file(char *buf, int buf_size, FILE *fortune_file, int *fortunes_index_buf, int fortunes_index_buf_size, int file_size){
	assert(batch_name!=NULL);

        FILE *file = fopen(batch_name, "r");

	int number;

	while (fscanf(file, "%d", &number)> 0 ) {
		if (number>fortunes_index_buf_size || number<1){
			printf("Invalid number on batch: %d \n", number);
			return ;
		}
		//becasue our index on teh batch file starts at 1
		number--;

		printf("\n------------- Imprimiendo el fortunes: #%d---------\n", number+1);
		int big_limit = big_limit_noninclusive(number, fortunes_index_buf, fortunes_index_buf_size, file_size);
								//small limit   ------------ big limit
 		write_to_output_batch(buf, buf_size, fortune_file, fortunes_index_buf[number], big_limit);
		printf("\n\n");
	}

}


//returns the BYTE of the big limit of the file. (not an index)
int big_limit_noninclusive(int small_limit_index, int *fortunes_index_buf, int fortunes_index_buf_size, int file_size){

	assert(small_limit_index>=0);
	assert(small_limit_index <=fortunes_index_buf_size);

	if (small_limit_index == fortunes_index_buf_size-1){
		return file_size;
	}

	//overflow not posible because we already checked the biggest one previously and sizes
	return fortunes_index_buf[small_limit_index+1]-1;
}




//write buffer size es el size del length de la fortune mas grande, por lo tanto si o si deberia caber todo en ese buffer ya allocado
void write_to_output_batch(char *write_buffer, int write_buffer_size, FILE *fortune_file, int small_limit, int big_limit){

	int total_bytes_to_write = big_limit - small_limit;
	int bytes_writen = 0;


	fseek(fortune_file, small_limit, SEEK_SET);

	while (bytes_writen < total_bytes_to_write) {
		int bytes_read;
		// verificar si podemos escribir todo de una vez, o toca llenar el buffer completo y continuar
		if (total_bytes_to_write > write_buffer_size){
	 		bytes_read = fread(write_buffer, sizeof(char), write_buffer_size, fortune_file);
		}else{
	 		bytes_read = fread(write_buffer, sizeof(char), total_bytes_to_write, fortune_file);
		}
		bytes_writen+=bytes_read;
		//flush the output. 
		fwrite(write_buffer, sizeof(char), bytes_read, stdout);
		fflush(stdout);
	}
}



