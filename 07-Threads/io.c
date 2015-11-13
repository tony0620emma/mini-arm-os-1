#include <stdint.h>
#include "malloc.h"
#include "reg.h"

extern void print_str(const char *);

void USART2_handler(void)
{
	char *str = malloc(2);
	str[0] = *(USART2_DR) & 0xff;
	str[1] = '\0';
	print_str(str);
	print_str("USART2 interrupt!!\n");
	free(str);
}
