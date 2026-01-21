//===================================================
//                Mino.System.h
//===================================================

#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdio.h>
#include <stddef.h>
#include <time.h>
//====================================================
//                Mino.System.IO
//====================================================

//----------------------------------------------------
//                     print
//----------------------------------------------------
typedef struct
{
    void (*PrintInt)(int value);
    void (*PrintFloat)(float value);
    void (*PrintDouble)(double value);
    void (*PrintString)(const char* str);
    void (*println)(const char* format, ...);
    void (*PrintIntLn)(int value);
    void (*printObject)(void* obj, const char* type);

}PrintModule;

//-----------------------------------------------------
//                    scanner
//-----------------------------------------------------
typedef struct
{
    void (*scanInt)(int* target);
    void (*scanFloat)(float* target);
    void (*scanDouble)(double* target);
    void (*scanString)(char* buffer, int maxLength);
    
    int (*inputInt)(const char* prompt);
    float (*inputFloat)(const char* prompt);

    void (*readLine)(char* buffer, int size);
}ScannerModule;

typedef struct {
    PrintModule print;
    ScannerModule scanner;
} InputAndOutput;

typedef struct {
    InputAndOutput IO;
    // Additional modules can be added, for example:
    // MathModule math;
    // FileModule file;
    // NetworkModule network;
} System;

extern System sys;

void initSystem();

    // Math module (basic functions)
    typedef struct {
        double (*sin)(double);
        double (*cos)(double);
        double (*tan)(double);
        double (*sqrt)(double);
        double (*pow)(double, double);
        double (*floor)(double);
        double (*ceil)(double);
        double (*abs)(double);
    } MathModule;

    // expose math module in System
    extern MathModule mathModule; /* optional global reference */

    // C-friendly math wrappers
    double sys_sin(double v);
    double sys_cos(double v);
    double sys_tan(double v);
    double sys_sqrt(double v);
    double sys_pow(double a, double b);
    double sys_floor(double v);
    double sys_ceil(double v);
    double sys_abs(double v);

    // Integer math helpers (callable from generated code using integer ABI)
    int sys_Math_absInt(int v);
    int sys_Math_powInt(int a, int b);

// Convenience C runtime API (simplified helpers used by generated code)
// These functions are thin wrappers around the System struct above.
// Caller responsibilities follow C conventions (e.g. free buffers returned by sys_readline).
void sys_print(const char* s);
void sys_println(const char* s);
void sys_printlnf(const char* fmt, ...);
void sys_PrintStringLn(const char* s);
char* sys_readline(void); /* returned buffer must be freed by caller */
void sys_exit(int code);
void* sys_malloc(size_t n);
void sys_free(void* p);
char* sys_strdup(const char* s);
char* sys_itoa(int v);

// File I/O helpers
FILE* sys_fopen(const char* path, const char* mode);
int sys_fclose(FILE* f);
size_t sys_fread(void* ptr, size_t size, size_t nmemb, FILE* f);
size_t sys_fwrite(const void* ptr, size_t size, size_t nmemb, FILE* f);
char* sys_fgets(char* s, int size, FILE* f); // returns s or NULL
int sys_remove(const char* path);

// Time and random
double sys_time_seconds(void);
int sys_rand_int(void);
void sys_srand_seed(unsigned int seed);

#endif