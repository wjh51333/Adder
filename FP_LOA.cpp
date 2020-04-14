#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <random>
#include <iostream>
#include <time.h>
//#include <values.h>

#define mask 0xFFF
#define bitnum 12

/* endianness testing */
const int EndianTest = 0x04030201;
#define LITTLE_ENDIAN() (*((const char *) &EndianTest) == 0x01)
/* extract nth LSB from object stored in lvalue x */
#define GET_BIT(x, n) ((((const char *) &x)[LITTLE_ENDIAN() ? (n) / CHARBITS : sizeof(x) - (n) / CHARBITS - 1] >> ((n) % CHARBITS)) & 0x01)

#define PUT_BIT(x, n) (putchar(GET_BIT((x), (n)) ? '1' : '0'))

#define not_real_number printf("Not real number\n"); \
						exit(1);

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 generator(rd()); //Standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<int>  RandomExponent(0, 254);
std::uniform_int_distribution<int>  RandomMantisa(0, 0x7FFFFF);
std::uniform_int_distribution<int>  RandomSign(0, 1);


int nnn = 1;
int checknum = 0;

//Union ���
typedef union {
	float f;
	struct {
		unsigned int mantisa : 23;
		unsigned int exponent : 8;
		unsigned int sign : 1;
	} parts;
} float_cast;

float_cast makeFP();
unsigned int loa(unsigned int a, unsigned int b);
float_cast FPAdder(float_cast a, float_cast b);

float_cast makeFP() {
	//exponet => 127(7F) ~ -128(80)
	//mantisa => 524287(7FFFF) ~ 0(0)
	float_cast num;
	num.parts.exponent = RandomExponent(generator); //0~255 -> 2^(exponent - 126) * 1.(mantisa)
													//if exponent == 255 -> inf
	num.parts.mantisa = RandomMantisa(generator);
	num.parts.sign = 0;
	return num;
}

unsigned int loa(unsigned int a, unsigned int b)
{
	unsigned int m, n, sum;
	int carry;

	m = a & mask;
	n = b & mask;

	sum = m | n;
	carry = (m >> bitnum - 1) & (n >> bitnum - 1);

	sum += (a - m) + (b - n) + (carry << bitnum);

	return sum;
}

float_cast FPAdder(float_cast a, float_cast b) {

	//���� �� ���� real number���� �Ǵ��ؾ��Ѵ�. (inf, -inf, 0, -0, NAN)
	//0 FF 000000 -> inf, 1 FF 000000 -> -inf, 00000 -> 0, 100000 -> -0

	//�Է°��� INF�� �� ���ֳ�?
	if (a.parts.exponent == 0xFF && a.parts.sign == 0)  //a is inf 
	{
		not_real_number
	}
	else if (a.parts.exponent == 0xFF && a.parts.sign == 1)//a is -inf 
	{
		not_real_number
	}

	if (b.parts.exponent == 0xFF && b.parts.sign == 0)  //b is inf 
	{
		not_real_number
	}
	else if (b.parts.exponent == 0xFF && b.parts.sign == 1)//b is -inf 
	{
		not_real_number
	}

	float_cast z; //return ��
	z.parts.sign = 0;
	unsigned int sum = 0;
	//���� 0 ���� ���ʿ����.
	if (a.f == 0) //a == 0 || b == 0  return a or b
		z.f = b.f;
	else if (b.f == 0)
		z.f = a.f;


	int subEx = a.parts.exponent - b.parts.exponent;

	if (subEx == 0) {//exponents equal
		checknum = 1;
		z.parts.exponent = a.parts.exponent;
		
		// LOA
		sum = loa(a.parts.mantisa, b.parts.mantisa);

		if (sum == 0)
			z.f = 0;
		else {
			z.parts.mantisa = sum >> 1;
			z.parts.exponent++;

			if (z.parts.exponent >= 0xFF) {//is it overflow?
				printf("Overflow\n");
			}
			else {

			}
			//z.parts.mantisa &= 0x7FFFF;
		}
	}
	else { //shift smaller one to bigger one
		if (subEx > 0) {// a's exponent > b's exponent  => shift mantisa right
						//b.parts.exponent = a.parts.exponent;
			z.parts.exponent = a.parts.exponent;
			if (abs(subEx) >= 23)	//shift �� mantisa�� 23��Ʈ �Ѿ�� 0���� �ʱ�ȭ!
				b.parts.mantisa = 0;
			else {
				//b.parts.mantisa >>= abs(subEx);
				if (b.parts.exponent != 0)
					b.parts.mantisa = (b.parts.mantisa >> 1) + 0x400000;

				if (abs(subEx) > 1)
					b.parts.mantisa >>= abs(subEx) - 1;
			}
		}
		else {// a's exponent < b's exponent => shift mantisa right
			  //a.parts.exponent = b.parts.exponent;
			z.parts.exponent = b.parts.exponent;
			if (abs(subEx) >= 23)//shift �� mantisa�� 23��Ʈ �Ѿ�� 0���� �ʱ�ȭ!
				a.parts.mantisa = 0;
			else {
				//a.parts.mantisa >>= abs(subEx);
				if (a.parts.exponent != 0)
					a.parts.mantisa = (a.parts.mantisa >> 1) + 0x400000;

				if (abs(subEx) > 1)
					a.parts.mantisa >>= abs(subEx) - 1;
			}
		}

		// LOA
		sum = loa(a.parts.mantisa, b.parts.mantisa);
		
		//mantisa + matisa�� 23��Ʈ�� �Ѿ������ �ڵ����� �߶����! (�ֳĸ� union�̴ϱ�)
		//���� �츮�� ���� �Ѿ�� carry���� ó��������Ѵ�.
		if (sum > 0x7FFFFF) {
			z.parts.mantisa = (sum >> 1) - 0x400000;
			z.parts.exponent++;
		}
		else
			z.parts.mantisa = sum;

		if (z.parts.mantisa == 0)
			z.f = 0;
	}
	return z;
}

int main(void) {
	float_cast A, B, ans;
	float_cast orgAns;
	int cnt = 0;
	printf("A\t\t+\t\tB\t=\torgANS\t\tmyANS\n");
	printf("**********************************************************************\n");

	//A = makeFP();
	//B = makeFP();
	//A, B ���� ����

	for (int i = 0; i < 10; i++) {
		A = makeFP();
		B = makeFP();

		orgAns.f = A.f + B.f;
		ans = FPAdder(A, B);

		printf("%d : %e    +    %e    =    %e,   %e\n\n", i + 1, A.f, B.f, orgAns.f, ans.f);
	}
}
/*
sign = 1
exponent = 7e
mantisa = 0*/

/*�����÷� ����÷� ���ð� ���� �� �ճ�
�����÷� ����÷� warning �̳� runtime error �߻���Ű����? ����ó��
�ܼ��� ����ó���ϴ°����� ����.
rounding �� ����? ��� �ϳ���
shift �� �� ��÷�� 1�ְ� , ����� 0���� �ֱ�?
exponent, mantissa �Ѵ� ���������� �Է��ϱ�
c++ �������� �ֱ�
����üũ , med �̷������� �˻��غ��� (ô���� ��� �׷�)
*/



/*
3.4028235E38
~
-3.4028235E38
0_0000_0000_ 11111111111111111 -> 1.1754942E-38
*/
