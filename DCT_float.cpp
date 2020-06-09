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

#define   BITMASK(bitWidth)   ( (unsigned int)(-1) >> (sizeof(unsigned int)*8 - (bitWidth)) )
#define GETSIGN(a)         ( ((a) >> (c_TADD_BW-1)) & BITMASK(1) )
#define EXTSIGN(a)         ( GETSIGN(a) ? ((a) & BITMASK(c_TADD_BW)) | (-1 << c_TADD_BW) : (a) )
#define GETBIT(a, n)      ( ((a) >> n) & BITMASK(1) )

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

void extbit_cal(float_cast x, int subEx, int *e);
void mantissa_cal(float_cast &z, float_cast &x, float_cast &y, int subEx);
unsigned int sum_cal(float_cast &z, float_cast x, float_cast y, int *e);

float_cast FPAdder(float fa, float fb);
unsigned int LOA(unsigned int a, unsigned int b);
unsigned int ETA1(unsigned int a, unsigned int b);
float_cast AppAdder(float fa, float fb, int caseNum);
int exp_cal(unsigned int x, unsigned int y);

/////////////////////////////////////////////////////////////////////////////////////////

unsigned char Org[SIZE][SIZE];
float Temp[SIZE][SIZE];
unsigned char New[SIZE][SIZE];
int output[SIZE][SIZE];
unsigned char output2[SIZE][SIZE] = { 1, };
unsigned char output3[SIZE][SIZE] = { 1, };
int output4[SIZE*SIZE] = { 1, };

int cnt = 0;
//DCT 수행
void DCT_1(float DCT[8][8], float DCT_T[8][8]) {
	printf("DCT_1\n");
	int i, j, m, n, p;
	float result;

	for (i = 0; i < 64; i++) //전체 이미지를 8byte씩 32개로 나눈 블록의 row
		for (j = 0; j < 64; j++) //전체 이미지를 8바이트씩 32개로 나눈 블록의 col
		{
			//OriginalxDCT_tranpose
			for (m = 0; m < 8; m++)
			{
				for (n = 0; n < 8; n++)
				{
					result = 0.0;
					for (p = 0; p < 8; p++)
					{
						//result += ((int)Org[(i * 8) + m][(j * 8) + p] - 128) * DCT_T[p][n];
						//result = FPAdder(result, ((int)Org[(i * 8) + m][(j * 8) + p] - 128) * DCT_T[p][n]).f;
						result = AppAdder(result, ((int)Org[(i * 8) + m][(j * 8) + p] - 128) * DCT_T[p][n], 1).f;
						cnt++;
					}
					Temp[(i * 8) + m][(j * 8) + n] = result;
				}
			}
			//DCTx(OriginalxDCT_transpose)
			for (m = 0; m < 8; m++) //DCT row
			{
				for (n = 0; n < 8; n++) //블록 안의 col
				{
					result = 0.0;
					for (p = 0; p < 8; p++) //DCT col & 블록 안의 row
					{
						result += DCT[m][p] * Temp[(i * 8) + p][(j * 8) + n];
					}
					output[(i * 8) + m][(j * 8) + n] = (int)(result);
				}
			}
		}

	printf("output matrix\n");
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			printf("%d ", output[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

//inverse DCT
void DCT_2(float DCT[8][8], float DCT_T[8][8]) {
	printf("DCT_2\n");
	int i, j, m, n, p;
	float result;

	//inverse DCT
	for (i = 0; i < 64; i++) //전체 이미지를 8byte씩 32개로 나눈 블록의 row
		for (j = 0; j < 64; j++) //전체 이미지를 8바이트씩 32개로 나눈 블록의 col
		{
			//outputxDCT
			for (m = 0; m < 8; m++)
			{
				for (n = 0; n < 8; n++)
				{
					result = 0.0;
					for (p = 0; p < 8; p++)
					{
						//result += output[(i * 8) + m][(j * 8) + p] * DCT[p][n];
						//result = FPAdder(result, output[(i * 8) + m][(j * 8) + p] * DCT[p][n]).f;
						result = AppAdder(result, output[(i * 8) + m][(j * 8) + p] * DCT[p][n], 1).f;
						cnt++;
					}
					Temp[(i * 8) + m][(j * 8) + n] = result;
				}
			}
			//DCT_Tx(outputxDCT)
			for (m = 0; m < 8; m++) //DCT row
			{
				for (n = 0; n < 8; n++) //블록 안의 col
				{
					result = 0.0;
					for (p = 0; p < 8; p++) //DCT col & 블록 안의 row
					{
						//result += DCT_T[m][p] * Temp[(i * 8) + p][(j * 8) + n];
						//result = FPAdder(result, DCT_T[m][p] * Temp[(i * 8) + p][(j * 8) + n]).f;
						result = AppAdder(result, DCT_T[m][p] * Temp[(i * 8) + p][(j * 8) + n], 1).f;
						cnt++;
					}
					result += 128.0;
					output2[(i * 8) + m][(j * 8) + n] = (unsigned char)(result + 0.5);
				}
			}
		}
}

//가우시안 스무딩
void smoothing() {
	int height = SIZE - 1;
	int width = SIZE - 1;
	int i, j, vmax, vmin;

	int MaskGaussian[3][3] = { { 1,2,1 },
	{ 2,4,2 },
	{ 1,2,1 } };
	int mr, mc;
	int newValue;

	for (i = 1; i < height; i++)
	{
		for (j = 1; j < width; j++)
		{
			newValue = 0; //0으로 초기화
			for (mr = 0; mr < 3; mr++)
				for (mc = 0; mc < 3; mc++)
					newValue += (MaskGaussian[mr][mc] * output2[i + mr - 1][j + mc - 1]);
			newValue /= 20; //마스크의 합의 크기로 나누기: 값의 범위를 0에서 255로
			output3[i][j] = (unsigned char)newValue;
		}
	}

	FILE *outfile = fopen("양자화 후 필터링.raw", "wb");
	fwrite(output3, sizeof(unsigned char), SIZE * SIZE, outfile);
	fclose(outfile);
}

void median() {
}

void main() {
	int i, j;
	float DCT_T[8][8];
	float result = 0.0;
	float MSE, SUM, PSNR;

	//DTC Matrix define
	float DCT[8][8] = {
		1 / sqrt(2.0), 1 / sqrt(2.0), 1 / sqrt(2.0), 1 / sqrt(2.0), 1 / sqrt(2.0), 1 / sqrt(2.0), 1 / sqrt(2.0), 1 / sqrt(2.0),
		cos(M_PI / 16), cos(3 * M_PI / 16), cos(5 * M_PI / 16), cos(7 * M_PI / 16), cos(9 * M_PI / 16), cos(11 * M_PI / 16), cos(13 * M_PI / 16), cos(15 * M_PI / 16),
		cos(2 * M_PI / 16), cos(6 * M_PI / 16), cos(10 * M_PI / 16), cos(14 * M_PI / 16), cos(18 * M_PI / 16), cos(22 * M_PI / 16), cos(26 * M_PI / 16), cos(30 * M_PI / 16),
		cos(3 * M_PI / 16), cos(9 * M_PI / 16), cos(15 * M_PI / 16), cos(21 * M_PI / 16), cos(27 * M_PI / 16), cos(33 * M_PI / 16), cos(39 * M_PI / 16), cos(45 * M_PI / 16),
		cos(4 * M_PI / 16), cos(12 * M_PI / 16), cos(20 * M_PI / 16), cos(28 * M_PI / 16), cos(36 * M_PI / 16), cos(44 * M_PI / 16), cos(52 * M_PI / 16), cos(60 * M_PI / 16),
		cos(5 * M_PI / 16), cos(15 * M_PI / 16), cos(25 * M_PI / 16), cos(35 * M_PI / 16), cos(45 * M_PI / 16), cos(55 * M_PI / 16), cos(65 * M_PI / 16), cos(75 * M_PI / 16),
		cos(6 * M_PI / 16), cos(18 * M_PI / 16), cos(30 * M_PI / 16), cos(42 * M_PI / 16), cos(54 * M_PI / 16), cos(66 * M_PI / 16), cos(78 * M_PI / 16), cos(90 * M_PI / 16),
		cos(7 * M_PI / 16), cos(21 * M_PI / 16), cos(35 * M_PI / 16), cos(49 * M_PI / 16), cos(63 * M_PI / 16), cos(77 * M_PI / 16), cos(91 * M_PI / 16), cos(105 * M_PI / 16)
	};
	//위의 matrix 2로 나눠야 완성
	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			DCT[i][j] = DCT[i][j] / 2;

	//define DCT transpose
	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			DCT_T[j][i] = DCT[i][j];

	//raw image 읽어오기
	FILE *pf = fopen("lena(512x512).raw", "rb");
	if (pf == NULL) { printf("File open error!!"); return; }
	fread(Org, sizeof(unsigned char), SIZE * SIZE, pf);
	fclose(pf);

	//테스트 출력
	printf("raw image\n");
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			printf("%u ", Org[i][j]);
		}
		printf("\n");
	}
	printf("\n");


	DCT_1(DCT, DCT_T);
	DCT_2(DCT, DCT_T);


	//테스트 출력
	printf("inverse DCT matrix\n");
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			printf("%u ", output2[i][j]);
		}
		printf("\n");
	}
	printf("\n");

	MSE = 0.0;
	SUM = 0.0;

	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		{
			SUM += ((int)Org[i][j] - (int)output2[i][j])*((int)Org[i][j] - (int)output2[i][j]);
		}
	}
	MSE = SUM / (SIZE*SIZE);
	PSNR = 10.0*log10((255.0*255.0) / MSE);
	printf("PSNR : %f\n\n", PSNR);
	printf("cnt : %d\n", cnt);

	//org: 52.430256
	//myorg: 52.429989 (3개)
	//LOA : 26.894617 (4개)  29.260611(3개)
	//ETA1 : 26.873035 (4개) 29.235916 (3개)


	FILE *outfile = fopen("LOA_3.raw", "wb");
	fwrite(output2, sizeof(unsigned char), SIZE * SIZE, outfile);
	fclose(outfile);

	//   smoothing();

	printf("complete!\n");
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

void extbit_cal(float_cast x, int subEx, int *e)
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

unsigned int sum_cal(float_cast &z, float_cast x, float_cast y, int *e)
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
		z.f = b.f;
	else if (b.f == 0)
		z.f = a.f;


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
	carry = (m >> bitnum - 1) & (n >> bitnum - 1);

	if (mode == 0) //sum
		sum += (a - m) + (b - n) + (carry << bitnum);
	else
		sum = (a - m) - (b - n) + (carry << bitnum);
	return sum;
}

unsigned int ETA1(unsigned int a, unsigned int b, int mode)
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

unsigned int APP_sum_cal(float_cast &z, float_cast x, float_cast y, int caseNum)
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