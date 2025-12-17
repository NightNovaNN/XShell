#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

#define MAX_ARGS 64
#define MAX_LINE 1024

/* ----------------- utils ----------------- */

double now_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void split(char *line, char **args) {
    int i = 0;
    char *tok = strtok(line, " \t\n");
    while (tok && i < MAX_ARGS - 1) {
        args[i++] = tok;
        tok = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

/* ----------------- command runner ----------------- */

int run_cmd(char **args, double *elapsed) {
    double start = now_sec();

    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("exec");
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    double end = now_sec();
    *elapsed = end - start;

    return status;
}

/* ----------------- shadow ----------------- */

void shadow(char **args) {
    printf("[shadow] ");
    for (int i = 0; args[i]; i++)
        printf("%s ", args[i]);
    printf("\n");
}

/* ----------------- measure ----------------- */

void measure(char **args) {
    const int warmups = 2;
    const int runs = 5;

    double total = 0.0;

    printf("measuring steady-state execution...\n");

    for (int i = 0; i < warmups + runs; i++) {
        double t;
        run_cmd(args, &t);

        if (i >= warmups) {
            total += t;
            printf("run %d: %.4fs\n", i - warmups + 1, t);
        }
    }

    printf("avg (steady): %.4fs\n", total / runs);
}

/* ----------------- budget ----------------- */

pid_t budget_pid = -1;

void kill_child(int sig) {
    if (budget_pid > 0) {
        kill(budget_pid, SIGKILL);
        printf("\n[budget] time limit exceeded — killed\n");
    }
}

void budget(double limit, char **args) {
    signal(SIGALRM, kill_child);

    double start = now_sec();
    alarm((unsigned int)limit);

    budget_pid = fork();
    if (budget_pid == 0) {
        execvp(args[0], args);
        exit(1);
    }

    int status;
    waitpid(budget_pid, &status, 0);
    alarm(0);

    double elapsed = now_sec() - start;

    if (elapsed > limit)
        printf("[budget] WARNING: %.2fs > %.2fs\n", elapsed, limit);
    else
        printf("[budget] OK: %.2fs\n", elapsed);
}

/* ----------------- shell loop ----------------- */

int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    printf("XShell (proto) — reality-aware shell 🧠\n");

    while (1) {
        printf("xshell> ");
        if (!fgets(line, sizeof(line), stdin))
            break;

        if (strncmp(line, "exit", 4) == 0)
            break;

        split(line, args);
        if (!args[0]) continue;

        /* shadow */
        if (strcmp(args[0], "shadow") == 0) {
            shadow(&args[1]);
            continue;
        }

        /* measure */
        if (strcmp(args[0], "measure") == 0) {
            measure(&args[1]);
            continue;
        }

        /* budget */
        if (strcmp(args[0], "budget") == 0) {
            double t = atof(args[1]);
            budget(t, &args[2]);
            continue;
        }

        /* normal command */
        double t;
        run_cmd(args, &t);
    }

    return 0;
}
