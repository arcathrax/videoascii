#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <glob.h>
#include <math.h>

// Expand ~ to $HOME
char *expand_path(const char *path) {
    if (path[0] == '~') {
        const char *home = getenv("HOME");
        char *expanded = malloc(strlen(home) + strlen(path));
        sprintf(expanded, "%s%s", home, path + 1);
        return expanded;
    }
    return strdup(path);
}

// Check if directory exists
int directory_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <full_path_to_video>\n", argv[0]);
        return 1;
    }

    const char *videoPath = argv[1];
    char *outputDir = expand_path("~/.videoascii/");

    if (directory_exists(outputDir)) {
        printf("%s does exist.\n", outputDir);
    } else {
        printf("%s does not exist, creating this folder.\n", outputDir);
        mkdir(outputDir, 0755);
    }

    // Get FPS using ffprobe
    char ffprobeCmd[1024];
    snprintf(ffprobeCmd, sizeof(ffprobeCmd),
             "ffprobe -v error -select_streams v:0 -show_entries stream=r_frame_rate "
             "-of default=noprint_wrappers=1:nokey=1 \"%s\"",
             videoPath);

    FILE *fp = popen(ffprobeCmd, "r");
    if (!fp) {
        perror("Failed to run ffprobe");
        return 1;
    }

    char fpsBuffer[128];
    if (!fgets(fpsBuffer, sizeof(fpsBuffer), fp)) {
        fprintf(stderr, "Could not read FPS info.\n");
        pclose(fp);
        return 1;
    }
    pclose(fp);

    // Parse FPS as fraction
    int num, den;
    if (sscanf(fpsBuffer, "%d/%d", &num, &den) != 2 || den == 0) {
        fprintf(stderr, "Invalid FPS format: %s\n", fpsBuffer);
        return 1;
    }

    double fps = (double)num / den;
    double delay = 1.0 / fps;

    // Extract frames
    char ffmpegCmd[1024];
    snprintf(ffmpegCmd, sizeof(ffmpegCmd),
             "ffmpeg -i \"%s\" -vsync 0 \"%s/frame_%%04d.png\"",
             videoPath, outputDir);
    system(ffmpegCmd);

    // Iterate through PNG frames
    char pattern[1024];
    snprintf(pattern, sizeof(pattern), "%s/frame_*.png", outputDir);

    glob_t glob_result;
    if (glob(pattern, 0, NULL, &glob_result) == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
            char *framePath = glob_result.gl_pathv[i];
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), "./img_to_txt/img_to_txt \"%s\"", framePath);
            system(cmd);
            usleep((useconds_t)(delay * 1e6)); // convert seconds to microseconds
        }
        globfree(&glob_result);
    } else {
        fprintf(stderr, "No frames found.\n");
    }

    // Cleanup
    char cleanupCmd[1024];
    snprintf(cleanupCmd, sizeof(cleanupCmd), "rm -rf %s/*", outputDir);
    system(cleanupCmd);

    free(outputDir);
    return 0;
}
