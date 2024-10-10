#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h> // Biblioteca para MPI

// Dimensões das matrizes (global)
typedef int T;
T DIM;

MPI_Status status;

// Função para medir o tempo de execução de uma função de multiplicação de matrizes
double measureExecutionTime(void (*func)(float[DIM][DIM], float[DIM][DIM], float[DIM][DIM], int, int, int), 
                            float A[DIM][DIM], float B[DIM][DIM], float C[DIM][DIM], 
                            int repetitions, int stride, int rank, int nprocs) {
    clock_t start, end;
    start = clock();
    for (int try = 0; try < repetitions; ++try) {
        func(A, B, C, stride, rank, nprocs); // Passar rank e nprocs
    }
    end = clock();
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

// Função de multiplicação de matrizes paralelizada com MPI
void matrix_multiply_parallel(float a[DIM][DIM], float b[DIM][DIM], float out[DIM][DIM], int stride, int rank, int nprocs) {
    int rows_per_proc = DIM / nprocs; // Divisão das linhas por processo
    int start_row = rank * rows_per_proc;
    int end_row = (rank == nprocs - 1) ? DIM : start_row + rows_per_proc;

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < DIM; j++) {
            float sum = 0.0;
            for (int k = 0; k < DIM; k++) {
                sum += a[i][k] * b[k][j];
            }
            out[i][j] = sum; // Armazena o resultado na matriz de saída
        }
    }
}

// Inicialização das matrizes com valores aleatórios
void initializeMatrices(float A[DIM][DIM], float B[DIM][DIM], float C[DIM][DIM], int dim) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            A[i][j] = rand() % 100;
            B[i][j] = rand() % 100;
            C[i][j] = 0.0f; // Inicializa matriz C com zeros
        }
    }
}

// Função para imprimir a matriz
void printMatrix(float matrix[DIM][DIM]) {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            printf("%.2f ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    FILE *file;
    if (rank == 0) {
        file = fopen("execution_times.txt", "w");
        if (file == NULL) {
            fprintf(stderr, "Erro ao abrir o arquivo para escrita.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    int size = 10;
    int powersOfTwo[size];
    for (int i = 0; i < size; i++) {
        powersOfTwo[i] = 1 << i; // 2^i
    }

    int repetitions = 1;

    for (int i = 1; i < size; i++) {
        DIM = powersOfTwo[i];
        float A[DIM][DIM];
        float B[DIM][DIM];
        float C[DIM][DIM];

        initializeMatrices(A, B, C, DIM);

        // Medição do tempo para o modelo paralelo
        double executionTime = measureExecutionTime(matrix_multiply_parallel, A, B, C, repetitions, 0, rank, nprocs);

        // Sincronizar antes de coletar os resultados
        MPI_Barrier(MPI_COMM_WORLD);

        // Reunir os resultados da matriz C na matriz C no rank 0
        MPI_Reduce(rank == 0 ? MPI_IN_PLACE : C, C, DIM * DIM, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

        // Imprimir as matrizes no rank 0
        
        if (rank == 0) {
            fprintf(file, "Execution time for matrix %dX%d with MPI model is equal to: %.6f seconds\n", DIM, DIM, executionTime / repetitions);
        }
        /*
            printf("Matriz A (%d x %d):\n", DIM, DIM);
            printMatrix(A);
            printf("Matriz B (%d x %d):\n", DIM, DIM);
            printMatrix(B);
            printf("Matriz C (%d x %d):\n", DIM, DIM);
            printMatrix(C);
        }
        */

        // Sincronização entre processos antes de seguir para o próximo teste
        MPI_Barrier(MPI_COMM_WORLD);
    }

    if (rank == 0) {
        fclose(file); // Fecha o arquivo
    }

    MPI_Finalize();
    return 0;
}
