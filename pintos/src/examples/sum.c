#include<stdio.h>
#include<syscall.h>

int atoi(char *);

int main(int argc, char **argv) {
	int i;
	int arr[4];
	for(i=0;i<argc-1;i++) {
		arr[i] = atoi(argv[i+1]);
	}
	printf("%d %d\n",pibonacci(arr[0]), sum_of_four_integers(arr[0],arr[1],arr[2],arr[3]));

	return 0;
}

int atoi(char *buf) {
	int i=0;
	int sum = 0;
	while(buf[i]) {
		sum *= 10;
		if(buf[i] >= '0' && buf[i] <= '9')
			sum += (buf[i] - '0');
		i++;
	}

	return sum;
}
