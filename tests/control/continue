int putchar(int c);
void printInt(int x) {
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
  int i = 0;
  while (i < 10) {
    i++;
    if (i < 3) continue;
    printInt(i);
  } 
}
