#define _USE_MATH_DEFINES
#define SIZE 512
#include <stdio.h>
#include <math.h>

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <random>
#include <iostream>
#include <time.h>

#define mask 0x1FFF
#define bitnum 13
#define etaimask 0x1000; // 1 0000 0000 0000
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

#define   BITMASK(bitWidth)   ( (unsigned int)(-1) >> (sizeof(unsigned int)*8 - (bitWidth)) )
#define GETSIGN(a)         ( ((a) >> (c_TADD_BW-1)) & BITMASK(1) )
#define EXTSIGN(a)         ( GETSIGN(a) ? ((a) & BITMASK(c_TADD_BW)) | (-1 << c_TADD_BW) : (a) )
#define GETBIT(a, n)      ( ((a) >> n) & BITMASK(1) )

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

void extbit_cal(float_cast x, int subEx, int* e);
void mantissa_cal(float_cast& z, float_cast& x, float_cast& y, int subEx);
unsigned int sum_cal(float_cast& z, float_cast x, float_cast y, int* e);

float_cast FPAdder(float fa, float fb);
float_cast AppAdder(float fa, float fb, int caseNum);
int exp_cal(unsigned int x, unsigned int y);

/////////////////////////////////////////////////////////////////////////////////////////


void main() {
	float_cast A, B;
	float_cast orgAns, FP, loa, eta1;
	const int c_total_rand = 1000;

	printf("A\t\t+\t\tB\t  =\torgANS\t\tFP\t\tLOA\t\tETA1\n");
	printf("**********************************************************************\n");

	for (int i = 0; i < c_total_rand; i++) {
		//A = makeFP();
		//B = makeFP();

		A.f = -1;
		B.f = 0;

		orgAns.f = A.f + B.f;
		FP = FPAdder(A.f, B.f);

		loa = AppAdder(A.f, B.f, 1);
		eta1 = AppAdder(A.f, B.f, 2);

		printf("%d: %e    +    %e    =    %e,   %e,   %e,   %e\n", i + 1, A.f, B.f, orgAns.f, FP.f, loa.f, eta1.f);
		printf("\n\n******************************\n");
	}
}


//////////////////////////////////////////////////////////////////////////////
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

void extbit_cal(float_cast x, int subEx, int* e)
{
	unsigned int m = emask;
	unsigned int temp = x.parts.mantissa;

	if (x.parts.exponent == 0)
		subEx--;

	if (subEx >= 25) {
		if (x.parts.exponent != 0) {
			m |= 0x800000; temp |= 0x800000;
		}

		e[0] = (temp & m) ? 1 : 0; // sticky bit

								   // round bit
		if (subEx == 25 && x.parts.exponent != 0)
			e[1] = 1;
		else
			e[1] = 0;

		e[2] = 0; // guard bit -> 0
	}
	else if (subEx == 24) {
		e[0] = (temp & (m >> 1)) ? 1 : 0; // sticky bit
		e[1] = (temp & (1 << 22)) ? 1 : 0; // round bit

										   // guard bit
		if (x.parts.exponent != 0)
			e[2] = 1;
		else
			e[2] = 0;

	}
	else {
		m >>= (23 - subEx);
		temp &= m;
		if (subEx >= 2) {
			e[0] = (temp & (m >> 2)) ? 1 : 0; // sticky bit
			e[1] = (temp & (1 << (subEx - 2))) ? 1 : 0; // round bit
		}
		e[2] = (temp & (1 << (subEx - 1))) ? 1 : 0; // guard bit
	}
}

void mantissa_cal(float_cast& z, float_cast& x, float_cast& y, int subEx) {
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

unsigned int sum_cal(float_cast& z, float_cast x, float_cast y, int* e)
{
	unsigned int sum = 0, temp;
	unsigned int esum = 0;

	if (e == NULL) {
		sum = x.parts.mantissa - y.parts.mantissa;
		z.parts.sign = x.parts.sign;
		z.parts.exponent = x.parts.exponent;
		return sum;
	}

	for (int i = 0; i < 3; i++)
		esum += e[i] << i;

	if (x.parts.exponent < y.parts.exponent) {
		sum = ((y.parts.mantissa | 0x800000) << 3) - ((x.parts.mantissa << 3) + esum);
		z.parts.sign = y.parts.sign;

		int cnt;
		temp = sum;
		for (cnt = 1; temp & 0x2000000 ? 0 : 1; cnt++)
			temp <<= 1;

		if (cnt >= 3) {
			for (int i = 0; i < 3; i++)
				e[i] = 0;

			sum <<= cnt - 3;
		}
		else {
			esum = sum & (0x7 >> cnt);

			for (int i = cnt; i < 3; i++)
				e[i] = (esum & (1 << (i - cnt))) ? 1 : 0;

			for (int i = 0; i < cnt; i++)
				e[i] = 0;

			sum >>= 3 - cnt;
		}

		sum &= 0x7FFFFF;
		int tempSum = z.parts.exponent - cnt;
		if (tempSum <= 0) {
			z.parts.exponent = 0;
			sum >>= (abs(tempSum) + 1);
			sum |= (0x400000 >> abs(tempSum));
		}
		else
			z.parts.exponent -= cnt;
	}
	else {
		sum = (x.parts.mantissa << 3) - ((y.parts.mantissa << 3) + esum);
		z.parts.sign = x.parts.sign;

		esum = sum & 0x7;
		for (int i = 0; i < 3; i++)
			e[i] = (esum & (1 << i)) ? 1 : 0;
		sum >>= 3;
	}

	return sum;
}

float_cast FPAdder(float fa, float fb) {


	float_cast a, b;
	a.f = fa;
	b.f = fb;

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
		return b;
	else if (b.f == 0)
		return a;


	int subEx = a.parts.exponent - b.parts.exponent;
	if (subEx != 0) {//exponents equal
		checknum = 1;
		if (subEx > 0) {// a's exponent > b's exponent  => shift mantissa right
						//b.parts.exponent = a.parts.exponent;
			extbit_cal(b, subEx, ext_bit);
			mantissa_cal(z, a, b, subEx);
		}
		else {// a's exponent < b's exponent => shift mantissa right
			  //a.parts.exponent = b.parts.exponent;
			extbit_cal(a, abs(subEx), ext_bit);
			mantissa_cal(z, b, a, abs(subEx));
		}
	}

	if (a.parts.sign != b.parts.sign) {
		if (a.parts.mantissa > b.parts.mantissa) {
			if (subEx != 0)
				sum = sum_cal(z, a, b, ext_bit);
			else
				sum = sum_cal(z, a, b, NULL);
		}
		else {
			if (subEx != 0)
				sum = sum_cal(z, b, a, ext_bit);
			else
				sum = sum_cal(z, b, a, NULL);
		}
	}
	else {
		sum = a.parts.mantissa + b.parts.mantissa;
		z.parts.sign = a.parts.sign;

		if (subEx == 0)
			z.parts.exponent = a.parts.exponent;
	}

	unsigned int z_mantissa;

	//mantissa + mantissa가 23비트가 넘어가버리면 자동으로 잘라버림! (왜냐면 union이니깐)
	//따라서 우리가 직접 넘어가는 carry값을 처리해줘야한다.
	if (sum > 0x7FFFFF) {
		if (subEx == 0) {

			if (z.parts.exponent != 0) {
				z.parts.mantissa = sum >> 1;
				ext_bit[0] = ext_bit[0] | ext_bit[1]; // sticky bit = sum[1] | sum[0]
				ext_bit[1] = ext_bit[2]; // round bit
				ext_bit[2] = (sum & 1) ? 1 : 0; // guard bit
			}
			else {
				//z.parts.mantissa = sum & 0x3FFFFF;
				z.parts.mantissa = sum;
			}
			z.parts.exponent++;
		}
		else {
			ext_bit[0] = ext_bit[0] | ext_bit[1]; // sticky bit = sum[1] | sum[0]
			ext_bit[1] = ext_bit[2]; // round bit
			ext_bit[2] = (sum & 1) ? 1 : 0; // guard bit

			z.parts.mantissa = (sum >> 1) & 0x3FFFFF;
			z.parts.exponent++;
		}
	}
	else {
		if (subEx == 0) {
			if (a.parts.sign == b.parts.sign) {
				if (z.parts.exponent != 0) {
					ext_bit[0] = ext_bit[1]; // round bit
					ext_bit[1] = ext_bit[2]; // round bit
					ext_bit[2] = (sum & 1) ? 1 : 0; // guard bit
					z.parts.mantissa = sum >> 1;
					z.parts.exponent++;
				}
				else
					z.parts.mantissa = sum;
			}
			else {
				int cnt = 1;
				if (z.parts.exponent != 0) {
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
				else
					z.parts.mantissa = sum;
			}
		}
		else {
			z.parts.mantissa = sum;
		}
	}

	/*normalize*/

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

	// guard && (round bit | sticky | z_m[0])
	if (ext_bit[2] && (ext_bit[1] | ext_bit[0] | (z.parts.mantissa & 1))) {
		z.parts.mantissa++;
		if (z.parts.mantissa >= 0xffffff)
			z.parts.exponent++;
	}

	if (z.parts.mantissa == 0) {
		z.f = 0;
	}

	//   not_real_number;
	return z;
}

unsigned int LOA(unsigned int a, unsigned int b, int mode)
{
	unsigned int m, n, sum;
	int carry;

	m = a & mask;
	n = b & mask;

	sum = m | n;
	carry = (m >> bitnum - 1)& (n >> bitnum - 1);

	if (mode == 0) //sum
		sum += (a - m) + (b - n) + (carry << bitnum);
	else
		sum = (a - m) - (b - n) + (carry << bitnum);
	return sum;
}

unsigned int ETA1(unsigned int a, unsigned int b, int mode)
{
	unsigned int M, N, m, n, inaccuratePart = 0, r, sum = 0;
	int carry;

	int imask = etaimask;

	M = a & mask;
	N = b & mask;

	while (1) {
		m = a & imask;
		n = b & imask;
		r = m | n;
		inaccuratePart += r;
		if (m == imask && n == imask) {
			inaccuratePart += imask - 1;
			break;
		}
		imask /= 2;
		if (imask == 0)
			break;
	}

	if (mode == 0)
		sum = (a - M) + (b - N) + inaccuratePart;
	else
		sum = (a - M) - (b - N) + inaccuratePart;

	return sum;
}

int exp_cal(unsigned int x, unsigned int y)
{
	int expnum = 4, exmask = 0xF0;
	int sub = 0;

	if (x > y)
		sub = (x & exmask) - (y & exmask);
	else
		sub = (y & exmask) - (x & exmask);

	exmask >>= expnum;

	sub += (x & exmask) ^ (y & exmask);

	return sub;
}

unsigned int APP_sum_cal(float_cast& z, float_cast x, float_cast y, int caseNum)
{
	unsigned int sum = 0, temp;
	unsigned int esum = 0;

	if (x.parts.exponent < y.parts.exponent) {
		if (caseNum == 1) //LOA
			sum = LOA((y.parts.mantissa | 0x800000), (x.parts.mantissa), 1);
		else if (caseNum == 2) //ETA1
			sum = ETA1((y.parts.mantissa | 0x800000), (x.parts.mantissa), 1);

		z.parts.sign = y.parts.sign;

		int cnt = 0;
		unsigned int temp;
		temp = sum;
		if (sum <= 0x7FFFFF) {
			for (cnt = 1; temp & 0x400000 ? 0 : 1; cnt++)
				temp <<= 1;

			if (cnt >= 3) {
				sum <<= cnt - 3;
			}
			else {
				sum >>= 3 - cnt;
			}
		}
		sum &= 0x7FFFFF;
		int tempSum = z.parts.exponent - cnt;
		if (tempSum <= 0) {
			z.parts.exponent = 0;
			sum >>= (abs(tempSum) + 1);
			sum |= (0x400000 >> abs(tempSum));
		}
		else
			z.parts.exponent -= cnt;

	}
	else {
		if (caseNum == 1) //LOA
			sum = LOA(x.parts.mantissa, y.parts.mantissa, 1);
		else if (caseNum == 2) //ETA1
			sum = ETA1(x.parts.mantissa, y.parts.mantissa, 1);

		z.parts.sign = x.parts.sign;
	}

	return sum;
}

float_cast AppAdder(float fa, float fb, int caseNum) {

	//먼저 두 값이 real number인지 판단해야한다. (inf, -inf, 0, -0, NAN)
	//0 FF 000000 -> inf, 1 FF 000000 -> -inf, 00000 -> 0, 100000 -> -0

	float_cast a, b;
	a.f = fa;
	b.f = fb;

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
	int subEx_tmp = a.parts.exponent - b.parts.exponent;
	int subEx = abs(subEx_tmp);
	if (subEx != 0) {//exponents equal
		checknum = 1;
		if (a.parts.exponent > b.parts.exponent) {// a's exponent > b's exponent  => shift mantissa right
												  //b.parts.exponent = a.parts.exponent;
			mantissa_cal(z, a, b, subEx);
			z.parts.sign = a.parts.sign;
		}
		else {// a's exponent < b's exponent => shift mantissa right
			  //a.parts.exponent = b.parts.exponent;
			mantissa_cal(z, b, a, subEx);
			z.parts.sign = b.parts.sign;
		}
	}
	else {
		z.parts.exponent = a.parts.exponent;
		if (a.parts.sign == b.parts.sign)
			z.parts.sign = a.parts.sign;
		else {
			if (a.parts.mantissa < b.parts.mantissa)
				z.parts.sign = b.parts.sign;
			else
				z.parts.sign = a.parts.sign;
		}
	}


	switch (caseNum) {
	case 1: //LOA
		if (a.parts.sign == b.parts.sign)
			sum = LOA(a.parts.mantissa, b.parts.mantissa, 0);
		else {
			if (a.parts.mantissa > b.parts.mantissa) {
				if (subEx != 0)
					sum = APP_sum_cal(z, a, b, caseNum);
				else
					sum = LOA(a.parts.mantissa, b.parts.mantissa, 1);
			}
			else {
				if (subEx != 0)
					sum = APP_sum_cal(z, b, a, caseNum);
				else
					sum = LOA(b.parts.mantissa, a.parts.mantissa, 1);;
			}
		}
		break;
	case 2: //ETA1
		if (a.parts.sign == b.parts.sign)
			sum = ETA1(a.parts.mantissa, b.parts.mantissa, 0);
		else {
			if (a.parts.mantissa > b.parts.mantissa) {
				if (subEx != 0)
					sum = APP_sum_cal(z, a, b, caseNum);
				else
					sum = ETA1(a.parts.mantissa, b.parts.mantissa, 1);
			}
			else {
				if (subEx != 0)
					sum = APP_sum_cal(z, b, a, caseNum);
				else
					sum = ETA1(b.parts.mantissa, a.parts.mantissa, 1);
			}
		}
		break;
	}



	//mantissa + mantissa가 23비트가 넘어가버리면 자동으로 잘라버림! (왜냐면 union이니깐)
	//따라서 우리가 직접 넘어가는 carry값을 처리해줘야한다.
	if (sum > 0x7FFFFF) {
		if (subEx == 0) {

			if (z.parts.exponent != 0) {
				z.parts.mantissa = sum >> 1;
			}
			else {
				//z.parts.mantissa = sum & 0x3FFFFF;
				z.parts.mantissa = sum;
			}
			z.parts.exponent++;
		}
		else {
			z.parts.mantissa = (sum >> 1) & 0x3FFFFF;
			z.parts.exponent++;
		}
	}
	else {
		if (subEx == 0) {
			if (a.parts.sign == b.parts.sign) {
				if (z.parts.exponent != 0) {
					z.parts.mantissa = sum >> 1;
					z.parts.exponent++;
				}
				else
					z.parts.mantissa = sum;
			}
			else {
				int cnt = 1;
				if (z.parts.exponent != 0) {
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
				else
					z.parts.mantissa = sum;
			}
		}
		else {
			z.parts.mantissa = sum;
		}
	}


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