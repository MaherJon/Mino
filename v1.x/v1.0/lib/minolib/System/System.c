#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <System.h>

//============= print =================
static void PrintIntImpl (int value)
{
    printf("%d", value);
}

static void PrintFloatImpl(float value)
{
    printf("%f", value);
}

static void PrintDoubleImpl(double value)
{
    printf("%lf", value);
}

static void PrintStringImpl(const char* str)
{
    printf("%s", str);
}

static void printlnImpl(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

static void PrintIntLnImpl(int value)
{
    printf("%d\n", value);
}

static void printObjectImpl(void* obj, const char* type) {
    printf("[%s object at %p]", type, obj);
}

//======================== scanner ============================
static void scanIntImpl(int* target) {
    scanf("%d", target);
    while (getchar() != '\n');
}

static void scanFloatImpl(float* target) {
    scanf("%f", target);
    while (getchar() != '\n');
}

static void scanDoubleImpl(double* target) {
    scanf("%lf", target);
    while (getchar() != '\n');
}

static void scanStringImpl(char* buffer, int maxLength) {
    fgets(buffer, maxLength, stdin);
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }
}

static int inputIntImpl(const char* prompt) {
    int value;
    printf("%s", prompt);
    scanf("%d", &value);
    while (getchar() != '\n');
    return value;
}

static float inputFloatImpl(const char* prompt) {
    float value;
    printf("%s", prompt);
    scanf("%f", &value);
    while (getchar() != '\n');
    return value;
}

static void readLineImpl(char* buffer, int size) {
    fgets(buffer, size, stdin);
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }
}

System sys;
MathModule mathModule;

// Initialize system modules
void initSystem() 
{
    //Initialize print mod
    sys.IO.print.PrintInt = PrintIntImpl;
    sys.IO.print.PrintFloat = PrintFloatImpl;
    sys.IO.print.PrintDouble = PrintDoubleImpl;
    sys.IO.print.PrintString = PrintStringImpl;
    sys.IO.print.println = printlnImpl;
    sys.IO.print.PrintIntLn = PrintIntLnImpl;
    sys.IO.print.printObject = printObjectImpl;

    //Initialize scanner mod
    sys.IO.scanner.scanInt = scanIntImpl;        // void scanInt(int*)
    sys.IO.scanner.scanFloat = scanDoubleImpl;    // void scanFloat(float*)
    sys.IO.scanner.scanDouble = scanDoubleImpl;  // void scanDouble(double*)
    sys.IO.scanner.scanString = scanStringImpl;  // void scanString(char*, int)
    
    sys.IO.scanner.inputInt = inputIntImpl;      // int inputInt(const char*)
    sys.IO.scanner.inputFloat = inputFloatImpl;  // float inputFloat(const char*)
    
    sys.IO.scanner.readLine = readLineImpl;      // void readLine(char*, int)

    // Initialize math module wrappers
    mathModule.sin = sin;
    mathModule.cos = cos;
    mathModule.tan = tan;
    mathModule.sqrt = sqrt;
    mathModule.pow = pow;
    mathModule.floor = floor;
    mathModule.ceil = ceil;
    mathModule.abs = fabs;
}

// Export simple wrappers for assembly-level calls (flattened dotted names)
void sys_IO_print_PrintInt(int v) { sys.IO.print.PrintInt(v); }
void sys_IO_print_PrintFloat(float v) { sys.IO.print.PrintFloat(v); }
void sys_IO_print_PrintDouble(double v) { sys.IO.print.PrintDouble(v); }
void sys_IO_print_PrintString(const char* s) { sys.IO.print.PrintString(s); }
void sys_IO_print_println(const char* s) { sys.IO.print.println(s); }
void sys_IO_print_PrintIntLn(int v) { sys.IO.print.PrintIntLn(v); }

void sys_IO_scanner_scanInt(int* p) { sys.IO.scanner.scanInt(p); }
int sys_IO_scanner_inputInt(const char* p) { return sys.IO.scanner.inputInt(p); }

// Simple C-friendly runtime wrappers
void sys_print(const char* s) { if (!s) return; sys.IO.print.PrintString(s); }
void sys_println(const char* s) { sys.IO.print.println(s ? s : ""); }
void sys_printlnf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void sys_PrintStringLn(const char* s) { if (!s) return; printf("%s\n", s); }

char* sys_readline(void) {
    char* line = NULL;
    size_t n = 0;
    ssize_t r = getline(&line, &n, stdin);
    if (r <= 0) { free(line); return NULL; }
    if (r > 0 && line[r-1] == '\n') line[r-1] = '\0';
    return line;
}

// File I/O helpers
FILE* sys_fopen(const char* path, const char* mode) {
    return fopen(path, mode);
}

int sys_fclose(FILE* f) {
    return fclose(f);
}

size_t sys_fread(void* ptr, size_t size, size_t nmemb, FILE* f) {
    return fread(ptr, size, nmemb, f);
}

size_t sys_fwrite(const void* ptr, size_t size, size_t nmemb, FILE* f) {
    return fwrite(ptr, size, nmemb, f);
}

char* sys_fgets(char* s, int size, FILE* f) {
    return fgets(s, size, f);
}

int sys_remove(const char* path) { return remove(path); }

// Time and random
double sys_time_seconds(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
    }
    return (double)time(NULL);
}

int sys_rand_int(void) { return rand(); }
void sys_srand_seed(unsigned int seed) { srand(seed); }

void sys_exit(int code) { fflush(stdout); exit(code); }

void* sys_malloc(size_t n) { return malloc(n); }
void sys_free(void* p) { free(p); }

char* sys_strdup(const char* s) {
    if (!s) return NULL;
    size_t l = strlen(s) + 1;
    char* r = (char*)malloc(l);
    if (r) memcpy(r, s, l);
    return r;
}

char* sys_itoa(int v) {
    char buf[32];
    int n = snprintf(buf, sizeof(buf), "%d", v);
    if (n < 0) return NULL;
    char* r = (char*)malloc(n + 1);
    if (!r) return NULL;
    memcpy(r, buf, n+1);
    return r;
}

// Math wrappers
double sys_sin(double v) { return mathModule.sin(v); }
double sys_cos(double v) { return mathModule.cos(v); }
double sys_tan(double v) { return mathModule.tan(v); }
double sys_sqrt(double v) { return mathModule.sqrt(v); }
double sys_pow(double a, double b) { return mathModule.pow(a,b); }
double sys_floor(double v) { return mathModule.floor(v); }
double sys_ceil(double v) { return mathModule.ceil(v); }
double sys_abs(double v) { return mathModule.abs(v); }

// Integer math helpers (use integer ABI)
int sys_Math_absInt(int v) {
    return v < 0 ? -v : v;
}

int sys_Math_powInt(int a, int b) {
    if (b < 0) return 0; // no negative exponents for int pow
    int res = 1;
    for (int i = 0; i < b; i++) res *= a;
    return res;
}

// Ensure system is initialized early when library is loaded
__attribute__((constructor)) static void __system_auto_init(void) {
    initSystem();
}