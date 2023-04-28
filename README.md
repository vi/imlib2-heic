imlib2-heic
---

Loader for [HEIC][1] for *old version* Imlib2 (i.e. for [Feh][2]). Based on libheif.

Note that new imlib2 may have heif loader built-in, hence may not require this project to open heic images.

There is a pre-built amd64 deb package on [Github releases](https://github.com/vi/imlib2-heic/releases/).

Building and deploying
---

On Debian amd64:

1. `apt-get install libimlib2-dev libheif-dev pkg-config build-essential`
2. `make`
3. Copy `libheic.so` to `/usr/lib/x86_64-linux-gnu/imlib2/loaders`

Licence
---

imlib2-heic is BSD (3-clause), but it depends on libheif which is LGPL.


[1]:https://nokiatech.github.io/heif/technical.html
[2]:http://feh.finalrewind.org/
