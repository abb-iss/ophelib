# Introduction to API
A brief overview of the API with examples is presented here.

Load data:

```
const string data_file = "test/fixtures/x17.txt";
NTL::Mat<float> X;
NTL::Vec<float> y;
Vector::load_data(data_file, X, y);
```

Generate Keys:

```
PaillierFast pai(2048);
pai.generate_keys();
```

Normalize and integerize:

```
Vector::Normalizer normX, normY;
Vector::Integerizer inter(30); // 30 bits precision

auto const X_tr = inter.transform(normX.fit_transform(X));
auto const y_tr = inter.transform(normY.fit_transform(y));
```

Encrypt/decrypt a vector/matrix:

```
auto const X_enc = Vector::encrypt(X_tr, pai);
auto const X_dec = Vector::decrypt(X_enc, pai);
```

Deintegerize and denormalize:

```
auto const X_orig = normX.inverse_transform(inter.inverse_transform(X_dec));
```
