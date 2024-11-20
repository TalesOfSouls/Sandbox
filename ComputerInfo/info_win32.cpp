#include <stdio.h>

#include "../../GameEngine/platform/SystemInfo.h"

#if _WIN32
    #include "../../GameEngine/platform/win32/SystemInfo.cpp"
#else
    #include "../../GameEngine/platform/linux/SystemInfo.cpp"
#endif

int main(int, char*)
{
    SystemInfo info;
    system_info_get(&info);

    char buf[4096];
    system_info_render(buf, &info);

    printf("%s\n\n", buf);

    // The SystemInfo has a lot of additional space since we don't know how much is actually getting filled
    // It is very common for the string representation to be actually smaller than the reserved memory in SystemInfo
    printf("Buffer size: %d, SystemInfo max size: %d\n", (int) strlen(buf), (int) sizeof(SystemInfo));

	return 0;
}
