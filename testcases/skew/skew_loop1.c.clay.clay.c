if (N >= 0) {
  for (i=-2*M;i<=-1;i++) {
    for (j=ceild(-i,2);j<=min(floord(-i+N,2),M);j++) {
      a[i+2*j][j] = 0;
    }
  }
  if (M <= -1) {
    for (i=0;i<=N;i++) {
      b[i] = 0;
    }
  }
  if (M >= 0) {
    for (i=0;i<=N;i++) {
      for (j=0;j<=min(floord(-i+N,2),M);j++) {
        a[i+2*j][j] = 0;
      }
      b[i] = 0;
    }
  }
}
