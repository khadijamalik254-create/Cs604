#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
    char studentID[50];
    int tasks[3];
    int pipe1[2];
    int pipe2[2];

    printf("Enter Student ID: ");
    scanf("%s", studentID);

    for (int i = 0; i < 3; i++) {
        printf("Enter Task %d Execution Time: ", i + 1);
        scanf("%d", &tasks[i]);
        if (tasks[i] <= 0) {
            printf("Error: Task time must be a positive integer.\n");
            return 1;
        }
    }

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        int receivedTasks[3];
        read(pipe1[0], receivedTasks, sizeof(receivedTasks));
        close(pipe1[0]);

        int total = receivedTasks[0] + receivedTasks[1] + receivedTasks[2];
        int maxTime = receivedTasks[0];
        int longestTask = 1;

        for (int i = 1; i < 3; i++) {
            if (receivedTasks[i] > maxTime) {
                maxTime = receivedTasks[i];
                longestTask = i + 1;
            }
        }

        int results[3];
        results[0] = total;
        results[1] = longestTask;
        results[2] = maxTime;

        write(pipe2[1], results, sizeof(results));
        close(pipe2[1]);
        exit(0);

    } else {
        close(pipe1[0]);
        close(pipe2[1]);

        write(pipe1[1], tasks, sizeof(tasks));
        close(pipe1[1]);

        int results[3];
        read(pipe2[0], results, sizeof(results));
        close(pipe2[0]);

        wait(NULL);

        int total = results[0];
        int longestTask = results[1];
        int maxTime = results[2];

        printf("\n--- Task Processing Report ---\n");
        printf("Student ID: %s\n", studentID);
        printf("Task 1: %d units\n", tasks[0]);
        printf("Task 2: %d units\n", tasks[1]);
        printf("Task 3: %d units\n", tasks[2]);
        printf("Total Execution Time: %d units\n", total);
        printf("Longest Task: Task %d\n", longestTask);
        printf("Maximum Execution Time: %d units\n", maxTime);

        char filename[100];
        sprintf(filename, "%s_report.txt", studentID);

        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("File open failed");
            return 1;
        }

        char report[500];
        sprintf(report,
            "--- Task Processing Report ---\n"
            "Student ID: %s\n"
            "Task 1: %d units\n"
            "Task 2: %d units\n"
            "Task 3: %d units\n"
            "Total Execution Time: %d units\n"
            "Longest Task: Task %d\n"
            "Maximum Execution Time: %d units\n",
            studentID, tasks[0], tasks[1], tasks[2],
            total, longestTask, maxTime);

        write(fd, report, strlen(report));
        close(fd);

        printf("Report saved to: %s\n", filename);
    }

    return 0;
}
