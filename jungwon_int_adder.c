#include <stdio.h>
#include <time.h>

#define bitnum 16

void show(char *arr, int a)
{
	printf("%s\t%d\t:", arr, a);
	for (int i = bitnum; i > 0; i--)
	{
		printf("%d", ((a & (1 << i - 1)) ? 1 : 0));
		if (i % 4 == 1) printf(" ");
	}
	printf("\n");
}

unsigned int loa(unsigned int a, unsigned int b)
{
	unsigned int c = 0;;
	int carry;
	int i;
	int a_i, b_i;

	for (i = 1; i <= bitnum / 2; i++)
	{
		a_i = (a & (1 << i - 1)) ? 1 : 0;
		b_i = (b & (1 << i - 1)) ? 1 : 0;
		c += (a_i | b_i) << (i - 1);
	}

	carry = a_i & b_i;

	c += ((a >> 8) + (b >> 8) + carry) << 8;

	return c;
}

unsigned int eta1(unsigned int a, unsigned int b)
{
	unsigned int c = 0;;
	int carry;
	int i;
	int a_i, b_i;

	for (i = bitnum / 2; i > 0; i--)
	{
		a_i = (a & (1 << i - 1)) ? 1 : 0;
		b_i = (b & (1 << i - 1)) ? 1 : 0;
		if ((a_i | b_i) == 1)
		{
			while (i > 0)
			{
				c += 1 << (i - 1);
				i--;
			}
			break;
		}
		else c += (a_i | b_i) << (i - 1);
	}


	c += (((a >> 8) + (b >> 8)) << 8);

	return c;
}

int main()
{
	srand(time(NULL));

	unsigned int a = rand() % 65536;
	unsigned int b = rand() % 65536;

	unsigned int ac = a + b;

	show("num A", a);
	show("num B", b);

	show("A + B", ac);

	show("LOA", loa(a, b));
	show("ETA1", eta1(a, b));
}