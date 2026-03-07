# Dependencies

- **stb_truetype.h**: Vendored from [nothings/stb](https://github.com/nothings/stb); used for font metrics (text measurement). Public domain.

- **default_font.ttf** (optional): Place a TTF file here (e.g. [Noto Sans Regular](https://github.com/google/fonts/raw/main/ofl/notosans/NotoSans-Regular.ttf)) for accurate text measurement. If missing or invalid, the font layer uses fallback metrics (approximate width by character count) so layout still runs.
