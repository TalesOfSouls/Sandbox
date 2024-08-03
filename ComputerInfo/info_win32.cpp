#include<stdio.h>

#include "../../GameEngine/utils/SystemInfo.h"

int main(int, char*)
{
    SystemInfo info;
    get_system_info(&info);

    char buf[4096];
    render_system_info(buf, &info);

    printf("%s\n", buf);
    printf("%d\n", (int) strlen(buf));
    printf("%d", (int) sizeof(SystemInfo));

	return 0;
}
