int putchar(int c);
int main() {
  for (int i = 0; i < 10; i++) {
    if (i < 3) continue;
    char c = '0' + i;
    if (i % 2 == 0) c = 'e';
    if (i == 8) break;
    putchar(c);
  }
}
