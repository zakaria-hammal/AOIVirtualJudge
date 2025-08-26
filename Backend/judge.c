#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <errno.h>

pid_t pid;
FILE* judgeOutput;
int a1, a2, a3;
int timeout;

void waitHandler(int sig) {
    timeout = 1;
    kill(pid, SIGKILL);
}

// Competitive programming judges' approach for MLE detection
int set_memory_limit(pid_t pid, size_t memory_limit_bytes) {
    char path[256];
    int fd;

    // Create cgroup directory
    snprintf(path, sizeof(path), "/sys/fs/cgroup/judge_%d", pid);
    if (mkdir(path, 0755) && errno != EEXIST) {
        return -1;
    }

    // Set memory limit
    snprintf(path, sizeof(path), "/sys/fs/cgroup/judge_%d/memory.max", pid);
    fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    dprintf(fd, "%zu", memory_limit_bytes);
    close(fd);

    // Add process to cgroup
    snprintf(path, sizeof(path), "/sys/fs/cgroup/judge_%d/cgroup.procs", pid);
    fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    dprintf(fd, "%d", pid);
    close(fd);

    return 0;
}

void cleanup_cgroup(pid_t pid) {
    char path[256];
    snprintf(path, sizeof(path), "/sys/fs/cgroup/judge_%d", pid);
    rmdir(path);
}

int main(int argc, char* argv[]) {
    // Saving standard in/out/err
    a1 = dup(STDOUT_FILENO);
    a2 = dup(STDIN_FILENO);
    a3 = dup(STDERR_FILENO);

    freopen("error.er", "w", stderr);
    judgeOutput = fopen("judgeOutput.out", "w");

    // Compiling the solution
    int ret = system("g++ -O2 Solution/solution.cpp -o solution");

    FILE* file = fopen("error.er", "rb");
    struct stat st;
    stat("error.er", &st);
    char *buffer1 = malloc(st.st_size + 1);
    int n = fread(buffer1, 1, st.st_size, file);
    fclose(file);

    buffer1[n] = '\0';
    if (ret) {
        if (n) {
            fprintf(judgeOutput, "Compilation Error :\n%s\n", buffer1);
        } else {
            fprintf(judgeOutput, "Compilation Error (No details)\n");
        }
        fflush(judgeOutput);
        free(buffer1);
        fclose(judgeOutput);
        dup2(a1, STDOUT_FILENO);
        dup2(a2, STDIN_FILENO);
        dup2(a3, STDERR_FILENO);
        return EXIT_SUCCESS;
    }
    free(buffer1);

    int nTests = atoi(argv[1]);
    int timeLimit = atoi(argv[2]);
    int memoryLimit = atoi(argv[3]); // in MB

    if (!timeLimit) timeLimit = 20;
    if (!memoryLimit) memoryLimit = 512;

    for (int i = 1; i <= nTests; i++) {
        char inputFile[100];
        snprintf(inputFile, sizeof(inputFile), "ExpectedIn/test%d.in", i);
        freopen(inputFile, "r", stdin);

        char soloutFile[100];
        snprintf(soloutFile, sizeof(soloutFile), "SolutionOutput/test%d.out", i);
        freopen(soloutFile, "w", stdout);

        timeout = 0;
        signal(SIGALRM, waitHandler);
        alarm(timeLimit);

        if (!(pid = fork())) {
            execlp("./solution", "./solution", NULL);
            perror("execlp failed");
            exit(EXIT_FAILURE);
        } else {
            // Set up cgroup memory limit (competitive judges approach)
            if (set_memory_limit(pid, memoryLimit * 1024 * 1024) < 0) {
                fprintf(judgeOutput, "Judge Error: Failed to set memory limit\n");
                fflush(judgeOutput);
                kill(pid, SIGKILL);
            }

            int status;
            waitpid(pid, &status, 0);
            alarm(0);
            signal(SIGALRM, SIG_IGN);

            cleanup_cgroup(pid);

            if (timeout) {
                fprintf(judgeOutput, "Time Limit Exceeded\n");
                fflush(judgeOutput);
                fclose(judgeOutput);
                dup2(a1, STDOUT_FILENO);
                dup2(a2, STDIN_FILENO);
                dup2(a3, STDERR_FILENO);
                return EXIT_SUCCESS;
            }

            if (WIFSIGNALED(status)) {
                int sig = WTERMSIG(status);

                // Check if SIGKILL was due to memory limit exceeded
                if (sig == SIGKILL) {
                    fprintf(judgeOutput, "Memory Limit Exceeded\n");
                } else {
                    fprintf(judgeOutput, "Runtime Error (signal %d)\n", sig);
                }
                fflush(judgeOutput);
                fclose(judgeOutput);
                dup2(a1, STDOUT_FILENO);
                dup2(a2, STDIN_FILENO);
                dup2(a3, STDERR_FILENO);
                return EXIT_SUCCESS;
            }

            if (WIFEXITED(status)) {
                int exitCode = WEXITSTATUS(status);
                if (exitCode != 0) {
                    fprintf(judgeOutput, "Runtime Error (exit code %d)\n", exitCode);
                    fflush(judgeOutput);
                    fclose(judgeOutput);
                    dup2(a1, STDOUT_FILENO);
                    dup2(a2, STDIN_FILENO);
                    dup2(a3, STDERR_FILENO);
                    return EXIT_SUCCESS;
                }
            }
        }

        file = fopen(soloutFile, "rb");
        stat(soloutFile, &st);
        buffer1 = malloc(st.st_size + 1);
        n = fread(buffer1, 1, st.st_size, file);
        fclose(file);
        buffer1[n] = '\0';

        char outputFile[100];
        snprintf(outputFile, sizeof(outputFile), "ExpectedOut/test%d.out", i);
        file = fopen(outputFile, "rb");
        stat(outputFile, &st);
        char *buffer2 = malloc(st.st_size + 1);
        n = fread(buffer2, 1, st.st_size, file);
        fclose(file);
        buffer2[n] = '\0';

        if (!strcmp(buffer1, buffer2)) {
            fprintf(judgeOutput, "Test %d passed successfully\n", i);
        } else {
            fprintf(judgeOutput, "Wrong Answer on test %d\n", i);
            free(buffer1);
            free(buffer2);
            fclose(judgeOutput);
            dup2(a1, STDOUT_FILENO);
            dup2(a2, STDIN_FILENO);
            dup2(a3, STDERR_FILENO);
            return EXIT_SUCCESS;
        }

        free(buffer1);
        free(buffer2);
    }

    fprintf(judgeOutput, "Accepted\n");
    fflush(judgeOutput);
    fclose(judgeOutput);
    dup2(a1, STDOUT_FILENO);
    dup2(a2, STDIN_FILENO);
    dup2(a3, STDERR_FILENO);
    return EXIT_SUCCESS;
}
