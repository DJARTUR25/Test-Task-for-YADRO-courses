#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MISSED_NAME_OF_FILES 10

// функция обработки чисел, которые больше 255 или меньше 0
uint8_t correct_number(int num) {
    if (num > 255) {
        return (uint8_t)(num % 251);
    }
    else if (num < 0) {
        int abs_num = -num;
        return (uint8_t)(abs_num % 241);
    }
    return (uint8_t)num;
}

// функция матричной свертки
void convolution(const uint8_t* input, int H, int W, const int8_t* kernel, int DH, int DW, uint8_t* output) {
    // центр ядра
    int mid_DH = DH / 2;
    int mid_DW = DW / 2;


    for (int i = 0; i < H; i++) {       // внешний цикл по строкам исходной матрицы
        for (int j = 0; j < W; j++) {   // внешний цикл по столбцам исходной матрицы
            
            int sum = 0;    // переменная для сохранения суммы

            for (int ker_i = -mid_DH; ker_i <= mid_DH; ker_i++) {       // внутренний цикл по строке ядра свертки
                for (int ker_j = -mid_DW; ker_j <= mid_DW; ker_j++) {   // внутренний цикл по столбцу ядра

                    // ячейка, на которую попадает сейчас ядро
                    int x = i + ker_i; 
                    int y = j + ker_j;  

                    // проверка, лежит ли эта ячейка внутри границ матрицы
                    if ((x >= 0) && (x < H) && (y >= 0) && (y < W)) {
                        int input_num = input[x * W + y];
                        int kernel_num = kernel[(ker_i + mid_DH) * DW + (ker_j + mid_DW)];
                        sum += (input_num * kernel_num);
                    }
                }
            }
            // сохранение результата в выходной массив
            output[i * W + j] = correct_number(sum);
        }
    }
}

// мейн
int main(int argc, char* argv[]) {
    char* in_name = NULL;
    char* out_name = NULL;

    // разбор аргументов, подсчет их количества
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-i") == 0) && (i + 1 < argc)) {
            in_name = argv[i + 1];
        }
        if ((strcmp(argv[i], "-o") == 0) && (i + 1 < argc)) {
            out_name = argv[i + 1];
        }
    }

    // если не найдены имена файлов, то завершение программы (10)
    if ((!in_name) || (!out_name)) {
        printf("To launch print: %s -o output.dat -i input.bin\n", argv[0]);
        return MISSED_NAME_OF_FILES;
    }

    // открытие файлов
    FILE* input_file = fopen(in_name, "rb"); // read, binary
    if (!input_file) {
        printf("Cannot open input file \n");
        return 1;
    }

    // чтение значений H и W
    uint32_t H, W;
    fread(&H, 4, 1, input_file);
    fread(&W, 4, 1, input_file);
    
    int N = H * W; // количество элементов в матрицах A, B, C

    // выделение памяти для матриц A, B, C
    uint8_t* A = (uint8_t*)malloc(N);
    uint8_t* B = (uint8_t*)malloc(N);
    uint8_t* C = (uint8_t*)malloc(N);

    // запись чисел в матрицы из инпут файла
    for (int i = 0; i < N; i++) {
        fread(&A[i], 1, 1, input_file);
        fread(&B[i], 1, 1, input_file);
        fread(&C[i], 1, 1, input_file);
    }

    // чтений значений DH и DW
    uint16_t DH, DW;
    fread(&DH, 2, 1, input_file);
    fread(&DW, 2, 1, input_file);
    
    int DN = DH * DW; // количество элементов в матрице D
    int8_t* D = (int8_t*)malloc(DN); // выделение памяти для матрицы D

    fread(D, 1, DN, input_file);    // запись чисел в матрицу D из инпут файла

    fclose(input_file); // закрытие инпут файла

    // выделение памяти для результирующих матриц
    uint8_t* A_out = (uint8_t*)malloc(N);
    uint8_t* B_out = (uint8_t*)malloc(N);
    uint8_t* C_out = (uint8_t*)malloc(N);

    // выполнение свертки для каждой матрицы с ядром D
    convolution(A, H, W, D, DH, DW, A_out);
    convolution(B, H, W, D, DH, DW, B_out);
    convolution(C, H, W, D, DH, DW, C_out);

    // открытие выходного файла
    FILE* output_file = fopen(out_name, "wb"); // write, binary
    if (!output_file) {
        printf("Cannot open output file\n");
        return 1;
    }

    // сначала запись высоты и ширины
    fwrite(&H, 4, 1, output_file);
    fwrite(&W, 4, 1, output_file);

    // запись результатов свертки
    for (int i = 0; i < N; i++) {
        fwrite(&A_out[i], 1, 1, output_file);
        fwrite(&B_out[i], 1, 1, output_file);
        fwrite(&C_out[i], 1, 1, output_file);
    }

    // закрытие файла
    fclose(output_file);

    // освобождение памяти
    free(A);
    free(B);
    free(C);
    free(D);
    free(A_out);
    free(B_out);
    free(C_out);
    
    return 0;
}