#include <stdio.h>
#include <stdlib.h>

int main()
{
    int n1 = 0;
    int n2 = 0;

    int l1 = scanf("%d", &n1);
    int l2 = scanf("%d", &n2);

    l1 = l2 + l1;

    printf("%d", n1 + n2);

    return 0;
}