extern void print_str(const char *);

void USART2_handler(void)
{
	print_str("USART2 interrupt!!\n");
	return;
}
