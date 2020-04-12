#include <iostream>

using namespace std;

int mask;
int a;
int b;

void show(int* a)
{
	for (int i = 8; i > 0; i--)
	{
		cout << ((*a & (1 << i - 1) ? 1 : 0));
		if (i % 5 == 0)
			cout << " ";
	}
}

int main()
{
	int mask = 0;
	int mask2 = 0;
	int carry = 0;
	int i;
	int n1 = 124; //숫자 1
	int n2 = 47; //숫자 2
	int loasum;
	int etasum;
	
	cout << "LOA sum";
	cout << "\n";
	show(&n1);
	cout << "\n";
	show(&n2);
	cout << "\n";
	cout << "----------";
	cout << "\n";

	//inaccurate part
	for (i = 3; i >= 0; i--)
	{
		a = n1 & (1 << i); //1인지 검사
		b = n2 & (1 << i); //1인지 검사

		if (a||b)
		{
			if (a && b)
			{
				if (i == 3) //carry구할거양
				{
					carry |= 1 << (i + 1);
				}
			}
			
			mask |= 1 << i;
			
		}
		
	}

	//accurate part
	loasum = (n1 >> 4) + (n2 >> 4) + (carry >> 4);
	loasum = (loasum << 4);

	loasum = loasum + mask;
	show(&loasum);

	//////////////////////////////////////////////
	
	cout << "\n\n";
	cout << "ETA1 sum";
	cout << "\n";
	show(&n1);
	cout << "\n";
	show(&n2);
	cout << "\n";
	cout << "----------";
	cout << "\n";

	//inaccurate part
	for (i = 3; i >= 0; i--)
	{
		a = n1 & (1 << i); //1인지 검사
		b = n2 & (1 << i); //1인지 검사

		if (a || b)
		{
			if (a && b) //a와 b가 둘다 1이 되면
			{
				while (i >= 0)
				{
					mask2 |= 1 << i;
					i--;
					
				}
			}
			else //xor
			{
				mask2 |= 1 << i;
			}

		}

	}

	//accurate part
	etasum = (n1 >> 4) + (n2 >> 4);
	etasum = (etasum << 4);

	etasum = etasum + mask2;
	show(&etasum);


	//////////////////////////////////////
	cout << "\n\n";
	cout << "accurate sum : ";
	cout << n1 + n2;
	cout << "\n";
	cout << "approximate sum(LOA) : ";
	cout << loasum;
	cout << "\n";
	cout << "approximate sum(ETA1) : ";
	cout << etasum;
}
