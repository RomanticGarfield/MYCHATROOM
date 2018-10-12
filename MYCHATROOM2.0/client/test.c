#include <stdio.h>

int  main()
{
		struct ss{
				int a;
				char b;
				int c;
		};

		printf("%d",sizeof(struct ss));
		return 0;
}
