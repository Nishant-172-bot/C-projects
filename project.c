#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TASKS 100
#define MAX_NOTES 1000
#define TASK_FILE "tasks.txt"
#define NOTE_FILE "notes.txt"
#define PASSWORD_FILE "password.txt"

// ANSI escape codes for colors
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_RESET "\x1b[0m"

struct Task {
    int id;
    char description[100];
    char priority[10];  // High, Medium, Low
    char status[10];    // Pending, Completed
    char deadline[15];  // YYYY-MM-DD
    char category[15];  // Work, Study, Personal, Other
};

struct Note {
    char date[15];      // YYYY-MM-DD
    char content[200];  // Note content
};

struct Task tasks[MAX_TASKS];
struct Note notes[MAX_NOTES];
int taskCount = 0;
int noteCount = 0;

// Function to clear the screen (cross-platform)
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Function to safely get today's date
void getTodayDate(char *todayDate, size_t size) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    strftime(todayDate, size, "%Y-%m-%d", tm);
}

// Optimized function to load tasks from file
int loadTasks() {
    FILE *file = fopen(TASK_FILE, "r");
    if (!file) return 0;
    taskCount = 0;
    struct Task tempTask;
    while (taskCount < MAX_TASKS &&
           fscanf(file, "%d\n%99[^\n]\n%9[^\n]\n%9[^\n]\n%14[^\n]\n%14[^\n]\n",
                  &tempTask.id,
                  tempTask.description,
                  tempTask.priority,
                  tempTask.status,
                  tempTask.deadline,
                  tempTask.category) == 6) {
        tasks[taskCount++] = tempTask;
    }
    fclose(file);
    return 1;
}

// Optimized function to save tasks to file
void saveTasks() {
    FILE *file = fopen(TASK_FILE, "w");
    if (!file) {
        perror("Error saving tasks");
        return;
    }
    for (int i = 0; i < taskCount; i++) {
        fprintf(file, "%d\n%s\n%s\n%s\n%s\n%s\n",
                tasks[i].id, tasks[i].description,
                tasks[i].priority, tasks[i].status,
                tasks[i].deadline, tasks[i].category);
    }
    fclose(file);
}

// Optimized function to load notes from file
int loadNotes() {
    FILE *file = fopen(NOTE_FILE, "r");
    if (!file) return 0;

    noteCount = 0;
    struct Note tempNote;
    while (noteCount < MAX_NOTES &&
           fscanf(file, "%14[^:]: %199[^\n]\n",
                  tempNote.date, tempNote.content) == 2) {
        notes[noteCount++] = tempNote;
    }
    fclose(file);
    return 1;
}

// Optimized function to save notes to file
void saveNotes() {
    FILE *file = fopen(NOTE_FILE, "w");
    if (!file) {
        printf(COLOR_RED "Error: Could not save notes to file.\n" COLOR_RESET);
        return;
    }

    for (int i = 0; i < noteCount; i++) {
        fprintf(file, "%s: %s\n", notes[i].date, notes[i].content);
    }

    fclose(file);
    printf(COLOR_GREEN "Successfully saved %d notes.\n" COLOR_RESET, noteCount);
}

// Safely calculate deadline countdown
void calculateCountdown(const char *deadline, char *countdown, size_t size) {
    struct tm deadline_tm = {0};
    time_t now = time(NULL);
    struct tm *now_tm = localtime(&now);

    if (sscanf(deadline, "%d-%d-%d",
               &deadline_tm.tm_year, &deadline_tm.tm_mon, &deadline_tm.tm_mday) != 3) {
        snprintf(countdown, size, "Invalid date");
        return;
    }
    deadline_tm.tm_year -= 1900;
    deadline_tm.tm_mon -= 1;

    double seconds = difftime(mktime(&deadline_tm), now);
    if (seconds < 0) {
        snprintf(countdown, size, "Deadline passed");
    } else {
        int days = seconds / 86400;
        int hours = (seconds - days * 86400) / 3600;
        snprintf(countdown, size, "%d days, %d hours", days, hours);
    }
}

// Safely add a task
void addTask() {
    if (taskCount >= MAX_TASKS) {
        printf(COLOR_RED "Task list is full!\n" COLOR_RESET);
        return;
    }

    struct Task *newTask = &tasks[taskCount];
    newTask->id = taskCount + 1;

    printf(COLOR_CYAN "Enter task description: " COLOR_RESET);
    if (fgets(newTask->description, sizeof(newTask->description), stdin) == NULL) {
        printf(COLOR_RED "Error reading description.\n" COLOR_RESET);
        return;
    }
    newTask->description[strcspn(newTask->description, "\n")] = '\0';

    printf(COLOR_CYAN "Enter priority (High/Medium/Low): " COLOR_RESET);
    if (scanf(" %9[^\n]", newTask->priority) != 1) {
        printf(COLOR_RED "Invalid priority.\n" COLOR_RESET);
        return;
    }

    printf(COLOR_CYAN "Enter deadline (YYYY-MM-DD): " COLOR_RESET);
    if (scanf(" %14[^\n]", newTask->deadline) != 1) {
        printf(COLOR_RED "Invalid deadline.\n" COLOR_RESET);
        return;
    }

    printf(COLOR_CYAN "Enter category (Work/Study/Personal/Other): " COLOR_RESET);
    if (scanf(" %14[^\n]", newTask->category) != 1) {
        printf(COLOR_RED "Invalid category.\n" COLOR_RESET);
        return;
    }

    strcpy(newTask->status, "Pending");
    taskCount++;
    saveTasks();
    printf(COLOR_GREEN "Task added successfully!\n" COLOR_RESET);
}

// Improved task display function
void displayTasks() {
    if (taskCount == 0) {
        printf(COLOR_YELLOW "No tasks available!\n" COLOR_RESET);
        return;
    }

    printf("\n" COLOR_BLUE "%-5s %-30s %-10s %-10s %-12s %-15s %-20s\n" COLOR_RESET,
           "ID", "Task", "Priority", "Status", "Deadline", "Category", "Time Left");
    printf(COLOR_BLUE "------------------------------------------------------------------------------------------------\n" COLOR_RESET);

    char countdown[50];
    for (int i = 0; i < taskCount; i++) {
        calculateCountdown(tasks[i].deadline, countdown, sizeof(countdown));
        printf("%-5d %-30s %-10s %-10s %-12s %-15s %-20s\n",
               tasks[i].id, tasks[i].description,
               tasks[i].priority, tasks[i].status,
               tasks[i].deadline, tasks[i].category, countdown);
    }
}

// Robust task deletion function
void deleteTask() {
    if (taskCount == 0) {
        printf(COLOR_YELLOW "No tasks available to delete!\n" COLOR_RESET);
        return;
    }

    displayTasks(); // Show the current list of tasks

    int id;
    printf(COLOR_CYAN "Enter the ID of the task to delete: " COLOR_RESET);
    if (scanf("%d", &id) != 1) {
        printf(COLOR_RED "Invalid input! Please enter a valid task ID.\n" COLOR_RESET);
        while (getchar() != '\n'); // Clear the input buffer
        return;
    }

    int found = 0;
    for (int i = 0; i < taskCount; i++) {
        if (tasks[i].id == id) {
            // Shift all tasks after the deleted task one position to the left
            for (int j = i; j < taskCount - 1; j++) {
                tasks[j] = tasks[j + 1];
            }
            taskCount--; // Reduce the task count
            found = 1;
            break;
        }
    }

    if (found) {
        // Update the IDs of the remaining tasks
        for (int i = 0; i < taskCount; i++) {
            tasks[i].id = i + 1; // Assign new sequential IDs
        }

        saveTasks(); // Save the updated task list to the file
        printf(COLOR_GREEN "Task with ID %d deleted successfully!\n" COLOR_RESET, id);
    } else {
        printf(COLOR_RED "Task with ID %d not found!\n" COLOR_RESET, id);
    }
}

// Improved mark completed function
void markCompleted() {
    if (taskCount == 0) {
        printf(COLOR_YELLOW "No tasks available to mark as completed!\n" COLOR_RESET);
        return;
    }

    displayTasks(); // Show the current list of tasks

    int id;
    printf(COLOR_CYAN "Enter the ID of the task to mark as completed: " COLOR_RESET);
    if (scanf("%d", &id) != 1) {
        printf(COLOR_RED "Invalid input! Please enter a valid task ID.\n" COLOR_RESET);
        while (getchar() != '\n'); // Clear the input buffer
        return;
    }

    int found = 0;
    for (int i = 0; i < taskCount; i++) {
        if (tasks[i].id == id) {
            strcpy(tasks[i].status, "Completed"); // Update the task status
            found = 1;
            break;
        }
    }

    if (found) {
        saveTasks(); // Save the updated task list to the file
        printf(COLOR_GREEN "Task with ID %d marked as completed!\n" COLOR_RESET, id);
    } else {
        printf(COLOR_RED "Task with ID %d not found!\n" COLOR_RESET, id);
    }
}

// Improved note addition function
void addNote() {
    if (noteCount >= MAX_NOTES) {
        printf(COLOR_RED "Note list is full!\n" COLOR_RESET);
        return;
    }

    struct Note *newNote = &notes[noteCount];

    printf(COLOR_CYAN "Enter date (YYYY-MM-DD): " COLOR_RESET);
    if (scanf(" %14[^\n]", newNote->date) != 1) {
        printf(COLOR_RED "Invalid date!\n" COLOR_RESET);
        return;
    }

    printf(COLOR_CYAN "Enter note: " COLOR_RESET);
    getchar(); // Clear previous newline
    if (fgets(newNote->content, sizeof(newNote->content), stdin) == NULL) {
        printf(COLOR_RED "Error reading note.\n" COLOR_RESET);
        return;
    }
    newNote->content[strcspn(newNote->content, "\n")] = '\0';

    noteCount++;
    saveNotes();
    printf(COLOR_GREEN "Note added successfully!\n" COLOR_RESET);
}

// Improved notes viewing function
void viewNotes() {
    if (noteCount == 0) {
        printf(COLOR_YELLOW "No notes available!\n" COLOR_RESET);
        return;
    }

    char filterDate[15] = {0};
    printf(COLOR_CYAN "Enter date to filter notes (YYYY-MM-DD) or leave blank to view all: " COLOR_RESET);
    fgets(filterDate, sizeof(filterDate), stdin);
    filterDate[strcspn(filterDate, "\n")] = '\0'; // Remove the newline character

    printf("\n" COLOR_BLUE "%-15s %-50s\n" COLOR_RESET, "Date", "Note");
    printf(COLOR_BLUE "------------------------------------------------------------\n" COLOR_RESET);

    int found = 0; // Track if any notes match the filter
    for (int i = 0; i < noteCount; i++) {
        // If filterDate is empty or matches the note's date, display the note
        if (strlen(filterDate) == 0 || strcmp(notes[i].date, filterDate) == 0) {
            printf("%-15s %-50s\n", notes[i].date, notes[i].content);
            found = 1; // At least one note matches
        }
    }

    if (!found && strlen(filterDate) > 0) {
        printf(COLOR_YELLOW "No notes found for the date '%s'.\n" COLOR_RESET, filterDate);
    }
}
// Secure password verification
int verifyPassword() {
    FILE *file = fopen(PASSWORD_FILE, "r");
    char storedPassword[50] = {0};
    char inputPassword[50] = {0};

    if (file) {
        if (fgets(storedPassword, sizeof(storedPassword), file) == NULL) {
            printf(COLOR_CYAN "No password set. Create a new password: " COLOR_RESET);
            if (scanf(" %49[^\n]", storedPassword) != 1) {
                printf(COLOR_RED "Invalid password.\n" COLOR_RESET);
                fclose(file);
                return 0;
            }
            file = freopen(PASSWORD_FILE, "w", file);
            fprintf(file, "%s", storedPassword);
        }
        fclose(file);
    } else {
        printf(COLOR_CYAN "No password set. Create a new password: " COLOR_RESET);
        if (scanf(" %49[^\n]", storedPassword) != 1) {
            printf(COLOR_RED "Invalid password.\n" COLOR_RESET);
            return 0;
        }
        file = fopen(PASSWORD_FILE, "w");
        fprintf(file, "%s", storedPassword);
        fclose(file);
        printf(COLOR_GREEN "Password set successfully!\n" COLOR_RESET);
        return 1;
    }

    // Trim newline from stored password
    storedPassword[strcspn(storedPassword, "\n")] = '\0';

    printf("Enter password: ");
    if (scanf(" %49[^\n]", inputPassword) != 1) {
        printf(COLOR_RED "Invalid input.\n" COLOR_RESET);
        return 0;
    }

    if (strcmp(inputPassword, storedPassword) == 0) {
        printf("Access granted!\n");
        return 1;
    } else {
        printf("Incorrect password. Access denied.\n");
        return 0;
    }
}

// Display menu
void displayMenu() {
    clearScreen();
    char todayDate[15];
    getTodayDate(todayDate, sizeof(todayDate));
    printf(COLOR_MAGENTA "\n========================================\n");
    printf("          To-Do List Manager           \n");
    printf("========================================\n" COLOR_RESET);
    printf(COLOR_CYAN "Today's Date: %s\n\n" COLOR_RESET, todayDate);
    printf(COLOR_CYAN "1. Add Task\n");
    printf("2. View Tasks\n");
    printf("3. Delete Task\n");
    printf("4. Mark Task as Completed\n");
    printf("5. Add Daily Note\n");
    printf("6. View Notes\n");
    printf("7. Exit\n" COLOR_RESET);
    printf(COLOR_MAGENTA "========================================\n" COLOR_RESET);
    printf(COLOR_CYAN "Enter your choice: " COLOR_RESET);
}

// Main function
int main() {
    if (!verifyPassword()) {
        return 0; // Exit if password is incorrect
    }

    loadTasks();
    loadNotes();

    int choice;
    while (1) {
        displayMenu();
        if (scanf("%d", &choice) != 1) {
            // Clear invalid input
            while (getchar() != '\n');
            continue;
        }

        // Clear input buffer after menu choice
        while (getchar() != '\n');

        switch (choice) {
            case 1: addTask(); break;
            case 2: displayTasks(); break;
            case 3: deleteTask(); break;
            case 4: markCompleted(); break;
            case 5: addNote(); break;
            case 6: viewNotes(); break;
            case 7: printf(COLOR_GREEN "Exiting...\n" COLOR_RESET); return 0;
            default: printf(COLOR_RED "Invalid choice! Try again.\n" COLOR_RESET);
        }
        printf(COLOR_CYAN "\nPress Enter to continue..." COLOR_RESET);
        getchar(); getchar(); // Wait for user input
    }
    return 0;

}
