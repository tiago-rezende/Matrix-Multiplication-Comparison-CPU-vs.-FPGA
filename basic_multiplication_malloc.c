#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <psapi.h>

// Dimensões das matrizes
#define R1 2 // Número de linhas da Matriz-1
#define C1 2 // Número de colunas da Matriz-1
#define R2 2 // Número de linhas da Matriz-2
#define C2 2 // Número de colunas da Matriz-2

void multiplyMatrix(int *m1, int *m2, int r1, int r2, int c1, int c2) {
    int *result = (int *)malloc(R1 * C2 * sizeof(int));
    printf("Resultant Matrix is:\n");

    for (int i = 0; i < r1; i++) {
        for (int j = 0; j < c2; j++) {
            result[i * c2 + j] = 0; // Inicializa o valor da célula
            for (int k = 0; k < c1; k++) { // C1 é o mesmo que R2
                result[i * c2 + j] += m1[i * c1 + k] * m2[k * c2 + j];
            }
            printf("%d\t", result[i * c2 + j]);
        }
        printf("\n");
    }
}

double measureExecutionTime(void (*func)(int *, int *, int, int, int, int), int *m1, int *m2, int r1, int r2, int c1, int c2) {
    clock_t start, end;
    start = clock();
    func(m1, m2, r1, r2, c1, c2);
    end = clock();
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

void printSystemInfo(const char* message) {
    PROCESS_MEMORY_COUNTERS memCounter;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter))) {
        printf("%s - Uso de Memória: %zu KB\n", message, memCounter.WorkingSetSize / 1024);
    }

    FILETIME createTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime)) {
        SYSTEMTIME userSystemTime;
        FileTimeToSystemTime(&userTime, &userSystemTime);
        printf("%s - Tempo de CPU usado: %d segundos\n", 
               message,
               (userSystemTime.wHour * 3600 + userSystemTime.wMinute * 60 + userSystemTime.wSecond));
    }

    HKEY hKey;
    DWORD dataSize = sizeof(DWORD);
    DWORD processorFrequency;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, TEXT("~MHz"), NULL, NULL, (LPBYTE)&processorFrequency, &dataSize);
        printf("%s - Frequencia do Processador: %lu MHz\n", message, processorFrequency);
        RegCloseKey(hKey);
    }
}


void printMatrix(int *matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

int main() {
    // Inicializa matrizes com valores aleatórios
     
    int *m1 = (int *)malloc(R1 * C1 * sizeof(int));
    int *m2 = (int *)malloc(R2 * C2 * sizeof(int));

    int totalElementsM1 = R1 * C1;
    int totalElementsM2 = R2 * C2;

    for (int i = 0; i < (totalElementsM1 > totalElementsM2 ? totalElementsM1 : totalElementsM2); i++) {
        if (i < totalElementsM1) {
            m1[i] = rand() % 100; // Valores aleatórios entre 0 e 99
        }
        if (i < totalElementsM2) {
            m2[i] = rand() % 100; // Valores aleatórios entre 0 e 99
        }
    }


    // Verifica se a multiplicação de matrizes é possível
    if (C1 != R2) {
        printf("O número de colunas na Matriz-1 deve ser igual ao número de linhas na Matriz-2\n");
        exit(EXIT_FAILURE);
    }

    printf("Matriz 1:\n");
    printMatrix(m1, R1, C1);
    printf("\nMatriz 2:\n");
    printMatrix(m2, R2, C2);

    // Mede o tempo de execução da multiplicação de matrizes
    double executionTime = measureExecutionTime(multiplyMatrix, m1, m2, R1, R2, C1, C2);
    printf("Tempo de execução: %.6f segundos\n", executionTime);

    // Coleta informações do sistema após a execução
    printSystemInfo("Após a execução");

    return 0;
}
