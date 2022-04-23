# oneko

Neko runs over the windows

## Getting Started

### Prerequisites

- C standard library (e.g. glibc)
- X11 client-side library (libX11)
- X11 miscellaneous extensions library (libXext)

### Building

The CMake Build system is unused because there is only one source file that includes all headers. You can install the program using make, or produce the `oneko` executable using the command below.

```sh
cc -Wno-parentheses -std=c11 -pedantic -D_DEFAULT_SOURCE oneko.c -o oneko -lc -lm -lX11 -lXext
```

### Usage

Read the [docs](doc).

## Versioning

Patch levels are used for versions compatible with the original oneko. The project will switch to [SemVer](https://semver.org/) for versioning after the first feature-breaking change.

## Authors

* Original program "xneko" written by Masayuki Koba
* Modified to program "oneko" by Tatsuya Kato
* Modified furthermore by
  - John Lerchey
  - Eric Anderson
  - Toshihiro Kanda
  - Kiichiroh Mukose

## License

According to [FSF Directory][1], the (Japanese) README file, as well as the LSM file originally distributed with oneko on sunsite both say that oneko is public domain software.

[1]: https://directory.fsf.org/wiki/Oneko#tab=Details
