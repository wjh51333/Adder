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

void extbit_cal(unsigned int mantissa, int subEx, int *e)
{
	unsigned int m = emask;
	unsigned int temp = mantissa;

	if (subEx >= 23) {
		e[0] = (temp & emask) ? 1 : 0; // sticky bit
		e[2] = e[1] = 0; // guard, round -> 0
	}
	else {
		m >>= (23 - subEx);
		temp &= m;

		e[0] = (temp & m) ? 1 : 0; // sticky bit
		e[2] = (temp & (1 << (subEx - 1))) ? 1 : 0; // guard bit
		e[1] = (temp & (1 << (subEx - 2))) ? 1 : 0; // round bit
	}
}

void mantissa_cal(float_cast &z, float_cast &x, float_cast &y, int subEx) {
	z.parts.exponent = x.parts.exponent;
	if (subEx >= 23)   //shift 가 mantissa의 23비트 넘어서면 0으로 초기화!
		y.parts.mantissa = 0;
	else {
		//a.parts.mantissa >>= abs(subEx);
		if (y.parts.exponent != 0)
			y.parts.mantissa = (y.parts.mantissa >> 1) | 0x400000;

		if (subEx > 1)
			y.parts.mantissa >>= subEx - 1;
	}
}

unsigned int sum_cal(float_cast &z, float_cast x, float_cast y)
{
	unsigned int sum = 0;

	if (x.parts.exponent < y.parts.exponent) {
		sum = (y.parts.mantissa | 0x800000) - x.parts.mantissa;
		z.parts.sign = y.parts.sign;

		int cnt;
		for (cnt = 1; sum & 0x400000 ? 0 : 1; cnt++)
			sum <<= 1;
		
		sum = (sum << 1) & 0x7FFFFF;
		z.parts.exponent -= cnt;
	}
	else {
		sum = x.parts.mantissa - y.parts.mantissa;
		z.parts.sign = x.parts.sign;
	}

	return sum;
}

float_cast FPAdder(float_cast a, float_cast b, int case_num) {


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
	int ext_bit[3] = { 0, }; // guard, round, sticky bit
	//굳이 0 따로 볼필요없다.
	if (a.f == 0) //a == 0 || b == 0  return a or b
		z.f = b.f;
	else if (b.f == 0)
		z.f = a.f;


	int subEx = a.parts.exponent - b.parts.exponent;
	if (subEx == 0) {//exponents equal
		checknum = 1;
		z.parts.exponent = a.parts.exponent;

		switch (case_num)
		{
		case 1:
			if (a.parts.sign != b.parts.sign) {
				if (a.parts.mantissa >= b.parts.mantissa) {
					sum = a.parts.mantissa - b.parts.mantissa;
					z.parts.sign = a.parts.sign;
				}
				else {
					sum = b.parts.mantissa - a.parts.mantissa;
					z.parts.sign = b.parts.sign;
				}
			}
			else {
				sum = a.parts.mantissa + b.parts.mantissa;
				z.parts.sign = a.parts.sign;
			}
			break;
		case 2:
			sum = LOA(a.parts.mantissa, b.parts.mantissa);
			break;
		case 3:
			sum = ETA1(a.parts.mantissa, b.parts.mantissa);
			break;
		}

		if (sum == 0)
			z.f = 0;
		else {
			if (a.parts.sign == b.parts.sign) {
				z.parts.mantissa = sum >> 1;
				z.parts.exponent++;
			}
			else {
				int cnt;
				for (cnt = 1; sum & 0x400000 ? 0 : 1; cnt++)
					sum <<= 1;
		
				z.parts.mantissa = (sum << 1) & 0x7FFFFF;
				z.parts.exponent -= cnt;
			}

			if (z.parts.exponent >= 0xFF) {//is it overflow?
				printf("Overflow\n");
			}
			else {

			}
			//z.parts.mantissa &= 0x7FFFF;
		}
	}
	else { //shift smaller one to bigger one
		if (subEx > 0) {// a's exponent > b's exponent  => shift mantissa right
					//b.parts.exponent = a.parts.exponent;
			extbit_cal(b.parts.mantissa, subEx, ext_bit);
			mantissa_cal(z, a, b, subEx);
		}
		else {// a's exponent < b's exponent => shift mantissa right
			 //a.parts.exponent = b.parts.exponent;
			extbit_cal(a.parts.mantissa, abs(subEx), ext_bit);
			mantissa_cal(z, b, a, abs(subEx));
		}

		switch (case_num)
		{
		case 1:
			if (a.parts.sign != b.parts.sign) {
				if (a.parts.mantissa > b.parts.mantissa)
					sum = sum_cal(z, a, b);
				else
					sum = sum_cal(z, b, a);

				ext_bit[2] = 0;
			}
			else {
				sum = a.parts.mantissa + b.parts.mantissa;
				z.parts.sign = a.parts.sign;
			}
			break;
		case 2:
			sum = LOA(a.parts.mantissa, b.parts.mantissa);
			break;
		case 3:
			sum = ETA1(a.parts.mantissa, b.parts.mantissa);
			break;
		}

		//mantissa + mantissa가 23비트가 넘어가버리면 자동으로 잘라버림! (왜냐면 union이니깐)
		//따라서 우리가 직접 넘어가는 carry값을 처리해줘야한다.
		if (sum > 0x7FFFFF) {
			ext_bit[0] = ext_bit[0] | ext_bit[1]; // sticky bit = sum[1] | sum[0]
			ext_bit[1] = ext_bit[2]; // round bit
			ext_bit[2] = (sum & 1) ? 1 : 0; // guard bit

			z.parts.mantissa = (sum >> 1) & 0x3FFFFF;
			z.parts.exponent++;
		}
		else
			z.parts.mantissa = sum;


		// guard && (round bit | sticky | z_m[0])
		if (ext_bit[2] && (ext_bit[1] | ext_bit[0] | (z.parts.mantissa & 1)))
			z.parts.mantissa++;
		
		if (z.parts.mantissa == 0)
			z.f = 0;
	}
	return z;
}

int main(void) {
	float_cast A, B;
	float_cast ans, loa, eta1;
	float_cast orgAns;
	FILE* input = fopen("input.txt", "r");
	int cnt = 0;
	printf("A\t\t+\t\tB\t=\torgANS\t\tmyANS\t\tLOA\t\tETA1\n");
	printf("**********************************************************************\n");

	//A = makeFP();
	//B = makeFP();
	//A, B 랜덤 지정


	while (!feof(input)) {
		fscanf(input, "%f %f ", &A.f, &B.f);

		//A.f = -2.603902000000E+17;
		//B.f = 3.452340000000E+17;
		//A, B 직접 지정

		//A = makeFP();
		//B = makeFP();
		orgAns.f = A.f + B.f;
		ans = FPAdder(A, B, 1);
		loa = FPAdder(A, B, 2);
		eta1 = FPAdder(A, B, 3);


		if (checknum == 1) {
			printf("%d: %e    +    %e    =    %e,   %e,   %e,   %e\n", nnn, A.f, B.f, orgAns.f, ans.f, loa.f, eta1.f);
			checknum = 0;
			printf("\n\n******************************\n");
		}
		else {
			printf("++%d: %e    +    %e    =    %e,   %e,   %e,   %e\n", nnn, A.f, B.f, orgAns.f, ans.f, loa.f, eta1.f);
			printf("\n\n******************************\n");
		}

		nnn++;
	}

	fclose(input);
}
/*
sign = 1
exponent = 7e
mantissa = 0*/

/*오버플로 언더플로 예시가 생길 수 잇나
오버플로 언더플로 warning 이나 runtime error 발생시키나요? 예외처리
단순히 에러처리하는것으로 보자.
rounding 은 무엇? 어떻게 하나요
shift 할 때 맨첨엔 1넣고 , 담부턴 0으로 넣기?
exponent, mantissa 둘다 랜덤정수로 입력하기
c++ 랜덤정수 넣기
에러체크 , med 이런식으로 검색해보기 (척도가 없어서 그럼)
*/



/*
3.4028235E38
~
-3.4028235E38
0_0000_0000_ 11111111111111111 -> 1.1754942E-38
*/
