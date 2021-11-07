int putchar(int c);
int main() {
  for (int i = 0; i < 10; i++) {
    if (i < 3) continue;
    putchar('a' + i);
  }
}
