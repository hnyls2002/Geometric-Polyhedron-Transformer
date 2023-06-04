a = 0;
if ((M >= 0) && (P >= 0)) {
  for (i=0;i<=N;i++) {
    b[i] = 0;
    for (j=0;j<=P;j++) {
      d[i][j] = 0;
    }
    for (j=0;j<=M;j++) {
      c[i][j] = 0;
    }
  }
}
if ((M >= 0) && (P <= -1)) {
  for (i=0;i<=N;i++) {
    b[i] = 0;
    for (j=0;j<=M;j++) {
      c[i][j] = 0;
    }
  }
}
if ((M <= -1) && (P >= 0)) {
  for (i=0;i<=N;i++) {
    b[i] = 0;
    for (j=0;j<=P;j++) {
      d[i][j] = 0;
    }
  }
}
if ((M <= -1) && (P <= -1)) {
  for (i=0;i<=N;i++) {
    b[i] = 0;
  }
}
