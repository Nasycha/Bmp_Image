#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#define no_argument 0 
#define required_argument 1

#define bmp_format_id 0x4d42

#pragma pack (push, 1)

typedef struct {
    unsigned short signature;
    unsigned int filesize;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int pixelArrOffset;
} BitmapFileHeader;

typedef struct {
    unsigned int headerSize;
    unsigned int width;
    unsigned int height;
    unsigned short planes;
    unsigned short bitsPerPixel;
    unsigned int compression;
    unsigned int imageSize;
    unsigned int xPixelsPerMeter;
    unsigned int yPixelsPerMeter;
    unsigned int colorsInColorTable;
    unsigned int importantColorCount;
} BitmapInfoHeader;

typedef struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
} Rgb;

#pragma pack(pop)

void help() {
    printf(
        "Course work for option 4.1, created by Anastasiya Milkhert."
        "\nДля запука программы ипользуются аргументы в следующем порядке: 'programm_name -flag1 -flag2 -flag3 ... input_file_name\n'"
        "Функции программы:\n"
        "help - Вывод справоной информации о работе программы\n\n"
        "info - Вывод информации об изображении\n\n"

        "circle - Рисование окружности\n"
        "флаги:\n"
        "center - Координаты центра окружности в формате `x.y`\n"
        "radius - Радиус окружности, больше 0\n"
        "thickness - Толщина линий окружности, больше 0\n"
        "color - Цвет линии окружности, задается строко й формата `rrr.ggg.bbb`\n"
        "fill - Флаг, который подается при наличии заливки\n"
        "fill_color - Цвет заливки при ее наличии, задается строкой формата `rrr.ggg.bbb`\n\n"

        "mirror - Отражение заданной области\n"
        "флаги:\n"
        "axis - Выбор оси относительно которой отражать, возможные значения 'x' и 'y'\n"
        "left_up - Координаты левого верхнего угла области, задается в формате 'x.y'\n"
        "right_down - Координаты левого верхнего угла области, задается в формате 'x.y'\n\n"

        "copy - Копирование заданной области\n"
        "флаги:\n"
        "left_up - Координаты левого верхнего угла области, задается в формате 'x.y'\n"
        "right_down - Координаты левого верхнего угла области, задается в формате 'x.y'\n"
        "dest_left_up - Координаты левого верхнего угла области-назначения, задается в формате 'x.y'\n"
    );
}

int checkCoordinates(int left_x, int up_y, int right_x, int down_y) {
    if (left_x >= right_x) {
        return 1;
    }
    if (up_y >= down_y) {
        return 1;
    }
    return 0;
}


int check_bmp(char file_name[], BitmapFileHeader* bmfh, BitmapInfoHeader* bmif) {
    FILE* f = fopen(file_name, "rb");
    fread(bmfh, 1, sizeof(BitmapFileHeader), f);
    fread(bmif, 1, sizeof(BitmapInfoHeader), f);
    if (!f) {
        fprintf(stderr, "Не удалось открыть файл\n");
        fclose(f);
        exit(45);
    }
    if (bmfh->signature != bmp_format_id) {
        fprintf(stderr, "Файл должен иметь формат bmp\n");
        fclose(f);
        exit(45);
    }
    if (bmif->compression != 0) {
        fprintf(stderr, "Изображение должно быть без сжатия\n");
        fclose(f);
        fclose(f);
        exit(45);
    }
    // if (bmif->headerSize != 40) {
    //     fprintf(stderr, "Изображение старой версии, не поддерживается\n");
    //     fclose(f);
    //     exit(45);
    // }
    if (bmif->bitsPerPixel != 24) {
        fprintf(stderr, "Изображение должно иметь 24 бита на пиксель\n");
        fclose(f);
        exit(45);
    }
    if (bmif->width > 50000 || bmif->height > 50000) {
        fprintf(stderr, "Изображение слишком большое\n");
        fclose(f);
        exit(45);
    }
    fclose(f);
    return 1;
}


void printFileHeader(BitmapFileHeader* bmfh) {
    printf("Информация о BMP-файле:\n");
    printf("signature:\t%x (%hx)\n", bmfh->signature, bmfh->signature);
    printf("filesize:\t%x (%u)\n", bmfh->filesize, bmfh->filesize);
    printf("reserved1:\t%x (%hx)\n", bmfh->reserved1, bmfh->reserved1);
    printf("reserved2:\t%x (%hx)\n", bmfh->reserved2, bmfh->reserved2);
    printf("pixelArrOffset:\t%x (%u)\n", bmfh->pixelArrOffset, bmfh->pixelArrOffset);
}

void printInfoHeader(BitmapInfoHeader* bmif) {
    printf("headerSize:\t%x (%u)\n", bmif->headerSize, bmif->headerSize);
    printf("width:     \t%x (%u)\n", bmif->width, bmif->width);
    printf("height:    \t%x (%u)\n", bmif->height, bmif->height);
    printf("planes:    \t%x (%hx)\n", bmif->planes, bmif->planes);
    printf("bitsPerPixel:\t%x (%hx)\n", bmif->bitsPerPixel, bmif->bitsPerPixel);
    printf("compression:\t%x (%u)\n", bmif->compression, bmif->compression);
    printf("imageSize:\t%x (%u)\n", bmif->imageSize, bmif->imageSize);
    printf("xPixelsPerMeter:\t%x (%u)\n", bmif->xPixelsPerMeter, bmif->xPixelsPerMeter);
    printf("yPixelsPerMeter:\t%x (%u)\n", bmif->yPixelsPerMeter, bmif->yPixelsPerMeter);
    printf("colorsInColorTable:\t%x (%u)\n", bmif->colorsInColorTable, bmif->colorsInColorTable);
    printf("importantColorCount:\t%x (%u)\n", bmif->importantColorCount, bmif->importantColorCount);
}

Rgb** read_bmp(char file_name[], BitmapFileHeader* bmfh, BitmapInfoHeader* bmif) {
    FILE* f = fopen(file_name, "rb");
    fread(bmfh, 1, sizeof(BitmapFileHeader), f);
    fread(bmif, 1, sizeof(BitmapInfoHeader), f);
    unsigned int Height = bmif->height;
    unsigned int Width = bmif->width;
    unsigned int padding = (Width * sizeof(Rgb)) % 4;
    if (padding) padding = 4 - padding;
    Rgb** arr = (Rgb**)malloc(Height * sizeof(Rgb*));
    for (int i = Height - 1; i >= 0; i--) {
        arr[i] = (Rgb*)malloc(Width * sizeof(Rgb) + padding);
        fread(arr[i], 1, Width * sizeof(Rgb) + padding, f);
    }
    fclose(f);
    return arr;
}

void write_bmp(char new_file[], BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, Rgb** arr) {
    FILE* f = fopen(new_file, "wb");
    fwrite(bmfh, 1, sizeof(BitmapFileHeader), f);
    fwrite(bmif, 1, sizeof(BitmapInfoHeader), f);
    unsigned int Height = bmif->height;
    unsigned int Width = bmif->width;
    size_t padding = (4 - (Width * sizeof(Rgb)) % 4) % 4;
    for (int i = Height - 1; i >= 0; i--) {
        fwrite(arr[i], 1, Width * sizeof(Rgb) + padding, f);
    }
    fclose(f);
}

Rgb check_color(int r, int g, int b) {
    if (r > 255 || g > 255 || b > 255 || r < 0 || g < 0 || b < 0) {
        fprintf(stdout, "Значения цвета должны быть в диапазоне от 0 до 255.\n");
        exit(45);
    }
    Rgb color;
    color.r = (unsigned char)r;
    color.g = (unsigned char)g;
    color.b = (unsigned char)b;
    return color;
}

void set_color(Rgb** arr, int x, int y, Rgb color) {
    arr[y][x].r = color.r;
    arr[y][x].g = color.g;
    arr[y][x].b = color.b;
}

int maximum(int a, int b) {
    return a > b ? a : b;
}

int minimum(int a, int b) {
    return a < b ? a : b;
}

int check_on_circle_line(int x, int y, int x0, int y0, int radius, int thickness) {
    int flag1 = (x - x0) * (x - x0) + (y - y0) * (y - y0) <= (radius + (thickness) / 2) * (radius + (thickness) / 2);
    int flag2 = (x - x0) * (x - x0) + (y - y0) * (y - y0) >= (maximum(0, radius - (thickness) / 2)) * (maximum(0, radius - (thickness) / 2));
    return flag1 && flag2;
}

int check_in_circle(int x, int y, int x0, int y0, int radius, int thickness) {
    int flag = (x - x0) * (x - x0) + (y - y0) * (y - y0) <= (radius - thickness / 2) * (radius - thickness / 2);
    return flag;
}

void drawCircle(BitmapInfoHeader* bmif, int x, int y, int r, int thickness, Rgb** arr, Rgb color, int fill, Rgb f_col) {
    int width = bmif->width;
    int height = bmif->height;

    for (int y0 = maximum(0, y - r - thickness / 2); y0 <= minimum(height - 1, y + r + thickness / 2); y0++) {
        for (int x0 = maximum(0, x - r - thickness / 2); x0 <= minimum(width - 1, x + r + thickness / 2); x0++) {
            if (fill && check_in_circle(x0, y0, x, y, r, thickness)) {
                set_color(arr, x0, y0, f_col);
            }
            if (check_on_circle_line(x0, y0, x, y, r, thickness)) {
                set_color(arr, x0, y0, color);
            }
        }
    }
}


// Отражение области
void set_area(int height, int width, int* x0, int* y0, int* x1, int* y1) {
    *x0 = *x0 < 0 ? 0 : (*x0 >= width ? width - 1 : *x0);
    *x1 = *x1 < 0 ? 0 : (*x1 >= width ? width - 1 : *x1);
    *y0 = *y0 < 0 ? 0 : (*y0 >= height ? height - 1 : *y0);
    *y1 = *y1 < 0 ? 0 : (*y1 >= height ? height - 1 : *y1);
}


int check_in_pic(int width, int height, int x, int y) {
    return (x >= 0 && x < width && y >= 0 && y < height);
}


void swap(Rgb** arr, int x1, int y1, int x2, int y2) {
    Rgb temp = arr[y1][x1];
    arr[y1][x1] = arr[y2][x2];
    arr[y2][x2] = temp;
}


void mirror_part(Rgb** arr, int x0, int y0, int x1, int y1, BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, char axis) {
    unsigned int H = bmif->height;
    unsigned int W = bmif->width;

    // Ограничиваем координаты в пределах изображения
    set_area(H, W, &x0, &y0, &x1, &y1);

    if (y0 > y1 || x0 > x1 || x1 >= W || y1 >= H || x0 < 0 || y0 < 0) {
        puts("Введены некорректные координаты.\n");
        exit(45);
    }

    if (axis == 'x') {
        int diff = x1 - x0;
        for (int y = y0; y <= y1; y++) {
            for (int x = x0; x <= x0 + (diff / 2); x++) {
                swap(arr, x, y, x0 + diff - (x - x0), y);
            }
        }
    } else if (axis == 'y') {
        int diff = y1 - y0;
        Rgb* color = (Rgb*)malloc(sizeof(Rgb) * (x1 - x0 + 1));

        for (int x = x0; x <= x1; x++) {
            color[x - x0] = arr[y1][x];
        }

        for (int y = y0; y <= y0 + (diff / 2); y++) {
            for (int x = x0; x <= x1; x++) {
                swap(arr, x, y, x, y0 + diff - (y - y0));
            }
        }

        for (int x = x0; x <= x1; x++) {
            arr[y1][x] = color[x - x0];
        }

        free(color);
    } else {
        puts("Неверная ось. Используйте 'x' или 'y'.\n");
        exit(45);
    }
}

void copy_area(Rgb** arr, int x0, int y0, int x1, int y1, int x2, int y2, BitmapFileHeader* bmfh, BitmapInfoHeader* bmif) {
    unsigned int H = bmif->height;
    unsigned int W = bmif->width;

    // Ограничиваем координаты в пределах изображения
    set_area(H, W, &x0, &y0, &x1, &y1);

    if (y0 > y1 || x0 > x1 || x2 < 0 || x2 >= W || y2 < 0 || y2 >= H) {
        puts("Введены некорректные координаты.\n");
        exit(45);
    }

    int diff_x = x2 - x0;
    int diff_y = y2 - y0;

    // Вычисляем лимиты для копирования
    int lim_x = (x2 + (x1 - x0 + 1) > W) ? W : x2 + (x1 - x0 + 1);
    int lim_y = (y2 + (y1 - y0 + 1) > H) ? H : y2 + (y1 - y0 + 1);

    // Копируем данные во временный буфер
    Rgb** arr_buf = malloc((y1 - y0 + 1) * sizeof(Rgb*));
    for (int i = 0; i <= y1 - y0; i++) {
        arr_buf[i] = malloc((x1 - x0 + 1) * sizeof(Rgb));
        for (int j = 0; j <= x1 - x0; j++) {
            arr_buf[i][j] = arr[y0 + i][x0 + j];
        }
    }

    // Копируем данные из временного буфера в целевой регион
    for (int i = y2; i < lim_y; i++) {
        for (int j = x2; j < lim_x; j++) {
            arr[i][j] = arr_buf[i - y2][j - x2];
        }
    }

    // Освобождаем память, выделенную под временный буфер
    for (int i = 0; i <= y1 - y0; i++) {
        free(arr_buf[i]);
    }
    free(arr_buf);  
}

void contrast(Rgb** arr, BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, float alpha, int beta) {
    if (alpha <= 0) {
        printf("Error: Alpha must be greater than 0\n");
        exit(45);
    }

    unsigned int height = bmif->height;
    unsigned int width = bmif->width;

    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            int r = (int)round(alpha * arr[i][j].r + beta);
            int g = (int)round(alpha * arr[i][j].g + beta);
            int b = (int)round(alpha * arr[i][j].b + beta);

            // Проверка и ограничение значений в диапазоне 0-255
            if (r < 0) r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            // Присваивание новых значений пикселя
            arr[i][j].r = (unsigned char)r;
            arr[i][j].g = (unsigned char)g;
            arr[i][j].b = (unsigned char)b;
        }
    }
}


int main(int argc, char* argv[]) {
    char* input_file_name = NULL;
    char* output_file_name = "out.bmp";

    // значения флагов
    int output_flag = 0, input_flag = 0, info_flag = 0;
    int circle_flag = 0, center_flag = 0, radius_flag = 0, thickness_flag = 0, color_flag = 0, fill_flag = 0, fill_color_flag = 0;
    int mirror_flag = 0, axis_flag = 0, left_up_flag = 0, right_down_flag = 0;
    int copy_flag = 0, dest_left_up_flag = 0;
    // для курсовой
    int contrast_flag = 0, alpha_flag = 0, beta_flag = 0;
    float alpha = 0.0;
    int beta = 0;

    int radius, x_center, y_center, x_left_up, y_left_up, x_right_down, y_right_down, r, g, b, thickness, x_dest_left_up, y_dest_left_up;
    char axis;

    Rgb color_circle, fill_color;

    int opt;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"output", required_argument, 0, 'o'},
        {"input", required_argument, 0, 'i'},
        {"info", no_argument, 0, 'I'},

        {"circle", no_argument, 0, 'C'},
        {"center", required_argument, 0, 'c'},
        {"radius", required_argument, 0, 'r'},
        {"thickness", required_argument, 0, 't'},
        {"color", required_argument, 0, 'l'},
        {"fill", no_argument, 0, 'f'},
        {"fill_color", required_argument, 0, 'F'},

        {"mirror", no_argument, 0, 'M'},
        {"axis", required_argument, 0, 'a'},
        {"left_up", required_argument, 0, 'L'},
        {"right_down", required_argument, 0, 'R'},

        {"copy", no_argument, 0, 'P'},
        {"left_up", required_argument, 0, 'L'},
        {"right_down", required_argument, 0, 'R'},
        {"dest_left_up", required_argument, 0, 'D'},

        {"contrast", no_argument, 0, 'X'},
        {"alpha", required_argument, 0, 'x'},
        {"beta", required_argument, 0, 'z'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "ho:i:ICc:r:t:l:fF:Ma:L:R:PD:Xx:z:", long_options, NULL)) != -1) {
        switch (opt) {
            // Дефолтные флаги
            case 'h':
                help();
                return 0;
            case 'o':
                output_flag++;
                output_file_name = optarg;
                break;
            case 'i':
                input_flag++;
                input_file_name = optarg;
                break;
            case 'I':
                info_flag++;
                break;

                // Рисование круга
            case 'C':
                circle_flag++;
                break;
            case 'c':
                center_flag++;
                if (sscanf(optarg, "%d.%d", &x_center, &y_center) != 2) {
                    fprintf(stdout, "Неверный формат координат. Используйте формат x.y.\n");
                    exit(45);
                }
                break;
            case 'r':
                radius_flag++;
                radius = atoi(optarg);
                if (radius <= 0) {
                    fprintf(stdout, "Неверный радиус\n");
                    exit(45);
                }
                break;
            case 't':
                thickness_flag++;
                thickness = atoi(optarg);
                if (thickness <= 0) {
                    fprintf(stdout, "Неверная толщина линии\n");
                    exit(45);
                }
                break;
            case 'l':
                color_flag++;
                if (sscanf(optarg, "%d.%d.%d", &r, &g, &b) != 3) {
                    fprintf(stdout, "Неверный формат цвета. Используйте формат rrr.ggg.bbb.\n");
                    exit(45);
                }
                color_circle = check_color(r, g, b);
                break;
            case 'f':
                fill_flag++;
                break;
            case 'F':
                fill_color_flag++;
                if (sscanf(optarg, "%d.%d.%d", &r, &g, &b) != 3) {
                    fprintf(stdout, "Неверный формат цвета. Используйте формат rrr.ggg.bbb.\n");
                    exit(45);
                }
                fill_color = check_color(r, g, b);
                break;

                // Отражение области
            case 'M':
                mirror_flag++;
                break;
            case 'a':
                axis_flag++;
                axis = optarg[0];
                if (axis != 'x' && axis != 'y') {
                    fprintf(stdout, "Неверная ось. Используйте 'x' или 'y'.\n");
                    exit(45);
                }
                break;

                // Копирование области
            case 'P':
                copy_flag++;
                break;
            case 'D':
                dest_left_up_flag++;// хз мб еще + проверка на правильность координат
                if (sscanf(optarg, "%d.%d", &x_dest_left_up, &y_dest_left_up) != 2) {
                    fprintf(stdout, "Неверный формат координат. Используйте формат x.y.\n");
                    exit(45);
                }
                break;

                // Координаты для отражения и копирования области
            case 'L':
                left_up_flag++;
                if (sscanf(optarg, "%d.%d", &x_left_up, &y_left_up) != 2) {
                    fprintf(stdout, "Неверный формат координат. Используйте формат x.y.\n");
                    exit(45);
                }
                break;
            case 'R':
                right_down_flag++;
                if (sscanf(optarg, "%d.%d", &x_right_down, &y_right_down) != 2) {
                    fprintf(stdout, "Неверный формат координат. Используйте формат x.y.\n");
                    exit(45);
                }
                break;
            // Курсовая
            case 'X':
                contrast_flag++;
                break;
            case 'x':
                alpha_flag++;
                alpha = atof(optarg);
                if (alpha <= 0) {
                    fprintf(stdout, "Неверное значение\n");
                    exit(45);
                }
                break;
            case 'z':
                beta_flag++;
                beta = atoi(optarg);
                if (beta <= 0) {
                    fprintf(stdout, "Неверное значение\n");
                    exit(45);
                }
                break;

        default:
            fprintf(stdout, "Неверный флаг.\n");
            exit(45);
        }
    }

    // Проверки флагов
    if ((circle_flag > 0 && copy_flag > 0) || (circle_flag > 0 && mirror_flag > 0) || (copy_flag > 0 && mirror_flag > 0) || (circle_flag > 0 && copy_flag > 0 && mirror_flag > 0)) {
        fprintf(stdout, "Нельзя использовать несколько функций вместе.\n");
        exit(45);
    }

    if (circle_flag > 1 || center_flag > 1 || radius_flag > 1 || thickness_flag > 1 || color_flag > 1 || fill_color_flag > 1 || fill_flag > 1 || mirror_flag > 1 || axis_flag > 1 || copy_flag > 1 || dest_left_up_flag > 1 || left_up_flag > 1 || right_down_flag > 1) {
        fprintf(stdout, "Флаги могут быть использованы только один раз\n");
        exit(45);
    }

    if (optind < argc && input_file_name == NULL) {
        if (strcmp(argv[optind], output_file_name) == 0) {
            fprintf(stdout, "Имя входного файла и имя выходного файла не должны совпадать.\n");
            exit(45);
        }
        input_file_name = argv[optind];
    }

    if (input_file_name == NULL) {
        fprintf(stdout, "Входной файл не указан.\n");
        exit(45);
    }

    if (output_file_name == NULL) {
        fprintf(stdout, "Выходной файл не указан.\n");
        exit(45);
    }

    // Память для структуры
    BitmapFileHeader* bmfh = malloc(sizeof(BitmapFileHeader));
    BitmapInfoHeader* bmif = malloc(sizeof(BitmapInfoHeader));

    // Вывод информации о bmp
    if (info_flag) {
        printFileHeader(bmfh);
        printInfoHeader(bmif);
        return 0;
    }

    // Обработка флагов и вызов функций 
    if (check_bmp(input_file_name, bmfh, bmif)) {
        Rgb** arr = read_bmp(input_file_name, bmfh, bmif);
        unsigned int H = bmif->height;
        unsigned int W = bmif->width;

        // Рисование круга
        if (circle_flag) {
            if (circle_flag && !(center_flag && radius_flag && thickness_flag && color_flag)) {
                fprintf(stdout, "Необходимо указать все параметры для флага --circle.\n");
                exit(45);
            }
            if (circle_flag && (axis || left_up_flag || right_down_flag || dest_left_up_flag)) {
                fprintf(stdout, "Указаны лишние параметры\n");
                exit(45);
            }
            if (circle_flag && (fill_flag && !fill_color_flag)) {
                fprintf(stdout, "Необходимо указать цвет заливки для окружности.\n");
                exit(45);
            }
            if (fill_flag && fill_color_flag) {
                int fill = 1;
            }

            drawCircle(bmif, x_center, y_center, radius, thickness, arr, color_circle, fill_flag, fill_color);

        }

        //Отражение области
        if (mirror_flag) {
            if (mirror_flag && !(axis_flag && left_up_flag && right_down_flag)) {
                fprintf(stdout, "Необходимо указать все параметры для флага --mirror.\n");
                exit(45);
            }
            if (circle_flag && (center_flag || radius_flag || thickness_flag || color_flag || fill_color_flag || fill_flag || dest_left_up_flag)) {
                fprintf(stdout, "Указаны лишние параметры\n");
                exit(45);
            }
            if (checkCoordinates(x_left_up, y_left_up, x_right_down, y_right_down) == 1) {
                fprintf(stdout, "Введены неправильные координаты\n");
                exit(45);
            }
            if (axis == 'x' || axis == 'y') {
                mirror_part(arr, x_left_up, y_left_up, x_right_down, y_right_down, bmfh, bmif, axis);
            }
            else {
                fprintf(stdout, "Некорректное значение для флага --axis. Используйте 'x' или 'y'.\n");
                exit(45);
            }
        }

        // Копирование области
        if (copy_flag) {
            if (copy_flag && !(left_up_flag && right_down_flag && dest_left_up_flag)) {
                fprintf(stdout, "Необходимо указать все параметры для флага --copy.\n");
                exit(45);
            }
            if (checkCoordinates(x_left_up, y_left_up, x_right_down, y_right_down) == 1) {
                fprintf(stdout, "Введены неправильные координаты\n");
                exit(45);
            }
            if (circle_flag && (center_flag || radius_flag || thickness_flag || color_flag || fill_color_flag || fill_flag || axis_flag)) {
                fprintf(stdout, "Указаны лишние параметры\n");
                exit(45);
            }
            copy_area(arr, x_left_up, y_left_up, x_right_down, y_right_down, x_dest_left_up, y_dest_left_up, bmfh, bmif);
        }
        // задание на защиту

        if (contrast_flag) {
            contrast(arr, bmfh, bmif, alpha, beta);
        }
        // Записать изображение в новый файл 
        write_bmp(output_file_name, bmfh, bmif, arr);
        for (int i = 0; i < bmif->height; i++) {
            free(arr[i]);
        }
        free(arr);
    }

    free(bmfh);
    free(bmif);

    return 0;
}
