
char *itoa(int i) {
    static char str[12];
    sprintf(str, "%i", i);
    return strdup(str);
}