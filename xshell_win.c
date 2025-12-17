#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024

/* ---------------- timing ---------------- */

double now_sec() {
    static LARGE_INTEGER freq;
    static int init = 0;
    LARGE_INTEGER t;

    if (!init) {
        QueryPerformanceFrequency(&freq);
        init = 1;
    }

    QueryPerformanceCounter(&t);
    return (double)t.QuadPart / (double)freq.QuadPart;
}

/* ---------------- runner ---------------- */

int run_cmd(char *cmd, double *elapsed) {
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    double start = now_sec();

    if (!CreateProcessA(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        printf("exec failed (%lu)\n", GetLastError());
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    double end = now_sec();
    *elapsed = end - start;

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}

/* ---------------- shadow ---------------- */

void shadow(char *cmd) {
    printf("[shadow] %s\n", cmd);
}

/* ---------------- measure ---------------- */

void measure(char *cmd) {
    const int warmups = 2;
    const int runs = 5;
    double total = 0.0;

    printf("measuring steady-state execution...\n");

    for (int i = 0; i < warmups + runs; i++) {
        double t;
        run_cmd(cmd, &t);

        if (i >= warmups) {
            total += t;
            printf("run %d: %.4fs\n", i - warmups + 1, t);
        }
    }

    printf("avg (steady): %.4fs\n", total / runs);
}

/* ---------------- budget ---------------- */

void budget(double limit, char *cmd) {
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    HANDLE job = CreateJobObject(NULL, NULL);

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {0};
    info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    SetInformationJobObject(
        job,
        JobObjectExtendedLimitInformation,
        &info,
        sizeof(info)
    );

    double start = now_sec();

    if (!CreateProcessA(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        CREATE_SUSPENDED,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        printf("exec failed (%lu)\n", GetLastError());
        return;
    }

    AssignProcessToJobObject(job, pi.hProcess);
    ResumeThread(pi.hThread);

    DWORD res = WaitForSingleObject(
        pi.hProcess,
        (DWORD)(limit * 1000)
    );

    if (res == WAIT_TIMEOUT) {
        printf("[budget] time limit exceeded — killed\n");
        CloseHandle(job); // nukes process tree
    } else {
        double elapsed = now_sec() - start;
        printf("[budget] OK: %.2fs\n", elapsed);
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

/* ---------------- shell ---------------- */

int main() {
    char line[MAX_LINE];

    printf("XShell (Windows) -- reality-aware shell \n");

    while (1) {
        printf("xshell> ");
        if (!fgets(line, sizeof(line), stdin))
            break;

        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, "exit") == 0)
            break;

        if (strncmp(line, "shadow ", 7) == 0) {
            shadow(line + 7);
            continue;
        }

        if (strncmp(line, "measure ", 8) == 0) {
            measure(line + 8);
            continue;
        }

        if (strncmp(line, "budget ", 7) == 0) {
            char *rest = line + 7;
            double t = atof(rest);

            while (*rest && *rest != ' ') rest++;
            while (*rest == ' ') rest++;

            budget(t, rest);
            continue;
        }

        double t;
        run_cmd(line, &t);
    }

    return 0;
}
