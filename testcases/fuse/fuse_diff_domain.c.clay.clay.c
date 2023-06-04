a = 0;
for (i=0;i<=min(M,N);i++) {
  a[i] = 0;
  t[i] = 0;
  s[i] = 0;
}
for (i=max(0,M+1);i<=N;i++) {
  a[i] = 0;
}
for (i=max(0,N+1);i<=M;i++) {
  t[i] = 0;
  s[i] = 0;
}
f = 0;
