# Safe Implementation Brief (Legitimate Client)

## What to build now (safe scope)

### Phase 1: Premium GUI Overhaul
- Create a consistent design system:
  - 8px spacing scale
  - Typography hierarchy (Title, Section, Body, Muted)
  - Neutral dark/light palettes with accessible contrast
  - Reusable components: primary/secondary button, card, toggle row, status pill
- UI layout standards:
  - Left rail nav + top status bar + scrollable content panels
  - Fixed action area for Apply/Reset
  - Empty/loading/error states in each panel
- Interaction polish:
  - 120-180ms transitions
  - Hover/focus states for all controls
  - Inline validation messages
  - Keyboard navigation and tooltips

### Phase 2: Stability + Hardening
- Crash prevention:
  - Guard all file/network operations with `std::error_code`/exceptions
  - Centralize recovery paths (retry/backoff/fallback)
  - Null/state checks before rendering calls
- Concurrency safety:
  - Replace shared mutable globals with synchronized state objects
  - Protect queues/maps with `std::mutex` / `std::shared_mutex`
  - Add strict ownership (`std::unique_ptr`) and lifetime boundaries
- Memory and resource safety:
  - RAII wrappers for handles/files/sockets
  - Close/release in destructors only
  - Remove hidden allocations in per-frame loops

### Logging + Error Handling
- Structured logger:
  - levels: trace/debug/info/warn/error/fatal
  - fields: timestamp, module, event_id, message, metadata JSON
- Error model:
  - typed errors (`ValidationError`, `IOError`, `NetworkError`, `InternalError`)
  - user-safe messages + internal diagnostics
- Add global exception boundary around main loop with crash report output

### Validation
- Build an input validation layer:
  - path normalization + allowlist checks
  - numeric range validation
  - enum/string strict parsing
- Return `ValidationResult { ok, code, message }` for all user-editable settings

### Tests
- Add a test harness for:
  - config parse/serialize roundtrip
  - validator edge cases
  - logger formatting + rotation behavior
  - updater manifest verification
- Include a smoke test that boots app shell and verifies startup/shutdown path

### Legit Launcher/Updater
- Launcher responsibilities:
  - validate installed version
  - fetch signed update manifest
  - compare hashes and download delta/full package
  - atomic swap + rollback on failure
- Security baseline:
  - HTTPS with certificate validation
  - signed manifest verification
  - SHA-256 package integrity check

## Suggested deliverables
1. `ClientAppTheme.*` for design tokens and theme application
2. `ClientAppShell.*` for dashboard layout
3. `AppLogger.*` for structured logging
4. `ErrorBoundary.*` for centralized error handling
5. `InputValidator.*` for validation rules
6. `LauncherUpdater.*` for legitimate updates
7. `tests/*` for unit + smoke tests

## Acceptance criteria
- GUI looks consistent and modern on 1080p and 1440p
- No uncaught exceptions during normal app usage
- Thread sanitizer/static analysis reports no high-severity issues in touched modules
- Unit tests for logger/validator/updater pass
- Launcher can detect/update/rollback safely
