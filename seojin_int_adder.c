//low part adder

#include <stdio.h>
int LOA(int a, int b);
int ETA1(int a, int b);
int LOAWA(int a, int b);
int OLOCA(int a, int b);
int HOERAA(int a, int b);
void show(int input);

int main(){
	int a = 500; 
	int b = 800;
	int LOA_sum,ETA_sum, LOAWA_sum, OLOCA_sum, HOERAA_sum = 0;

	LOA_sum = LOA(a,b);
	ETA_sum = ETA1(a,b);
	LOAWA_sum = LOAWA(a,b);
	OLOCA_sum = OLOCA(a,b);
	HOERAA_sum = HOERAA(a,b);
	//printf("----------------------------------------\n");
	printf("accurate_sum : %d\n",a+b); 
	printf("approximate_sum(LOA) : %d\n", LOA_sum); //
	printf("approximate_sum(ETA1) : %d\n", ETA_sum); //
	printf("approximate_sum(LOAWA) : %d\n", LOAWA_sum);
	printf("approximate_sum(OLOCA) : %d\n", OLOCA_sum);
	printf("approximate_sum(HOERAA) : %d\n", HOERAA_sum);
	return 0;
}

int LOA(int a, int b){
	int bitmask=1;
	int sum = 0;
	int accurate_len = 6;
	int inaccurate_len = 12-accurate_len;
	int accurate_part,inaccurate_part = 0;
	int carry_in=0;
	
	//a,b모두 inaccurate 길이만큼 오른쪽으로 shift, 비트마스크는 accurate 길이만큼
	//(bitmask<<accurate_len)-1)의미 ~ 1<<3 = 1000 , 1000(2)-1 = 111(2)
	accurate_part = ((a>>inaccurate_len)&((bitmask<<accurate_len)-1)) + ((b>>inaccurate_len)&((bitmask<<accurate_len)-1));

	//carry 계산 (low part에서 제일 상위비트 And 연산)
	carry_in = ((a>>inaccurate_len-1)&bitmask) & ((b>>inaccurate_len-1)&bitmask);
	accurate_part += carry_in;

	//다시 원래자리로 shift
	accurate_part = accurate_part<<inaccurate_len;

	//low part 계산 = OR연산, bitmask로 lowpart만 연산되도록!
	inaccurate_part = (a|b)&((bitmask<<inaccurate_len)-1);


	//전체 합 계산
	sum = accurate_part + inaccurate_part;

	printf("<<LOA sum>>\n"); 
	show(a); printf("\n"); 
	show(b); printf("\n");
	printf("-----------\n");
	show(sum); printf("\n\n");
	

	return sum;
}

int ETA1(int a, int b){
	int bitmask=1;
	int sum = 0;
	int accurate_len = 6;
	int inaccurate_len = 12-accurate_len;
	int accurate_part,inaccurate_part = 0;
	int i=0;
	
	//a,b모두 inaccurate 길이만큼 오른쪽으로 shift, 비트마스크는 accurate 길이만큼
	//(bitmask<<accurate_len)-1)의미 ~ 1<<3 = 1000 , 1000(2)-1 = 111(2)
	accurate_part = ((a>>inaccurate_len)&((bitmask<<accurate_len)-1)) + ((b>>inaccurate_len)&((bitmask<<accurate_len)-1));

	//다시 원래자리로 shift
	accurate_part = accurate_part<<inaccurate_len;

	//low part 계산
	for(i=inaccurate_len-1;i>0;i--){
		if((a&(bitmask<<i)) && (b&(bitmask<<i))){
			//두 비트가 모두 1인 경우 하위비트 모두 1로 바꾸고 break;
			inaccurate_part= 0xff&((bitmask<<(i+1))-1); 
			break;
		}
		//else인 경우
		//low part 계산 = XOR연산, bitmask로 lowpart만 연산되도록!
		inaccurate_part = (a&(bitmask<<i))^(b&&(bitmask<<i));
	}

	//전체 합 계산
	sum = accurate_part + inaccurate_part;

	printf("<<ETA1 sum>>\n"); 
	show(a); printf("\n"); 
	show(b); printf("\n");
	printf("-----------\n");
	show(sum); printf("\n\n");

	return sum;
}

int LOAWA(int a, int b){
	//LOA에서 carry input이 없는 상태
	int bitmask=1;
	int sum = 0;
	int accurate_len = 6;
	int inaccurate_len = 12-accurate_len;
	int accurate_part,inaccurate_part = 0;
	int carry_in=0;
	
	//a,b모두 inaccurate 길이만큼 오른쪽으로 shift, 비트마스크는 accurate 길이만큼
	//(bitmask<<accurate_len)-1)의미 ~ 1<<3 = 1000 , 1000(2)-1 = 111(2)
	accurate_part = ((a>>inaccurate_len)&((bitmask<<accurate_len)-1)) + ((b>>inaccurate_len)&((bitmask<<accurate_len)-1));

	//다시 원래자리로 shift
	accurate_part = accurate_part<<inaccurate_len;

	//low part 계산 = OR연산, bitmask로 lowpart만 연산되도록!
	inaccurate_part = (a|b)&((bitmask<<inaccurate_len)-1);

	//전체 합 계산
	sum = accurate_part + inaccurate_part;

	printf("<<LOAWA sum>>\n"); 
	show(a); printf("\n"); 
	show(b); printf("\n");
	printf("-----------\n");
	show(sum); printf("\n\n");
	
	return sum;
}

int OLOCA(int a, int b){
	//LOA인데 lower-part부분이 다름! 
	//lower part의 상위비트 2개만 LOA식 연산, 나머지는 1로 고정
	int bitmask=1;
	int sum = 0;
	int accurate_len = 6;
	int inaccurate_len = 12-accurate_len;
	int accurate_part,inaccurate_part = 0;
	int carry_in=0;
	
	//a,b모두 inaccurate 길이만큼 오른쪽으로 shift, 비트마스크는 accurate 길이만큼
	//(bitmask<<accurate_len)-1)의미 ~ 1<<3 = 1000 , 1000(2)-1 = 111(2)
	accurate_part = ((a>>inaccurate_len)&((bitmask<<accurate_len)-1)) + ((b>>inaccurate_len)&((bitmask<<accurate_len)-1));

	//carry 계산 (low part에서 제일 상위비트 And 연산)
	carry_in = ((a>>inaccurate_len-1)&bitmask) & ((b>>inaccurate_len-1)&bitmask);
	accurate_part += carry_in;

	//다시 원래자리로 shift
	accurate_part = accurate_part<<inaccurate_len;

	//low part 계산 = OR연산, bitmask로 lowpart만 연산되도록!
	inaccurate_part = (a|b)&((bitmask<<inaccurate_len)-1);

	//inaccurate_len-2인 하위비트는 1
	inaccurate_part= inaccurate_part|(0xff&(bitmask<<(inaccurate_len-2))-1);

	//전체 합 계산
	sum = accurate_part + inaccurate_part;

	printf("<<OLOCA sum>>\n"); 
	show(a); printf("\n"); 
	show(b); printf("\n");
	printf("-----------\n");
	show(sum); printf("\n\n");
	
	return sum;

}

int HOERAA(int a, int b){
	//OLOCA와 유사
	//lower part의 상위비트 2개만 새로운 연산, 나머지는 1로 고정
	int bitmask=1;
	int sum = 0;
	int accurate_len = 6;
	int inaccurate_len = 12-accurate_len;
	int accurate_part,inaccurate_part = 0;
	int carry_in=0;
	
	//a,b모두 inaccurate 길이만큼 오른쪽으로 shift, 비트마스크는 accurate 길이만큼
	//(bitmask<<accurate_len)-1)의미 ~ 1<<3 = 1000 , 1000(2)-1 = 111(2)
	accurate_part = ((a>>inaccurate_len)&((bitmask<<accurate_len)-1)) + ((b>>inaccurate_len)&((bitmask<<accurate_len)-1));

	//carry 계산 (low part에서 제일 상위비트 And 연산)
	carry_in = ((a>>inaccurate_len-1)&bitmask) & ((b>>inaccurate_len-1)&bitmask);
	accurate_part += carry_in;

	//다시 원래자리로 shift
	accurate_part = accurate_part<<inaccurate_len;

	//low part 계산
	if((a&(bitmask<<(inaccurate_len-1))) && (b&(bitmask<<(inaccurate_len-1)))){
		//(n-k-1)번째 input이 둘 다 1일때
		//(n-k-1)번째 output = (n-k-2)번째 input의 and 연산
		//(n-k-2)번째는 그대로 OR 연산
		inaccurate_part = (a&b)&((bitmask<<inaccurate_len-1));
		inaccurate_part = inaccurate_part + (a|b)&((bitmask<<inaccurate_len-2));
	}
	else{
		//그냥 LOA연산(OR)
		inaccurate_part = (a|b)&((bitmask<<inaccurate_len)-1);
	}
	
	//inaccurate_len-2인 하위비트는 1
	inaccurate_part= inaccurate_part|(0xff&(bitmask<<(inaccurate_len-2))-1);

	//전체 합 계산
	sum = accurate_part + inaccurate_part;

	printf("<<HOERAA sum>>\n"); 
	show(a); printf("\n"); 
	show(b); printf("\n");
	printf("-----------\n");
	show(sum); printf("\n\n");
	

	return sum;
}

void show(int input){
	int i=0;
	int mask = 1;

	if(input == 0){
		return;
	}

	for(i=11; i>=0; i--){
		mask = 1 << i;
		printf("%d",(input&mask)?1:0);

		if(i%4 == 0)
			printf(" ");
	}
}
