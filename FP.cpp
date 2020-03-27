#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <random>
#include <iostream>
#include <time.h>
//#include <values.h>

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

//std::uniform_int_distribution<int>  DRandomExponent(0, 0x7FF);
//std::uniform_int_distribution<int>  DRandomMantisa(0, 0x7FFFFFFFFFFFF);
//std::uniform_int_distribution<int>  DRandomSign(0, 1);



/*
void
print_float_bits(float f)
{
int i;

i = FLOATBITS - 1;
PUT_BIT(f, i);
putchar(' ');
for (i--; i >= 23; i--) {
PUT_BIT(f, i);
}
putchar(' ');
for (; i >= 0; i--) {
PUT_BIT(f, i);
}
}

int
main(int argc, char** argv)
{
float f;

while (scanf("%f", &f) == 1) {
printf("%10g = %24.17g = ", f, f);
print_float_bits(f);
putchar('\n');
}

return 0;
}
*/

// ��Ʈ�����ϴ°� 
/*void getSME(int*, int*, int*, float);

void getSME(int *s, int *m, int *e, float number)
{
unsigned int* ptr = (unsigned int*)& number;

*s = *ptr >> 31;
*e = *ptr & 0x7f800000;
*e >>= 23;
*m = *ptr & 0x007fffff;
}

void main() {
printf("%a", (float)0.75);
int Exponent, Fraction, Sign;
float num = 0.75;

//Exponent = num & E;
//Fraction = num & F;
//Sign = num >> 31 & 1;
}*/

//Union ���
typedef union {
	float f;
	struct {
		unsigned int mantisa : 23;
		unsigned int exponent : 8;
		unsigned int sign : 1;
	} parts;
} float_cast;

//typedef union {
//	double f;
//	struct {
//		unsigned int mantisa : 52;
//		unsigned int exponent : 11;
//		unsigned int sign : 1;
//	} parts;
//} double_cast;

float_cast makeFP();
//double_cast makeDFP();
float_cast FPAdder(float_cast a, float_cast b);
void FPAddition();


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
//double_cast makeDFP() {
//	//exponet => 127(7F) ~ -128(80)
//	//mantisa => 524287(7FFFF) ~ 0(0)
//	double_cast num;
//	num.parts.exponent = DRandomExponent(generator); //0~255 -> 2^(exponent - 126) * 1.(mantisa)
//	//if exponent == 255 -> inf
//	num.parts.mantisa = DRandomMantisa(generator);
//	num.parts.sign = 0;	
//	return num;
//}

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
		z.parts.exponent = a.parts.exponent;
		z.parts.mantisa = a.parts.mantisa + b.parts.mantisa;

		if (z.parts.mantisa == 0)
			z.f = 0;
		else {
			if (1) {//is it overflow?

			}
			else {

			}
			z.parts.mantisa &= 0x7FFFF;
		}
	}
	else { //shift smaller one to bigger one
		if (subEx > 0) {// a's exponent > b's exponent  => shift mantisa right
						//b.parts.exponent = a.parts.exponent;
			z.parts.exponent = a.parts.exponent;
			if (abs(subEx) >= 23)	//shift �� mantisa�� 23��Ʈ �Ѿ�� 0���� �ʱ�ȭ!
				b.parts.mantisa = 0;
			else
				b.parts.mantisa >>= abs(subEx);
		}
		else {// a's exponent < b's exponent => shift mantisa right
			  //a.parts.exponent = b.parts.exponent;
			z.parts.exponent = b.parts.exponent;
			if (abs(subEx) >= 23)//shift �� mantisa�� 23��Ʈ �Ѿ�� 0���� �ʱ�ȭ!
				a.parts.mantisa = 0;
			else
				a.parts.mantisa >>= abs(subEx);
		}

		sum = a.parts.mantisa + b.parts.mantisa;
		//mantisa + matisa�� 23��Ʈ�� �Ѿ������ �ڵ����� �߶����! (�ֳĸ� union�̴ϱ�)
		//���� �츮�� ���� �Ѿ�� carry���� ó��������Ѵ�.
		if (sum > 0x7FFFFF) {
			z.parts.mantisa = (sum >> 1);
			z.parts.exponent++;
		}
		else
			z.parts.mantisa = sum;

		if (z.parts.mantisa == 0)
			z.f = 0;
	}
	return z;
}

void FPAddition() {
	float_cast a, b, ans, d1;
	a = makeFP();
	b = makeFP();
	printf("--------------A---------------\n");
	printf("sign = %x\n", a.parts.sign);
	printf("exponent = %x\n", a.parts.exponent);
	printf("mantisa = %x\n", a.parts.mantisa);
	printf("FP = %e\n", a.f);
	printf("FP = %lf\n", a.f);

	printf("--------------B---------------\n");
	printf("sign = %x\n", b.parts.sign);
	printf("exponent = %x\n", b.parts.exponent);
	printf("mantisa = %x\n", b.parts.mantisa);
	printf("answer = %e\n", b.f);
	printf("FP = %lf\n", b.f);


	ans = FPAdder(a, b);

	printf("--------------Ans---------------\n");
	printf("sign = %x\n", ans.parts.sign);
	printf("exponent = %x\n", ans.parts.exponent);
	printf("mantisa = %x\n", ans.parts.mantisa);
	printf("answer = %e\n", ans.f);
	printf("FP = %lf\n", ans.f);

}
void compare() {
	float_cast A, B, ans;
	float_cast orgAns;
	int cnt = 0;
	printf("A\t\t+\t\tB\t=\torgANS\t\tmyANS\n");
	printf("**********************************************************************\n");
	//for (int i = 0; i < 1000; i++) {
	A = makeFP();
	B = makeFP();
	//A.f = 1.196897e+21;
	//B.f = 6.368948e+23;

	//A.f = 7.899285e-20;
	//B.f = 8.390388e-22;
	A.f = 2.494615e+09;
	B.f = 1.094270e+04;
	//A.f = 1.626195e+35;
	//B.f = 1.484308e+34;
	ans = FPAdder(A, B);
	orgAns.f = A.f + B.f;
	if (orgAns.f != ans.f) {
		printf("%e    +    %e    =    %e,   %e\n", A.f, B.f, orgAns.f, ans.f);
		cnt++;
	}
	//}
	printf("\n\n*************%d", cnt);

}



int main(void) {
	float_cast d1, d2, ans;// = { .f = 0.75 };
						   //d1.parts.sign = 0;
						   //d1.parts.exponent = 0x47f;
						   //d1.parts.mantisa = 0x731c8;
						   //d1.parts.sign = 1;
						   //d1.parts.exponent = 0x7e;
						   //d1.parts.mantisa = 0x0;
						   //d1.f = -0.5;
						   ////d2.f = -0.4375;
						   ////exponet => 127(7F) ~ -128(80)
						   ////mantisa => 524287(7FFFF) ~ 0(0)
						   //printf("sign = %f\n", d1.f);

						   //printf("sign = %x\n", d1.parts.sign);
						   //printf("exponent = %x\n", d1.parts.exponent);
						   //printf("mantisa = %x\n", d1.parts.mantisa);


						   /*for (int i = 0; i < 100; i++) {
						   float_cast num = makeFP();
						   printf("%d %d %d\n", num.parts.exponent, num.parts.mantisa, num.parts.sign);
						   }*/

						   //FPAddition();
	compare();

	//RandomGen();
	//ans.f = FPAdder(d1, d2);
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