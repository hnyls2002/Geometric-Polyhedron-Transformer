if ((M >= 0) && (N >= 0) && (P >= 0)) {
  for (i=0;i<=N;i++) {
    for (__kk0=0;__kk0<=floord(P,32);__kk0++) {
      for (j=0;j<=M;j++) {
        for (k=32*__kk0;k<=min(P,32*__kk0+31);k++) {
          a[i][j][k] = 0;
        }
      }
    }
  }
}
