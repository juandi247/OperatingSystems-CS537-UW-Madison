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
	size_t read_bytes;
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
