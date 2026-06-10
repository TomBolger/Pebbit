# Agent Notes

This workspace is `/mnt/c/Users/tombo/Documents/Pebbit`. Planning is complete and implementation has started.

## Goal

Build Pebbit, a personal PebbleOS client for Reddit, by reusing proven code from Pebblegram and Pebble Teams. The app should read feeds/subreddits/posts/comments and support comment-level actions, but should not support creating a new post in the first version.

## Donor Projects

Use these local sources first:
- Pebblegram: `/mnt/c/Users/tombo/Documents/Pebblegram/main-release`
- Pebble Teams: `/mnt/c/Users/tombo/Documents/Pebble Teams`

Important state:
- Pebblegram `main-release` is a git repo but was dirty during planning: `plan.md`, `src/c/Pebblegram.c`, `src/pkjs/index.js`, and `src/pkjs/pgjs/image.js` had local modifications, plus `tall-test-photo.jpg` was untracked.
- Pebble Teams did not appear to be a git repo at its local root.
- Do not modify either donor project unless the user explicitly asks. Copy from them into Pebbit when implementation begins.

## Reusable Pieces

Pebblegram:
- Watch chat list and message bubbles.
- Sender names, reply/forward context, metadata row, reactions.
- Dictation, canned replies, emoji replies, Emery keyboard path.
- Native/custom action menu patterns.
- Serialized AppMessage queue.
- Lazy image transfer/decode and phone-side image preparation.

Pebble Teams:
- Planner list modes and card-style rows.
- Card detail bubble rendering.
- Checklist-like selectable rows, useful as a model for comment depth and "load more" rows.
- Interaction locking and custom action sheet lifecycle fixes.
- Warnings from prior crashes: serialize backend actions, avoid stacking dictation/menu transitions, keep media unloaded during complex actions.

## Implementation Principles

- Keep the first build small: mock UI, then read-only Reddit, then actions, then media.
- Preserve AppMessage serialization. Do not send listing rows, comment rows, image chunks, avatars, and statuses in parallel.
- Flatten Reddit comments on the phone side. The watch should receive normalized rows with depth, author, body, metadata, and parent quote text.
- Prefer short-lived caches. Reddit's policy requires deleted content and author-identifying data to be removed; avoid durable content storage.
- Keep a clear boundary between UI and API modules.
- Add only the message keys Pebbit really needs.
- Do not expose `/api/submit` for post creation in v1. Comment replies via `/api/comment` are allowed by the product scope.
- Use a descriptive User-Agent and respect Reddit rate-limit response headers.

## Expected File Shape After Coding Starts

Likely seed files:
- `package.json`
- `wscript`
- `src/c/Pebbit.c` or initially `src/c/Pebblegram.c`
- `src/pkjs/index.js`
- `src/pkjs/config.html` or Clay settings files
- `src/pkjs/reddit/auth.js`
- `src/pkjs/reddit/client.js`
- `src/pkjs/reddit/listings.js`
- `src/pkjs/reddit/comments.js`
- `src/pkjs/reddit/actions.js`
- `src/pkjs/reddit/media.js`
- `src/pkjs/reddit/mock.js`
- `resources/images/menu_icon.png`

## Reddit Notes

Checked current docs during planning:
- Reddit Data API free access for eligible usage documents 100 queries per minute per OAuth client ID.
- OAuth or login credentials are required; unauthenticated traffic is blocked from the default rate-limit model.
- Non-commercial/personal use is plausible, but Reddit still requires sign-up/eligibility and policy compliance.
- Commercial use, ads, paywalls, bulk export, AI training, and misleading Reddit trademark use are out of scope.

Useful endpoints:
- Listings: `/best`, `/hot`, `/rising`, `/top`, `/new`, `/r/{subreddit}/{sort}`.
- Comments: `/r/{subreddit}/comments/{article}`.
- More comments: `/api/morechildren`.
- Reply: `/api/comment`.
- Vote: `/api/vote`.
- Save/unsave: `/api/save`, `/api/unsave`.
- Edit own text: `/api/editusertext`.
- Subreddits: `/subreddits/mine/subscriber`, `/subreddits/popular`, `/r/{subreddit}/about`.

Likely scopes:
- `identity`, `read`, `mysubreddits`, `vote`, `save`, `submit`, `edit`.

## UX Notes

- Top-level list should include Front Page, Hot, Trending, Saved, and Subreddits. Implement Trending as Rising or Popular unless the user chooses otherwise.
- Feed rows should use the chat list style.
- Subreddits should use Planner card styling.
- Post/comment detail should use chat bubbles.
- Comment depth should be visible with small indents and vertical colored lines.
- Comment actions should be reachable from select/long press and should include reply, vote, save, edit own comment.
- Media loading should be explicit on selected item first, not automatic.

## Verification Later

Current implementation state:
- The app is seeded from local Pebble Teams, which already carried the Pebblegram watch shell plus Planner/card surfaces.
- App identity is Pebbit, UUID `f54ed9a8-12db-4282-92c5-536f1190e73f`.
- `src/pkjs/reddit/backend.js` provides the Reddit/mock backend boundary.
- `src/pkjs/config.html` is a Pebbit-for-Reddit settings fallback; the backend also returns an inline settings page.
- Active watch paths should not expose post creation. Compatibility `create_task` paths remain fail-closed and should not be wired into visible UI.
- `src/c/Pebblegram.c` is intentionally still named after the donor app for now.
- Mock mode is explicit: unchecked saves as `pebbit.mockMode = "0"`; it still falls back to mock if no client ID exists.
- Default Reddit redirect URI is `https://tombolger.github.io/Pebbit/reddit-callback.html`; static callback source is `docs/reddit-callback.html`. The app also accepts the old localhost fallback and strips `#_` from pasted codes/URLs.
- Reddit media preview extraction now feeds both post-list thumbnail loading and opened-post selected media loading.
- Watch UI uses Reddit orange/yellow colors; comment detail renders row 0 as a top post card and comments as left-aligned indented rows with orange rails.

Verification:
- `node --check src/pkjs/index.js`
- `node --check src/pkjs/reddit/backend.js`
- `pebble build` completed successfully and produced `build/Pebbit.pbw`.

Next verification:
- Run mock mode in an emulator before touching Reddit auth.
- Test Basalt/Diorite/Emery/Gabbro memory-sensitive paths if target platforms remain broad.
- Test auth with a personal Reddit app/client ID.
- Confirm no create-post UI or backend route is exposed.
