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
#define emask 0x7FFFFF

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
std::uniform_int_distribution<int>  RandomMantissa(0, 0x7FFFFF);
std::uniform_int_distribution<int>  RandomSign(0, 1);


int nnn = 1;
int checknum = 0;


//Union 사용
typedef union {
	float f;
	struct {
		unsigned int mantissa : 23;
		unsigned int exponent : 8;
		unsigned int sign : 1;
	} parts;
} float_cast;

float_cast makeFP();
float_cast FPAdder(float_cast a, float_cast b, int case_num);
float_cast makeFP() {
	//exponet => 127(7F) ~ -128(80)
	//mantissa => 524287(7FFFF) ~ 0(0)
	float_cast num;
	num.parts.exponent = RandomExponent(generator); //0~255 -> 2^(exponent - 126) * 1.(mantissa)
										//if exponent == 255 -> inf
	num.parts.mantissa = RandomMantissa(generator);
	num.parts.sign = RandomSign(generator);
	return num;
}

void mantissa_cal(float_cast &z, float_cast &x, float_cast &y, int subEx) {
	z.parts.exponent = x.parts.exponent;

	if (subEx > 23)   //shift 가 mantissa의 23비트 넘어서면 0으로 초기화!
		y.parts.mantissa = 0;
	else {
		//a.parts.mantissa >>= abs(subEx);
		if (y.parts.exponent != 0)
			y.parts.mantissa = (y.parts.mantissa >> 1) | 0x400000;

		if (subEx > 1)
			y.parts.mantissa >>= subEx - 1;
	}
}


unsigned int LOA(unsigned int a, unsigned int b)
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

unsigned int ETA1(unsigned int a, unsigned int b)
{
	unsigned int M, N, m, n, inaccuratePart = 0, r, sum = 0;
	int carry, imask = 0x800;

	M = a & mask;
	N = b & mask;

	while (1) {
		m = a & imask;
		n = b & imask;
		r = m ^ n;
		inaccuratePart += r;
		if (m == imask && n == imask) {
			inaccuratePart += imask - 1;
			break;
		}
		imask /= 2;
	}

	sum = (a - M) + (b - N) + inaccuratePart;

	return sum;
}

float_cast AXAdder(float_cast a, float_cast b, int caseNum) {

	//먼저 두 값이 real number인지 판단해야한다. (inf, -inf, 0, -0, NAN)
	//0 FF 000000 -> inf, 1 FF 000000 -> -inf, 00000 -> 0, 100000 -> -0

	//입력값이 INF일 수 가있나?
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

	float_cast z; //return 값
	z.parts.sign = 0;
	unsigned int sum = 0;

	int subEx = a.parts.exponent - b.parts.exponent;
	if (subEx != 0) {//exponents equal
		checknum = 1;
		if (subEx > 0) {// a's exponent > b's exponent  => shift mantissa right
							//b.parts.exponent = a.parts.exponent;
			mantissa_cal(z, a, b, subEx);
			z.parts.sign = a.parts.sign;
		}
		else {// a's exponent < b's exponent => shift mantissa right
			 //a.parts.exponent = b.parts.exponent;
			mantissa_cal(z, b, a, abs(subEx));
			z.parts.sign = b.parts.sign;
		}
	}

	switch (caseNum) {
	case 1: //LOA
		sum = LOA(a.parts.mantissa, b.parts.mantissa);
		break;
	case 2: //ETA1
		sum = ETA1(a.parts.mantissa, b.parts.mantissa);
		break;
	}


	//mantissa + mantissa가 23비트가 넘어가버리면 자동으로 잘라버림! (왜냐면 union이니깐)
	//따라서 우리가 직접 넘어가는 carry값을 처리해줘야한다.
	if (sum > 0x7FFFFF) {
		if (subEx == 0) {
			sum >>= 1;
			z.parts.exponent++;
		}
		else {
			sum = (sum >> 1) & 0x3FFFFF;
			z.parts.exponent++;
		}
	}
	z.parts.mantissa = sum;


	//overflow!
	if (z.parts.mantissa == 0 && (z.parts.exponent >= 0xFF)) {
		//printf("\noverflow!\n");
		z.parts.mantissa = 1 << 23;
		z.parts.exponent = 255;
		return z;
		//z.parts.mantissa <<= 1;
		//z.parts.exponent--;
		//z.parts.mantissa += ext_bit[2];
		//ext_bit[2] = ext_bit[1];
		//ext_bit[1] = 0;
	}
	//underflow!
	else if (z.parts.exponent >= 0xFF) {
		//printf("\nunderflow!\n");
		z.parts.mantissa = 1 << 23;
		z.parts.exponent = 255;
		return z;
		//ext_bit[2] = z.parts.mantissa & 1;
		//ext_bit[1] = ext_bit[2];
		//ext_bit[0] = ext_bit[0] | ext_bit[1];
		//z.parts.mantissa >>= 1;
		//z.parts.exponent++;
	}

	return z;
}


int main(void) {
	float_cast A, B;
	float_cast orgAns, loa, eta1;
	//FILE* input = fopen("input(subEx=0).txt", "r");
	//FILE* output = fopen("Errorlist.txt", "a");
	int cnt = 0;
	printf("A\t\t+\t\tB\t  =\torgANS\t\tLOA\t\tETA1\n");
	printf("**********************************************************************\n");


	while (nnn <= 30) {
		//fscanf(input, "%f %f ", &A.f, &B.f);

		//A, B 직접 지정
		//A.f = 2.845072e-34;
		//B.f = -6.625885e-34;
		do {
			A = makeFP();
			B = makeFP();
		} while (A.parts.sign != 0 || B.parts.sign != 0);
		orgAns.f = A.f + B.f;
		loa = AXAdder(A, B, 1);
		eta1 = AXAdder(A, B, 2);


		if (checknum == 0) {
			printf("%d: %e    +    %e    =    %e,   %e,   %e\n", nnn, A.f, B.f, orgAns.f, loa.f, eta1.f);
			printf("\n\n******************************\n");
		}
		else {
			checknum = 0;
			printf("++%d: %e    +    %e    =    %e,   %e,   %e\n", nnn, A.f, B.f, orgAns.f, loa.f, eta1.f);
			printf("\n\n******************************\n");
		}

		nnn++;
	}
	//fclose(output);
	//fclose(input);
}
