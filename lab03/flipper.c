#include <fcntl.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h> 

void flipper(char *inputPath, char *outputPath) {
    DIR *inputDir = opendir(inputPath);

    if (inputDir == NULL) {
        perror("Failed to open input directory");
        return;
    }
    
    struct dirent *entry;

    while ((entry = readdir(inputDir)) != NULL) {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        char *dot = strrchr(entry->d_name, '.');
        if (!dot || strcmp(dot, ".txt") != 0) { 
            continue; 
        }

        char finPath[1024];
        snprintf(finPath, sizeof(finPath), "%s/%s", inputPath, entry->d_name);

        char foutPath[1024];
        snprintf(foutPath, sizeof(foutPath), "%s/%s", outputPath, entry->d_name);
        
        FILE *fin = fopen(finPath, "r");
        if (!fin) {
            perror("fopen input");
            return;
        }

        FILE *fout = fopen(foutPath, "w");
        if (!fout) {
            perror("fopen output");
            fclose(fin);
            return;
        }

        char line[4096];
        while (fgets(line, sizeof(line), fin)) {
            int len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                len--;
            }
            for (int i = len - 1; i >= 0; i--) {
                fputc(line[i], fout);
            }
            fputc('\n', fout);
        }
        fclose(fin);
        fclose(fout);
    }
    closedir(inputDir);
}


int main() {

    char _inputPath[1024];
    char _outputPath[1024];

    printf("Input path: ");
    scanf("%s", _inputPath);

    printf("Output path: ");
    scanf("%s", _outputPath);

    flipper(_inputPath, _outputPath);

    return 0;
}