
#include <inc/lib.h>

#define N 3

int A[N][N] = { 1, 2, 3,
				4, 5, 6,
				7, 8, 9};

int IN[N][N] = { 9, 8, 7
				 6, 5, 4,
				 3, 2, 1};

int 
multiplication(int index)
{

}

void
umain(int argc, char **argv)
{
	int i, j;
	envid_t child, multiply[N];

	for (i = 0; i < N; i++) {
		if ((multiply[i] = fork()) < 0) {
			panic("fork: %e", multiply[i]);
		}
		if (multiply[i] == 0) {
			multiplication(i);
		}
	}

	for (i = 0; i < N; i++) {
		if ((child = fork()) < 0) {
			panic("fork: %e", child);
		}
		if (child == 0) {
			for (j = 0; j < N; j++) {
				ipc_send(multiply[j], IN[i][j], 0, 0);
			}
			return;
		}
	}
}

