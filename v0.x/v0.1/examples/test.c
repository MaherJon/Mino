#include <System.h>

int main() {
    initSystem();
    
    sys.IO.print.println("Mino System Test");
    sys.IO.print.println("================");
    
    // 测试 scanInt
    int age;
    sys.IO.print.PrintString("Enter age (using scanInt): ");
    sys.IO.scanner.scanInt(&age);
    sys.IO.print.println("Age is: %d", age);  // 使用 println 输出格式化
    
    // 测试 inputInt
    int height = sys.IO.scanner.inputInt("Enter height (using inputInt): ");
    sys.IO.print.println("Height is: %d cm", height);  // 使用 println
    
    // 测试 inputFloat
    float weight = sys.IO.scanner.inputFloat("Enter weight (kg): ");
    sys.IO.print.println("Weight is: %.2f kg", weight);  // 使用 println 格式化
    
    // 或者使用 PrintFloat（但只能输出纯数值）
    sys.IO.print.PrintString("Weight is: ");
    sys.IO.print.PrintFloat(weight);
    sys.IO.print.println(" kg");  // 添加换行
    
    // 测试字符串输入
    char name[100];
    sys.IO.print.PrintString("Enter name: ");
    sys.IO.scanner.scanString(name, sizeof(name));
    sys.IO.print.println("Hello, %s!", name);
    
    // 测试 readLine
    char line[256];
    sys.IO.print.println("Enter a sentence: ");
    sys.IO.scanner.readLine(line, sizeof(line));
    sys.IO.print.println("You entered: %s", line);
    
    return 0;
}