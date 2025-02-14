#include <stdio.h>
#include <stdlib.h>
#include <complex>
#include <cmath>
#include <png.h>
#include <omp.h>
#include <unistd.h>

#define A -1.00
#define B 0.16
#define C 1.97
#define D -2.31

#define WIDTH 1980
#define HEIGHT 1080
#define MAX_ITER 2000
#define ESCAPE_RADIUS 4.0

double p(double phi) {
    return A * phi * phi + B * phi + C;
}

std::complex<double> T(double r, double phi) {
    return std::pow(r, D) * std::exp(std::complex<double>(0, p(phi)));
}

void generate_fractal(png_bytep *row_pointers) {
    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < HEIGHT; y++) {
        png_bytep row = row_pointers[y];
        for (int x = 0; x < WIDTH; x++) {
            double total_red = 0, total_green = 0, total_blue = 0;
            int samples = 4;

            double scale_x = 4.0 / WIDTH;
            double scale_y = 4.0 / HEIGHT;
            double offset_x = (x - WIDTH / 2.0) * scale_x;
            double offset_y = (y - HEIGHT / 2.0) * scale_y;

            for (int sy = 0; sy < 2; sy++) {
                for (int sx = 0; sx < 2; sx++) {
                    double zx = offset_x + (sx + 0.5) * scale_x / 2.0;
                    double zy = offset_y + (sy + 0.5) * scale_y / 2.0;
                    std::complex<double> z(zx, zy);
                    std::complex<double> c = z;

                    int iter;
                    for (iter = 0; iter < MAX_ITER; iter++) {
                        double r = abs(z);
                        double phi = arg(z);

                        z = T(r, phi) + c;

                        if (abs(z) > ESCAPE_RADIUS) {
                            break;
                        }
                    }

                    png_byte red, green, blue;
                    if (iter == MAX_ITER) {
                        red = green = blue = 0;
                    } else {
                        red = (iter * 15) % 256;
                        green = (iter * 3) % 256;
                        blue = (iter * 5) % 256;
                    }

                    total_red += red;
                    total_green += green;
                    total_blue += blue;
                }
            }

            row[x * 3] = total_red / samples;
            row[x * 3 + 1] = total_green / samples;
            row[x * 3 + 2] = total_blue / samples;
        }
    }
}

int main() {
    // Print current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    }

    // Create a PNG file
    FILE *fp = fopen("heart_fractal.png", "wb");
    if (!fp) {
        // Print more detailed error message
        perror("Could not open file for writing");
        return 1;
    }

    printf("Successfully opened file for writing\n");

    // Initialize PNG writing
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fprintf(stderr, "Could not create PNG write struct\n");
        fclose(fp);
        return 1;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        fprintf(stderr, "Could not create PNG info struct\n");
        png_destroy_write_struct(&png, NULL);
        fclose(fp);
        return 1;
    }

    // Set up error handling
    if (setjmp(png_jmpbuf(png))) {
        fprintf(stderr, "Error during PNG creation\n");
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return 1;
    }

    // Initialize PNG IO
    png_init_io(png, fp);

    // Set PNG header
    png_set_IHDR(png, info, WIDTH, HEIGHT, 8,
                 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    // Allocate memory for the image
    png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * HEIGHT);
    for (int y = 0; y < HEIGHT; y++) {
        row_pointers[y] = (png_byte*)malloc(WIDTH * 3);
    }

    printf("Starting fractal generation...\n");
    generate_fractal(row_pointers);
    printf("Fractal generation complete\n");

    printf("Writing PNG file...\n");
    png_write_image(png, row_pointers);
    png_write_end(png, NULL);
    printf("PNG file written\n");

    // Clean up
    for (int y = 0; y < HEIGHT; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
    png_destroy_write_struct(&png, &info);
    fclose(fp);

    printf("Heart fractal has been generated as '%s/heart_fractal.png'\n", cwd);
    return 0;
}
