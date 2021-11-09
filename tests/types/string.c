int puts(char s[1]);
int main() {
  char hello[10] = "hello world";
  puts(hello);
  hello[0] = 'a';
  puts(hello);
  return 0;
}
