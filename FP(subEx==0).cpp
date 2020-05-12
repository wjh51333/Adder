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


//Union ���
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
	if (subEx >= 23)   //shift �� mantissa�� 23��Ʈ �Ѿ�� 0���� �ʱ�ȭ!
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
	int ext_bit[3] = { 0, }; // guard, round, sticky bit
	//���� 0 ���� ���ʿ����.
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
			extbit_cal(sum, abs(subEx), ext_bit);
			break;
		case 2:
			sum = LOA(a.parts.mantissa, b.parts.mantissa);
			break;
		case 3:
			sum = ETA1(a.parts.mantissa, b.parts.mantissa);
			break;
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
	}
	unsigned int z_mantissa;

	//mantissa + mantissa�� 23��Ʈ�� �Ѿ������ �ڵ����� �߶����! (�ֳĸ� union�̴ϱ�)
	//���� �츮�� ���� �Ѿ�� carry���� ó��������Ѵ�.
	if (sum > 0x7FFFFF) {
		if ((sum & 0xc00000) == 0x800000) {
			printf("1\n");
			ext_bit[0] = ext_bit[0] | ext_bit[1]; // sticky bit = sum[1] | sum[0]
			ext_bit[1] = ext_bit[2]; // round bit
			ext_bit[2] = (sum & 1) ? 1 : 0; // guard bit
			z.parts.mantissa = (sum >> 1);
			z.parts.exponent++;
		}
		else {
			printf("2\n");
			z.parts.mantissa = sum >> 1;
			z.parts.exponent++;
		}
	}
	else {
		if (a.parts.sign == b.parts.sign) {
			printf("3\n");
			ext_bit[0] = ext_bit[1]; // round bit
			ext_bit[1] = ext_bit[2]; // round bit
			ext_bit[2] = (sum & 1) ? 1 : 0; // guard bit
			z.parts.mantissa = sum >> 1;
			z.parts.exponent++;
		}
		else {
			int cnt;
			printf("4\n");
			for (cnt = 1; sum & 0x400000 ? 0 : 1; cnt++) {
				if (z.parts.exponent - cnt == 0)
					break;
				sum <<= 1;
			}
			if (z.parts.exponent - cnt == 0)
				z.parts.mantissa = sum;
			else
				z.parts.mantissa = (sum << 1) & 0x7FFFFF;
			z.parts.exponent -= cnt;
		}
	}

	/*normalize*/

	//overflow!
	if ((z_mantissa = z.parts.mantissa & 0x400000) == 0 && (z.parts.exponent >= 0xFF)) {
		printf("\noverflow!\n");
		z.parts.mantissa <<= 1;
		z.parts.exponent--;
		z.parts.mantissa += ext_bit[2];
		ext_bit[2] = ext_bit[1];
		ext_bit[1] = 0;
	}
	//underflow!
	else if (z.parts.exponent > 0xFF) {
		printf("\nunderflow!\n");
		ext_bit[2] = z.parts.mantissa & 1;
		ext_bit[1] = ext_bit[2];
		ext_bit[0] = ext_bit[0] | ext_bit[1];
		z.parts.mantissa >>= 1;
		z.parts.exponent++;
	}

	// guard && (round bit | sticky | z_m[0])
	if (ext_bit[2] && (ext_bit[1] | ext_bit[0] | (z.parts.mantissa & 1))) {
		printf("5\n");
		z.parts.mantissa++;
		if (z.parts.mantissa >= 0xffffff)
			z.parts.exponent++;
	}

	if (z.parts.mantissa == 0) {
		printf("6\n");
		z.f = 0;
	}

	//if (z.parts.exponent == 0xff)
	//	not_real_number;
	return z;
}

int main(void) {
	float_cast A, B;
	float_cast ans, loa, eta1;
	float_cast orgAns;
	//FILE* input = fopen("input(subEx=0).txt", "r");
	//FILE* output = fopen("ErrorOutput(subEx=0).txt", "w");
	int cnt = 0;
	printf("A\t\t+\t\tB\t=\torgANS\t\tmyANS\t\tLOA\t\tETA1\n");
	printf("**********************************************************************\n");

	//A = makeFP();
	//B = makeFP();
	//A, B ���� ����

	int subEx0_cnt = 1;
	while (subEx0_cnt<=30) {
		//fscanf(input, "%f %f ", &A.f, &B.f);

	//A.f = -1.315185e-20;
	//B.f = -1.179307e-20;
	//A, B ���� ����
	do {
		A = makeFP();
		B = makeFP();
	} while (A.parts.exponent != B.parts.exponent);
	orgAns.f = A.f + B.f;
	ans = FPAdder(A, B, 1);
	//loa = FPAdder(A, B, 2);
	//eta1 = FPAdder(A, B, 3);


	if (checknum == 1) {
		printf("%d: %e    +    %e    =    %e,   %e\n", nnn, A.f, B.f, orgAns.f, ans.f);
		//printf("%d: %e    +    %e    =    %e,   %e,   %e,   %e\n", nnn, A.f, B.f, orgAns.f, ans.f, loa.f, eta1.f);
		checknum = 0;
		if (ans.f != orgAns.f) {
			printf("Error!\n");
			//fprintf(output,"%e %e\n",A.f, B.f);
		}
		printf("\n\n******************************\n");
		subEx0_cnt++;
	}
	else {
		//printf("++%d: %e    +    %e    =    %e,   %e\n", nnn, A.f, B.f, orgAns.f, ans.f);
		//printf("++%d: %e    +    %e    =    %e,   %e,   %e,   %e\n", nnn, A.f, B.f, orgAns.f, ans.f, loa.f, eta1.f);
		//printf("\n\n******************************\n");
	}

	nnn++;
	}

	//fclose(output);
	//fclose(input);
}
/*
sign = 1
exponent = 7e
mantissa = 0*/

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