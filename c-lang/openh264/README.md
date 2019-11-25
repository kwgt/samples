## Sample for OpenH264(https://www.openh264.org/)

サンプルをビルドする場合は、OpenH264のヘッダとライブラリファイルをincludeディレクトリとlibディレクトリに展開してビルドすること。

展開時のファイルレイアウトの例は以下の通り。

```
include
  |
  +- wels/
     |
     +- codec_api.h
     +- codec_app_def.h
     +- codec_def.h
     +- codec_ver.h

lib
  |
  +- libopenh264.a
  +- libopenh264.so@ -> libopenh264.so.5
  +- libopenh264.so.2.0.0*
  +- libopenh264.so.5@ -> libopenh264.so.2.0.0
  +- pkgconfig/
     |
     +- openh264.pc
```
