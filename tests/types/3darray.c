int putchar(int c);
void printInt(int x) {
  if (!x) {
    putchar('0');
    return;
  }
  int a[10];
  int sz = 0;
  while (x) {
    a[sz] = x%10;
    sz++;
    x = x/10;
  }
  for (int i = sz-1; i >= 0; i--) {
    putchar(a[i] + '0');
  }
  return;
}
int main() {
  int a[3][3][3];
  int c = 0;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
    for (int k = 0; k < 3; k++) {
      a[i][j][k] = c++;
    }
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) for (int k = 0; k < 3; k++) {
      printInt(a[i][j][k]);
      putchar(' ');
    }
    putchar('\n');
  }
  return 0;
}
