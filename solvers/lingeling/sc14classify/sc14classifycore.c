#include "sc14classify.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

static int initialized;

static int rel (int a, int b) {
  if (!b) return 0;
  return (a * 100l) / (long) b;
}

static int ld (int a) {
  int res;
  if (a == 0) return 0;
  for (res = 1; res < 31 && a > (1 << (res - 1)); res++)
    ;
  return res;
}

static void init_row (Row * r) {
  r->vl = ld (r->vc);
  r->cl = ld (r->cc);
  r->vr = rel (r->vc, r->vo);
  r->cr = rel (r->cc, r->co);
  r->br = rel (r->b, r->cc);
  r->tr = rel (r->t, r->cc);
  r->qr = rel (r->q, r->cc);
  r->c1r = rel (r->c1, r->cc);
  r->c2r = rel (r->c2, r->cc);
  r->c3r = rel (r->c3, r->cc);
  r->c4r = rel (r->c4, r->cc);
}

static const int ntrain = sizeof (train) / sizeof (train[0]);

static void init () {
  int i;
  for (i = 0; i < ntrain; i++) init_row (train + i);
  initialized = 1;
}

#define FIRST vo
#define LAST K

#define MAXK 11

static long dist (Row * r, Row * s, Row * w) {
  const int * p, * q, * v;
  long res = 0, a, b, d, tmp;
  q = &s->FIRST;
  v = &w->FIRST;
  for (p = &r->FIRST; p < &r->LAST; p++, q++, v++) {
    assert (q < &s->LAST);
    assert (v < &w->LAST);
    if (!(tmp = *v)) continue;
    a = *p, b = *q;
    d = tmp * labs (a - b);
    res += d*d;
  }
  assert (q == &s->LAST);
  assert (v == &w->LAST);
  return res;
}

static int match (Row * r, Row * classifier) {
  int f = 0, c[MAXK], i, j, csum, res;
  const int K = classifier->K;
  long d[MAXK], tmp;
  for (i = 0; i < K; i++) d[i] = c[i] = 0;
  for (i = 0; i < ntrain; i++) {
    Row * s = train + i;
    if (s == r) continue;
    tmp = dist (r, s, classifier);
    for (j = 0; j < f; j++)
      if (d[j] > tmp) break;
    if (j >= K) continue;
    d[j] = tmp;
    c[j] = !strcmp (s->bucket, classifier->bucket);
    if (j == f) f++;
  }
  csum = 0;
  for (i = 0; i < f; i++) csum += c[i];
  res = csum >= f/2;
  return res;
}

static const int nclassifiers = sizeof (classifiers) / sizeof (classifiers[0]);

const char *
sc14classify  (
  int vo, int vc, int co, int cc,
  int b, int t, int q, int c1, int c2, int c3, int c4,
  int x, int a1, int a2, int g, int j, int c, int o) {
  int i, res = -1;
  Row r;
  if (!initialized) init ();
  memset (&r, 0, sizeof (r));
  r.vo  = vo;
  r.vc  = vc;
  r.co  = co;
  r.cc  = cc;
  r.b   = b;
  r.t   = t;
  r.q   = q;
  r.c1  = c1;
  r.c2  = c2;
  r.c3  = c3;
  r.c4  = c4;
  r.x   = x;
  r.a1  = a1;
  r.a2  = a2;
  r.g   = g;
  r.j   = j;
  r.c   = c;
  r.o   = o;
  init_row (&r);
  for (i = 0; i < nclassifiers; i++) {
    if (match (&r, classifiers + i)) {
      if (res < 0) res = i;
      else return "unknown";
    }
  }
  return res < 0 ? "unknown" : classifiers[res].bucket;
}
