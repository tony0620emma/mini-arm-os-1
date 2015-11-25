#ifndef __IO_H
#define __IO_H

void USART2_attach(int thread_id);
char get_input();
int USART2_is_empty();

void print_str(const char *);
void print_char(const char *);

#endif
