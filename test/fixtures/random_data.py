#!/usr/bin/python2

import numpy as np

n = 1000
m = 6

X = np.random.randn(n, m)
scale = np.tile(np.random.randn(m), (n, 1)) * 100
intercept = np.tile(np.random.randn(m), (n, 1)) * 100

noise = np.matrix(np.random.normal(20, 5, n)).T

X = np.multiply(X, scale) + intercept
w = np.random.randn(m, 1) * 10
y = np.dot(X, w) + noise

np.set_printoptions(threshold=np.nan)
print np.hstack([X, y])
print np.squeeze(np.asarray(w))
