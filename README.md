# Pebbit

Pebbit is a personal PebbleOS client for Reddit. It is built from the Pebblegram watch UI shell and Pebble Teams card/list patterns, with a Reddit-shaped phone backend.

## Current Status

- Browse Front Page, Hot, Trending, Saved, and configured subreddits.
- Open posts as a top card with comments below.
- Reply/edit comments, vote, save/unsave, and load selected media.
- Mock mode is available for emulator/UI work.
- Creating new Reddit posts is intentionally not supported.

## Build

```sh
pebble build
```

The PBW is written to `build/Pebbit.pbw`.

## Reddit OAuth

Create a Reddit installed app and use this redirect URI once GitHub Pages is enabled:

```text
https://tombolger.github.io/Pebbit/reddit-callback.html
```

Local fallback:

```text
http://localhost:65010/pebbit
```

Pebbit settings accept a bare authorization code, `code=...`, a full redirected URL, or copied text with the trailing `#_`.

## GitHub Pages

Set Pages to publish from the `main` branch, `/docs` folder. The callback page lives at `docs/reddit-callback.html`.
