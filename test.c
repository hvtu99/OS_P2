#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int main(void)
{
	int flag, file_device, my_num;

	printf("Opening...\n");
	file_device = open("/dev/my_rand", O_RDONLY);
	if(file_device < 0)
	{
		perror("Failed to open the device!!\n");
		return errno;
	}

	printf("Receiving...\n");
	flag = read(file_device, &my_num, sizeof(my_num));
	if(flag < 0)
	{
		perror("Failed to read from device!!!\n");
		return errno;
	}

	printf("Random number: %d\n", my_num);
	printf("Finish!");

	return 0;
}
	
	
