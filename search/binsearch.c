int
binsearch(double* a, size_t n, double v)
{
  int l;
  int r;
  int ret;

  l = 0;
  r = n - 1;

  while (1) {
    ret = (l + r) / 2;

    if (r <= l) break;

    if (a[ret] < v) {
      l = ret + 1;
      continue;
    } 
    
    if (a[ret] > v) {
      r = ret - 1;
      continue;
    }

    break;
  }

  return ret;
}
