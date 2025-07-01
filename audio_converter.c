#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void write_header(FILE* out) {
    fprintf(out, "#include <stddef.h>\n\n");
}

void convert_file(const char* input_file, const char* output_file) {
    FILE *in = fopen(input_file, "rb");
    if (!in) {
        fprintf(stderr, "Error: Cannot open input file %s\n", input_file);
        exit(1);
    }

    FILE *out = fopen(output_file, "w");
    if (!out) {
        fprintf(stderr, "Error: Cannot create output file %s\n", output_file);
        fclose(in);
        exit(1);
    }

    // Get file size
    fseek(in, 0, SEEK_END);
    long size = ftell(in);
    fseek(in, 0, SEEK_SET);

    // Read file content
    unsigned char *buffer = (unsigned char*)malloc(size);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(in);
        fclose(out);
        exit(1);
    }
    fread(buffer, 1, size, in);

    // Generate variable name from input filename
    char var_name[256];
    const char* base_name = strrchr(input_file, '/');
    if (!base_name) base_name = strrchr(input_file, '\\');
    if (!base_name) base_name = input_file;
    else base_name++;
    
    // Extract name without extension
    strncpy(var_name, base_name, sizeof(var_name) - 1);
    var_name[sizeof(var_name) - 1] = '\0';
    char* dot = strchr(var_name, '.');
    if (dot) *dot = '\0';
    
    // Replace any non-alphanumeric characters with underscore
    for (char* p = var_name; *p; p++) {
        if (!isalnum(*p)) *p = '_';
    }

    // Write header and array declaration
    write_header(out);
    fprintf(out, "const unsigned char %s_data[] = {\n    ", var_name);

    // Write data as hex values
    for (long i = 0; i < size; i++) {
        fprintf(out, "0x%02X", buffer[i]);
        if (i < size - 1) fprintf(out, ", ");
        if ((i + 1) % 12 == 0 && i < size - 1) fprintf(out, "\n    ");
    }

    // Write array size variable
    fprintf(out, "\n};\n\nconst size_t %s_size = sizeof(%s_data);\n", var_name, var_name);

    free(buffer);
    fclose(in);
    fclose(out);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    convert_file(argv[1], argv[2]);
    return 0;
} 