# Pebbit

Pebbit is a PebbleOS client for Reddit. It is built from the Pebblegram watch UI shell and Pebble Teams card/list patterns, with a Reddit-shaped phone backend.

## Current Status

- Browse Front Page, Hot, Trending, Saved, pinned subreddits, and subscribed subreddits.
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

Each user should create their own Reddit installed app and paste that app's client ID into Pebbit settings. Do not add a client secret; Reddit installed apps are public clients and Pebbit only uses the client ID.

Use this redirect URI for the hosted callback:

```text
https://tombolger.github.io/Pebbit/reddit-callback.html
```

Local fallback:

```text
http://localhost:65010/pebbit
```

Pebbit settings accept a bare authorization code, `code=...`, a full redirected URL, or copied text with the trailing `#_`.

Advanced settings also expose OAuth scopes and User-Agent. Forks or published variants should keep the User-Agent descriptive for that build.

## GitHub Pages

Set Pages to publish from the `main` branch, `/docs` folder. The callback page lives at `docs/reddit-callback.html`.
