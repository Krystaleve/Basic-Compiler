int printf(char *, ...);

int hello(int (*func)(char *, ...), char *name) {
    int (*func2)(char *, ...);
    // (lvalue = rvalue) = rvalue
    (func2 = 0) = func;
    func2("he\x6c\154o, %s!\n", name);
}


int main() {
    int sscanf(char *, char *, ...);
    char buffer[80];
    sscanf("\r\t\n  world", "%s", buffer);
    hello(printf, buffer);
    return 0;
}
