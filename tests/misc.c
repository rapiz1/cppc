int putchar(int c);
void printInt(int x) {
  int a[10];
  int sz = 0;
  while (x) {
    a[sz] = x % 10;
    sz++;
    x = x / 10;
  }
  for (int i = sz - 1; i >= 0; i--) {
    putchar(a[i] + '0');
  }
  return;
}

int gcd(int a, int b) {
  if (b == 0) return a;
  a = a % b;
  return gcd(b, a);
}

void showGcd() {
  for (int i = 90; i < 100; i++)
    for (int j = 90; j < 100; j++) {
      printInt(i);
      putchar(' ');
      printInt(j);
      putchar(' ');
      printInt(gcd(i, j));
      putchar('\n');
    }
  return;
}

int main() {
  showGcd();
  return 0;
}
