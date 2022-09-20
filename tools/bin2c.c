#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BUFSIZE 256

void normalize_name(char *name) {
    for(size_t i=0; name[i]!=0;) {
        if('a'<=name[i]&&name[i]<='z')
            goto next;
        else if('A'<=name[i]&&name[i]<='Z')
            goto next;
        else if('0'<=name[i]&&name[i]<='9')
            goto next;
        else
            name[i]='_';
next:
        ++i;
    }
}

void convert_to_char_array(const char *name, FILE *in, FILE *out) {
    fseek(in, 0, SEEK_END);
    size_t file_size=ftell(in);
    fseek(in, 0, SEEK_SET);

    fprintf(out, "const unsigned long %s_size=%lu;\n", name, file_size);
    uint8_t buf[BUFSIZE];
    size_t read_bytes, wrote_bytes=0;

    fprintf(out, "unsigned char %s_data[]={\n", name);

    do {
        read_bytes=fread(buf, 1, BUFSIZE, in);

        for(size_t i=0; i<read_bytes;) {
            if(++wrote_bytes<file_size)
                fprintf(out, "0x%02x,", buf[i]);
            else
                fprintf(out, "0x%02x", buf[i]);
            ++i;
            if(((i&15)==0)&&i>0)
                fprintf(out, "\n");
        }
    } while(read_bytes==BUFSIZE);
    fprintf(out, "\n};\n");
}

int main(int ac, char *as[]) {
    if(ac<2||ac>3) {
        fprintf(stderr, "Usage: %s [input file] [output file?]\n", as[0]);
        return EXIT_FAILURE;
    }

    FILE *in=NULL, *out=NULL;

    if((in=fopen(as[1], "rb"))==NULL) {
        fprintf(stderr, "Could not open \"%s\" for reading!\n", as[1]);
        goto fail;
    }

    if(ac==3) {
        if((out=fopen(as[2], "w"))==NULL) {
            fprintf(stderr, "Could not open \"%s\" for writing!\n", as[2]);
            goto fail;
        }
    } else
        out=stdout;

    char *norm_name=strdup(as[1]);
    normalize_name(norm_name);

    convert_to_char_array(norm_name, in, out);

    free(norm_name);

    fclose(in);
    fclose(out);

    return EXIT_SUCCESS;

fail:
    if(in)
        fclose(in);
    if(out)
        fclose(out);
    return EXIT_FAILURE;
}
