//===================================================
//                Mino.System.h
//===================================================

#ifndef SYSTEM_H
#define SYSTEM_H

#include<stdio.h>
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

#endif 