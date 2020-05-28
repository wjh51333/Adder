// SimplifiedETA.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <random>
#include "SimplifiedETA.h"

using namespace std;

int SJ(int a, int b, int acc_len) {
	int bitmask = 1, temp = 0;
	int sum = 0;
	//int acc_len = 8;
	int inacc_len = 16 - acc_len;
	int accurate_part, inaccurate_part = 0;
	int carry_in, inaccur_carry = 0;

	//a,b��� inaccurate ���̸�ŭ ���������� shift, ��Ʈ����ũ�� accurate ���̸�ŭ
	//(bitmask<<accurate_len)-1)�ǹ� ~ 1<<3 = 1000 , 1000(2)-1 = 111(2)
	accurate_part = ((a >> inacc_len) & ((bitmask << acc_len) - 1)) + ((b >> inacc_len) & ((bitmask << acc_len) - 1));

	//low part ���
	//low part�� �ֻ��� 2�� ��Ʈ OR ����
	//1<<2 = 100 100-1 = 11���� 11<<inacc_len-2 ~ 11000000
	//temp�� low part �ֻ�����Ʈ����
	inaccurate_part = (a | b) & ((bitmask << inacc_len) - 1);
	temp = ((a | b) >> (inacc_len - 1)) & bitmask;

	//carry ��� 
	carry_in = ((a >> inacc_len - 1) & bitmask) & ((b >> inacc_len - 1) & bitmask);
	accurate_part += carry_in;

	//�ٽ� �����ڸ��� shift
	accurate_part = accurate_part << inacc_len;

	//�ٵ� �츮�� �ѹ� �� ������ �ϴµ�, carry�� �̿��Ұž� �������Ѽ� AND����
	//�ٵ� �� ��찡 (carry_in)�̶� temp�� 1�϶��� ���� ���̶� �޶���.
	if (carry_in & temp == 1) {
		inaccurate_part = inaccurate_part & (~(bitmask << (inacc_len - 1)));
	}

	//��ü �� ���
	sum = accurate_part + inaccurate_part;

	//printf("<<newadder4 sum>>\n"); 
	//show(a); printf("\n"); 
	//show(b); printf("\n");
	//printf("----------------\n");
	//show(sum); printf("\n\n");

	return sum & ((1 << 16) - 1);
}

int UB(int a, int b, int accLen)
{
	int mask = 0;
	int carryAND1 = 0;
	int carryAND2 = 0;
	int resultNOR = 0;
	int sum = 0;

	//inaccurate part
	for (int i = accLen - 1; i >= 0; i--)
	{
		int aa = a & (1 << i); //1���� �˻�
		int bb = b & (1 << i); //1���� �˻�

		if (i == accLen - 1) // carry ���ϱ�
		{
			if (aa && bb) // and
			{
				carryAND1 |= 1 << (i + 1);
			}

			if (!(aa || bb)) // Nor
			{
				resultNOR |= 1 << i;
			}

			int cA = carryAND1 & (1 << (i + 1)); //1���� �˻�
			int rN = resultNOR & (1 << i); //1���� �˻�

			if (!(cA || rN))
			{
				mask |= 1 << i;
			}
		}
		else
		{
			if (aa || bb) //or
			{
				if (aa && bb)
				{
					carryAND2 |= 1 << (i + 1); //carry2 ���ϱ�
				}

				mask |= 1 << i;
			}
		}
	}

	//accurate part
	sum = (a >> accLen) + (b >> accLen) + (carryAND1 >> accLen);
	sum = (sum << accLen);

	sum = sum + mask + carryAND2;

	/*
	show(&a);
	cout << "\n";

	show(&b);
	cout << "\n";

	show(&sum);
	cout << "\n";
	*/
	return sum;

}

int m_additionJWETA(int a, int b, int accLen)
{
	const int c_accLen = accLen;
	const int c_inaccLen = c_TADD_BW - c_accLen;

	int	accuratePart = 0;
	int	inaccuratePart = 0;
	int	carry = 0;

	carry = ((a >> (c_inaccLen - 1)) &
		(b >> (c_inaccLen - 1))) & BITMASK(1);
	accuratePart = (((a >> c_inaccLen) & BITMASK(c_accLen)) +
		((b >> c_inaccLen) & BITMASK(c_accLen)) + carry) & BITMASK(c_accLen);
	accuratePart <<= c_inaccLen;

	inaccuratePart = (a ^ b) & BITMASK(c_inaccLen);

	if ( !( (a & (1 << (c_inaccLen - 2))) & (b & (1 << (c_inaccLen - 2))) ) )
	{
		inaccuratePart &= ~(1 << (c_inaccLen - 2));
	}
	else inaccuratePart |= (1 << (c_inaccLen - 2));

	if ((a & (1 << (c_inaccLen - 2))) | (b & (1 << (c_inaccLen - 2))))
	{
		inaccuratePart |= BITMASK(c_inaccLen - 2);
	}
	else inaccuratePart &= ((inaccuratePart & (1 << (c_inaccLen-1))) | (inaccuratePart & (1 << (c_inaccLen - 2))));

	return	(accuratePart | inaccuratePart) & BITMASK(c_TADD_BW);
}

int m_additionETA1(int a, int b, int accLen)
{
	const int c_accLen = accLen;
	const int c_inaccLen = c_TADD_BW - c_accLen;

	int	accuratePart = 0;
	int	inaccuratePart = 0;

	accuratePart = (((a >> c_inaccLen) & BITMASK(c_accLen)) +
		((b >> c_inaccLen) & BITMASK(c_accLen))) & BITMASK(c_accLen);
	accuratePart <<= c_inaccLen;

	inaccuratePart = (a | b) & BITMASK(c_inaccLen);

	int	bitmask = 0;
	for (int i = c_inaccLen - 1; i >= 0; --i)
		if ((a & 1 << i) && (b & 1 << i))
		{
			bitmask = BITMASK(i + 1);
			break;
		}
	inaccuratePart |= bitmask;

	return	(accuratePart | inaccuratePart) & BITMASK(c_TADD_BW);
}

int m_additionLOA(int a, int b, int accLen)
{
	const int c_accLen = accLen;
	const int c_inaccLen = c_TADD_BW - c_accLen;

	int	accuratePart = 0;
	int	inaccuratePart = 0;
	int	carry = 0;

	carry = ((a >> (c_inaccLen - 1)) &
		(b >> (c_inaccLen - 1))) & BITMASK(1);
	accuratePart = (((a >> c_inaccLen) & BITMASK(c_accLen)) +
		((b >> c_inaccLen) & BITMASK(c_accLen)) + carry) & BITMASK(c_accLen);
	accuratePart <<= c_inaccLen;

	inaccuratePart = (a | b) & BITMASK(c_inaccLen);

	return	(accuratePart | inaccuratePart) & BITMASK(c_TADD_BW);
}

int m_addition(int a, int b, int l)
{
	//return	m_additionSJ(a, b, l);
	//return	m_additionUB(a, b, l);
	//return	m_additionJWETA(a, b, l);
	//return	m_additionETA1(a, b, l);
	//return	m_additionLOA(a, b, l);

}

int main()
{
	std::cout << "Hello World!\n";

	default_random_engine generator;
	uniform_int_distribution<int> distribution(0, 65535);

	//for (int l = 4; l <= 12; l++)
	for (int l = 8; l <= 8; l++)
	{
		int error_count = 0;
		int error_count_ovf = 0;
		const int c_total_rand = 10000000;
		int error_dist = 0;
		double mean_error_dist = 0.0;
		double mean_rel_error_dist = 0.0;
		int num_large_red = 0;
		int maxED = 0;
		double maxRED = 0.0;


		for (int i = 0; i < c_total_rand; i++)
		{
			int a = distribution(generator) & BITMASK(c_TADD_BW);
			int b = distribution(generator) & BITMASK(c_TADD_BW);

			int accurate = (a + b) & BITMASK(c_TADD_BW);
			int approximate = m_addition(a, b, l);

			if (accurate != approximate)
			{
				error_count++;
				error_dist = abs(approximate - accurate);// &BITMASK(c_TADD_BW);
				mean_error_dist += error_dist;

				if (maxED < error_dist)
					maxED = error_dist;

				if (accurate)
				{
					double	rel_err_dist = (double)error_dist / (double)accurate;
					mean_rel_error_dist += rel_err_dist;
					if (rel_err_dist > maxRED)
					{
						maxRED = rel_err_dist;
					}
				}
			}
		}
		mean_error_dist /= c_total_rand;
		mean_rel_error_dist /= c_total_rand;

		cout << "Error Count                    = " << error_count << " / " << c_total_rand << endl;
		cout << "Error Rate (ER)                = " << (double)error_count / (double)c_total_rand * 100 << endl;
		cout << "Mean Error Distance (MED)      = " << mean_error_dist << endl;
		cout << "Mean Rel Error Distance (MRED) = " << mean_rel_error_dist << endl;
		cout << "Normalized MED (NMED)          = " << mean_error_dist / (double)maxED << endl;
		cout << "Max Error Distance             = " << maxED << endl;
		cout << "Max Rel Error Distance         = " << maxRED << endl;
		cout << endl;
	}
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file