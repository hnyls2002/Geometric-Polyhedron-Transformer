if (N >= 0) {
  if (M >= 0) {
    for (i=0;i<=N;i++) {
      for (j=i;j<=i+M;j++) {
        A[i][-i+j] = 0;
      }
      B[i] = 0;
    }
  }
  if (M <= -1) {
    for (i=0;i<=N;i++) {
      B[i] = 0;
    }
  }
}
