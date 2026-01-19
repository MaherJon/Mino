#include <stdarg.h>
#include <string.h>
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

//Initialize system modules
void initSystem() 
{
    //Initialize print mod
    sys.IO.print.PrintInt = PrintIntImpl;
    sys.IO.print.PrintFloat = PrintFloatImpl;
    sys.IO.print.PrintDouble = PrintDoubleImpl;
    sys.IO.print.PrintString = PrintStringImpl;
    sys.IO.print.println = printlnImpl;
    sys.IO.print.printObject = printObjectImpl;

    //Initialize scanner mod
    sys.IO.scanner.scanInt = scanIntImpl;        // void scanInt(int*)
    sys.IO.scanner.scanFloat = scanDoubleImpl;    // void scanFloat(float*)
    sys.IO.scanner.scanDouble = scanDoubleImpl;  // void scanDouble(double*)
    sys.IO.scanner.scanString = scanStringImpl;  // void scanString(char*, int)
    
    sys.IO.scanner.inputInt = inputIntImpl;      // int inputInt(const char*)
    sys.IO.scanner.inputFloat = inputFloatImpl;  // float inputFloat(const char*)
    
    sys.IO.scanner.readLine = readLineImpl;      // void readLine(char*, int)
}