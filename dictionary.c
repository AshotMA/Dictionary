//-------------------------------------------------------------------------------
//	Программа по управлению толковым словарем
//	Выполнил: Маилян Ашот Арменович
//-------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
// длина слова включая символ конца строки
#define wordLength 129
// длина описания включая символ конца строки
#define descriptionLength 2000
// в динамическом массиве страктур хрянатся слова,и положение слова в файле в байтах
typedef struct Word {
		char data[wordLength];
		long position;
} Word;
// переменная для хранения размера динамического массива структур
unsigned wordCounter = 0;

int Compare( const void *a, const void *b);
void AddWord (FILE* fp, char* newWord);
Word* ReadFile(FILE* fp, Word* words);
Word* WSearchWord(FILE* fp, Word* words, char* key);
_Bool BSearchWord (FILE* fp, Word* words, char* key);
void RemoveWord (char* fileName, Word* word);
void PrintWord (FILE* fp, Word* wordForPrint);

int main(int argc, char* argv[]) {
	// если формат вызова программы не соответствует заданному
	if ( argc != 3 && argc != 4 ) {
		printf("%d\n", argc);
		fprintf( stderr, "Формат вызова программы: ./dictionary <имя_файла_словаря> <слово> <ключ>\n\
Ключи: -r удалить слово, -a добавить слово, без ключа поиск опредения слова\n" );
		exit(1);
		}	
	FILE* out = fopen (argv[1], "rb+");
	// проверка существования файла
	if (out == NULL) {
		fprintf(stderr, "Невозможно открыть файл \"%s\" или он не существует\n", argv[1]);
		out = fopen (argv[1], "a");
		rewind(out);
		fprintf(stdout, "Создан новый файл \"%s\"\n", argv[1]);
		fclose(out);
		return 0;
	}
	// массив где будут хранится все слова из словаря
	Word* words = NULL;
	// Запись слов из файла в массив структур
	words = ReadFile(out, words);
	if (words == NULL) {
		printf("Не удалось считать файл\n");
		exit(1);
	}
	// если не задан ключ
	if (argc == 3) {
		Word* searchResult = WSearchWord(out, words, argv[2]);
 		if(searchResult == NULL) {
 			printf("Слово \"%s\" не было найдено\n", argv[2]);
 		}
 		else {
 			PrintWord(out, searchResult);
 		}
	}
	else {
		// выполнение в зависимости какой ключ задан
		switch (argv[3][1]) {
		 	case 'a': {
		 		// проверка существования слова для исключения дублирования
		 		if(BSearchWord(out, words, argv[2])) {
		 			printf("Слово \"%s\" уже есть в словаре\n", argv[2]);
		 		}
		 		else {
		 			AddWord(out, argv[2]);
		 		}
		 	}
		 	break;
		 	case 'r': {
		 		Word* searchResult = WSearchWord(out, words, argv[2]);
		 		if(searchResult == NULL) {
		 			printf("Слово \"%s\" не было найдено\n", argv[2]);
		 		}
		 		else {
		 			// закрывем файловый поток, чтобы не было конфликтов с функцией удаления
		 			fclose(out);					
					RemoveWord(argv[1], searchResult);
					printf("Слово \"%s\" удалено\n", searchResult->data);
		 		}
		 		free(words);
		 		return 0;
		 	}
		 	break;
		 	default:
		 		printf("Задан неверный ключ. Ключи: -r удалить слово, -a добавить слово, без ключа поиск слова в словаре\n");
		 	break;
		}
	}
	free(words);
	fclose(out);
	return 0;
}
// удаление слова из словаря путем переноса всех данных в новый файл за исключением строки, которую нужно удалить
void RemoveWord (char* fileName, Word* word) {
	// создание нового файла для хранения словаря
	char* tmpFile = "tmpFile";	
	FILE* out = fopen(fileName, "rb");
	FILE* in = fopen (tmpFile, "wb");
	int length = wordLength+descriptionLength+1;
	char buff[length];
	// перенос данных из прежнего файла в новый
	// условие со счетчиком нужно, чтобы в новом файле не было лишней пустой строки
	// т.к. строк в новом файле будет меньше на одну
	while(!feof(out)) {
		if (ftell(out) != word->position) {
			fgets(buff,length,out);
			if(feof(out)) break;
			fputs(buff,in);
			continue;
		}
		else {
			fgets(buff,length,out);
			if(feof(out)) break;
			continue;	
		}
	}
	fclose(out);
	fclose(in);
	remove(fileName);
	rename(tmpFile,fileName);
}
// функция поиска при добавлении нового слова. Нет необходимости возвращать сам элемент массива, важно только знать если ли такое слово
_Bool BSearchWord (FILE* fp, Word* words, char* key) {
	// быстрая сортировка из стандартной библиотеки stdlib.h
	qsort (words, wordCounter, sizeof(Word), (int(*)( const void*, const void*))Compare);
	// двоичный поиск в массиве из стандартной библиотеки stdlib.h
	Word* result = (Word*) bsearch(key, words, wordCounter, sizeof(Word), (int(*)( const void*, const void*))Compare);
	if (result != NULL){
		return (true);
	}	
	else {
		return (false);
	}
}
// поиск слова для его вывода
Word* WSearchWord(FILE* fp, Word* words, char* key) {
	qsort (words, wordCounter, sizeof(Word), (int(*)( const void*, const void*))Compare);
	Word* result = (Word*) bsearch(key, words, wordCounter, sizeof(Word), (int(*)( const void*, const void*))Compare);
	return (result);	
}
// вывод слова и его определения на экран
void PrintWord (FILE* fp, Word* wordForPrint) {
	fseek (fp, wordForPrint->position, SEEK_SET);
	int length = (wordLength + descriptionLength + 1);
	char buff[length];
	fgets(buff, length, fp);
	printf("\n%s\n", buff);
	rewind(fp);
}
// запись слов из файла в массив
Word* ReadFile(FILE* fp, Word* words) {
	char buff[wordLength+descriptionLength+1];
	while (!feof(fp)) {
		// добавление нового элемента массива
		if ( (words = (Word *)realloc(words,sizeof(Word) * (wordCounter+1))) == NULL) {
			fprintf(stderr, "%s\n", "Ошибка выделения паммяти");
			exit(1);
		}
		// запись текущего положения в файле в байтах
		words[wordCounter].position = ftell(fp);
		// получение строки
		fgets (buff, wordLength, fp);
		// получение слова из всей строки
		sscanf(buff,"%s\t",buff);
		// запись слова в массив
		strcpy(words[wordCounter].data, buff);
		wordCounter++;
	}
	return (words);
}
// Добавление нового слова
void AddWord (FILE* fp, char* newWord) {
	char newDescription[descriptionLength];
	printf ("Введите определение: ");
	fgets (newDescription, descriptionLength, stdin);
	// замена символа переноса карретки на символ конца строки
	newDescription[strlen(newDescription)] = '\0';
	// Добавление слова в конец файла
	fseek(fp, -1, SEEK_END);
	fprintf(fp, "%s\t%s\n", newWord, newDescription);
	rewind(fp);
}
// Сравнение двух слов
int Compare( const void *a, const void *b) {
  return strcmp (((Word*)a)->data,((Word*)b)->data);
}