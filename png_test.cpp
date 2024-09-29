#include <stdio.h>

#include "handmade_png.cpp"

internal stream
ReadEntireFile(const char *FileName, stream *Errors)
{
    buffer Buffer = {};

    FILE *In = fopen(FileName, "rb");
    if(In)
    {
        fseek(In, 0, SEEK_END);
        Buffer.Count = ftell(In);
        fseek(In, 0, SEEK_SET);

        Buffer.Data = (u8 *)malloc(Buffer.Count);
        fread(Buffer.Data, Buffer.Count, 1, In);
        fclose(In);
    }

    stream Result = MakeReadStream(Buffer, Errors);
    return(Result);
}

int main(int, char*)
{
    /*int w2,h2;
    unsigned char *data2;
    data2 = stbi_load("C:/Users/spl1nes/Documents/git/TalesOfSouls/build/editor/engine/assets/images/font_atlas1.png", &w2, &h2, 0, 4);
    */

    memory_arena Memory = {};
    stream ErrorStream = OnDemandMemoryStream(&Memory);
    stream InfoStream = OnDemandMemoryStream(&Memory, &ErrorStream);

    stream File = ReadEntireFile("C:/Users/spl1nes/Documents/git/TalesOfSouls/build/editor/engine/assets/images/font_atlas1.png", &ErrorStream);
    image_u32 Image = ParsePNG(&Memory, File, &InfoStream);

	return 0;
}
