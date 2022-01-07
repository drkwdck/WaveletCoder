// Simplest & slowest version of arithmetic codec (almost demo version)
#include <stdio.h>
#include <iostream>
#include "lib/wavelet2s.h"
//для избежания переполнения:	MAX_FREQUENCY * (TOP_VALUE+1) < ULONG_MAX 
//число MAX_FREQUENCY должно быть не менее, чем в 4 раза меньше TOP_VALUE 
//число символов NO_OF_CHARS должно быть много меньше MAX_FREQUENCY 
#define BITS_IN_REGISTER			17	
#define TOP_VALUE					(((unsigned long)1<<BITS_IN_REGISTER)-1)	// 1111...1
#define FIRST_QTR					((TOP_VALUE>>2) +1)							// 0100...0
#define HALF						(2*FIRST_QTR)								// 1000...0
#define THIRD_QTR					(3*FIRST_QTR)								// 1100...0
#define MAX_FREQUENCY				((unsigned)1<<15)
#define NO_OF_CHARS					256
#define EOF_SYMBOL					NO_OF_CHARS			// char-коды: 0..NO_OF_CHARS-1 
#define NO_OF_SYMBOLS				(NO_OF_CHARS+1)		// + EOF_SYMBOL

unsigned long						low, high, value;
int									buffer, bits_to_go, garbage_bits, bits_to_follow;
unsigned int						cum_freq[NO_OF_SYMBOLS+1];	//интервалы частот символов
// относительная частота появления символа s (оценка вероятности его появления)
// определяется как p(s)=(cum_freq[s+1]-cum_freq[s])/cum_freq[NO_OF_SYMBOLS]

FILE *in, *out; 

void start_model(void)	
{ // исходно все символы в сообщении считаем равновероятными 
	for (int i=0; i<=NO_OF_SYMBOLS; i++)	cum_freq[i]=i;
}
inline void update_model(int symbol)
{
        if (cum_freq[NO_OF_SYMBOLS]==MAX_FREQUENCY) 
        {	//	масштабируем частотные интервалы, уменьшая их в 2 раза
            int cum=0; 
            for (int i=0; i<NO_OF_SYMBOLS; i++)
                {
					int fr=(cum_freq[i+1]-cum_freq[i]+1)>>1;
					cum_freq[i]=cum;
					cum+=fr;
                }
			cum_freq[NO_OF_SYMBOLS]=cum;
        }
		// обновление интервалов частот
        for (int i=symbol+1; i<=NO_OF_SYMBOLS; i++) cum_freq[i]++;
}

inline int input_bit(void) // ввод 1 бита из сжатого файла
{
        if (bits_to_go==0)
        {
                buffer=getc(in);	// заполняем буфер битового ввода
                if (buffer==EOF)	// входной поток сжатых данных исчерпан !!!
                {
									// Причина попытки дальнейшего чтения: следующим 
									// декодируемым символом должен быть EOF_SYMBOL,
									// но декодер об этом пока не знает и может готовиться 
									// к дальнейшему декодированию, втягивая новые биты 
									// (см. цикл for(;;) в процедуре decode_symbol). Эти 
									// биты - "мусор", реально не несут никакой 
									// информации и их можно выдать любыми
                        garbage_bits++; 
                        if (garbage_bits>BITS_IN_REGISTER-2)
                        {	// больше максимально возможного числа мусорных битов
                                printf("ERROR IN COMPRESSED FILE !!! \n");
                                return -1;
                        }
						bits_to_go=1;
                }
                else bits_to_go=8;
        }
        int t=buffer&1;
        buffer>>=1;
        bits_to_go--;
        return t;
}

inline void output_bit(int bit)			// вывод одного бита в сжатый файл
{
        buffer=(buffer>>1)+(bit<<7);	// в битовый буфер (один байт)
        bits_to_go--;
        if (bits_to_go==0)				// битовый буфер заполнен, сброс буфера
        {
                putc(buffer,out);
                bits_to_go=8;
        }
}
inline void output_bit_plus_follow(int bit)
{		// вывод одного очередного бита и тех, которые были отложены
        output_bit(bit);
        while (bits_to_follow>0)
        {
                output_bit(!bit);
                bits_to_follow--;
        }
}

void start_encoding(void)
{
        bits_to_go		=8;				// свободно бит в битовом буфере вывода
        bits_to_follow	=0;				// число бит, вывод которых отложен
        low				=0;				// нижняя граница интервала
        high			=TOP_VALUE;		// верхняя граница интервала
}
void done_encoding(void)
{
        bits_to_follow++;
        if ( low < FIRST_QTR ) output_bit_plus_follow(0);
        else output_bit_plus_follow(1);
        putc(buffer>>bits_to_go,out);				// записать незаполненный буфер
}

void start_decoding(void)
{
        bits_to_go		=0;				// свободно бит в битовом буфере ввода
        garbage_bits	=0;				// контроль числа "мусорных" бит в конце сжатого файла
        low				=0;				// нижняя граница интервала
        high			=TOP_VALUE;		// верхняя граница интервала
        value			=0;				// "ЧИСЛО"
        for (int i=0; i<BITS_IN_REGISTER; i++) value=(value<<1)+input_bit();
}

void encode_symbol(int symbol)
{
		// пересчет границ интервала
        unsigned long range;
        range=high-low+1;
        high=low	+range*cum_freq[symbol+1]/cum_freq[NO_OF_SYMBOLS] -1;
        low	=low	+range*cum_freq[symbol]/cum_freq[NO_OF_SYMBOLS];
		// далее при необходимости - вывод бита или меры от зацикливания
        for (;;)
        {			// Замечание: всегда low<high
                if (high<HALF) // Старшие биты low и high - нулевые (оба)
									output_bit_plus_follow(0); //вывод совпадающего старшего бита
                else if (low>=HALF) // Старшие биты low и high - единичные
                {	 
                        output_bit_plus_follow(1);	//вывод старшего бита
                        low-=HALF;					//сброс старшего бита в 0
                        high-=HALF;					//сброс старшего бита в 0
                }
                else if (low>=FIRST_QTR && high < THIRD_QTR)
                {		/* возможно зацикливание, т.к. 
								 HALF<=high<THIRD_QTR,	i.e. high=10...
								 FIRST_QTR<=low<HALF,	i.e. low =01...
							выбрасываем второй по старшинству бит	*/
                        high-=FIRST_QTR;		// high	=01...
                        low-=FIRST_QTR;			// low	=00...
                        bits_to_follow++;		//откладываем вывод (еще) одного бита
						// младший бит будет втянут далее
                }
                else break;		// втягивать новый бит рано 
				//	старший бит в low и high нулевой, втягиваем новый бит в младший разряд 
                low<<=1;			// втягиваем 0
                (high<<=1)++;		// втягиваем 1
        }
}

int decode_symbol(void)
{
        unsigned long range, cum;
        int symbol;
        range=high-low+1;
					// число cum - это число value, пересчитанное из интервала
					// low..high в интервал 0..CUM_FREQUENCY[NO_OF_SYMBOLS]
		cum=( (value-low+1)*cum_freq[NO_OF_SYMBOLS] -1 )/range;
					// поиск интервала, соответствующего числу cum
		for (symbol=0; cum_freq[symbol+1] <= cum; symbol++);
					// пересчет границ
        high=low +range*cum_freq[symbol+1]/cum_freq[NO_OF_SYMBOLS] -1;
        low =low +range*cum_freq[symbol]/cum_freq[NO_OF_SYMBOLS];
        for (;;)
        {		// подготовка к декодированию следующих символов
                if (high<HALF) {/* Старшие биты low и high - нулевые */}
                else if (low>=HALF) 
                {		// Старшие биты low и high - единичные, сбрасываем
                        value-=HALF;
                        low-=HALF;
                        high-=HALF;
                }
                else if (low>=FIRST_QTR && high<THIRD_QTR)
                {		// Поступаем так же, как при кодировании
                        value-=FIRST_QTR;
                        low-=FIRST_QTR;
                        high-=FIRST_QTR;
                }
                else break;	// втягивать новый бит рано 
				low<<=1;							// втягиваем новый бит 0
                (high<<=1)++;						// втягиваем новый бит 1
                value=(value<<1)+input_bit();		// втягиваем новый бит информации
        }
        return symbol;
}

void encode(void)
{
        int symbol;
        start_model();
        start_encoding();
        while ( (symbol=getc(in))!=EOF )
        {
                encode_symbol(symbol);
                update_model(symbol);
        }
        encode_symbol(EOF_SYMBOL);
        done_encoding();
}

void decode(void)
{
        int symbol;
        start_model();
        start_decoding();
        while ((symbol=decode_symbol())!=EOF_SYMBOL)
        {
                update_model(symbol);
                putc(symbol,out);
        }
}

int main(int argc, char **argv)
{
        printf("\nAlpha version of arithmetic Codec\n");

        if ( argc!=4 ||  argv[1][0]!='e' && argv[1][0]!='d' ) {
                printf("\nUsage: arcode e|d infile outfile \n");
        }
        else if ( (in=fopen(argv[2],"r+b"))==NULL ) {
                printf("\nIncorrect input file\n");
        }
        else if ( (out=fopen(argv[3],"w+b"))==NULL ) {
                printf("\nIncorrect output file\n");
        }
        else {
                if (argv[1][0]=='e') encode(); else decode();
                fclose(in);
                fclose(out);
        }

        return 0;
}