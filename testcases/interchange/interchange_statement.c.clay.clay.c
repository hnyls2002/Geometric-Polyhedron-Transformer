if ((M >= 0) && (N >= 0) && (P >= 0)) {
  for (i=0;i<=min(N,P);i++) {
    for (j=0;j<=M;j++) {
      for (k=0;k<=min(N,P);k++) {
        a[i][j][k] = 0;
        b[k][j][i] = 0;
        c[i][j][k] = 0;
      }
      for (k=N+1;k<=P;k++) {
        a[i][j][k] = 0;
        c[i][j][k] = 0;
      }
      for (k=P+1;k<=N;k++) {
        b[k][j][i] = 0;
      }
    }
  }
  for (i=P+1;i<=N;i++) {
    for (j=0;j<=M;j++) {
      for (k=0;k<=P;k++) {
        a[i][j][k] = 0;
        c[i][j][k] = 0;
      }
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
