if ((M >= 0) && (N >= 0)) {
  for (i=0;i<=min(N,P);i++) {
    for (j=0;j<=M;j++) {
      a[i][j] = 0;
      for (k=0;k<=N;k++) {
        b[k][j][i] = 0;
      }
    }
  }
  for (i=max(0,P+1);i<=N;i++) {
    for (j=0;j<=M;j++) {
      a[i][j] = 0;
    }
  }
  for (i=N+1;i<=P;i++) {
    for (j=0;j<=M;j++) {
      for (k=0;k<=N;k++) {
        b[k][j][i] = 0;
      }
    }
  }
}
