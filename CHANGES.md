# Changelog

## 1.3 - Unreleased
* String: Upper/lower-casing in a specific language.

## 1.2 - 2022-03-05
* String: Added replacing/substituting with a regular expression.
* TlsCertificate: Fixed copying of X509 chains.
* TlsRequest: Added a session cache to avoid repeat handshakes.

## 1.1 - 2022-01-20
* Detect Android as a variant of Linux. Some features like starting child processes are disabled in Android.
* Block: gzip-compatible decompression.
* Char: Querying script of a Unicode code point.
* Process: Remember exit status code.

## 1.0.3
* TlsRequest: Set "valid from" time using UTC instead of local time.

## 1.0 - 2021-11-06
* Initial release.