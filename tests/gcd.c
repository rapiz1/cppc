int gcd(int a, int b) {
  if (b == 0) return a;
  while (a >= b) a = a - b;
  return gcd(b, a);
}

int main() {}
