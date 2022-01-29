int
binsearch(double* a, size_t n, double v)
{
  int ret;
  int l;
  int r;
  int i;
  double x;

  ret = -1;
  l   = 0;
  r   = n - 1;

  while (r >= l) {
    i = (l + r) / 2;
    x = a[i];

    if (x < v) {
      l = i + 1;
      continue;
    } 
    
    if (x > v) {
      r = i - 1;
      continue;
    }

    ret = i;
    break;
  }

  return ret;
}
