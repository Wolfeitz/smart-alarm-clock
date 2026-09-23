# Fresh-session bootstrap evidence

Date: 2026-09-23. Runtime: installed Codex CLI 0.156.1, fresh ephemeral session,
read-only sandbox, approval policy never, existing model/runtime configuration.
No previous conversation was passed. This proves the native CLI session path;
a separately launched desktop UI session was not tested. No independent internal
loader trace was captured; the fresh model identified supplied instructions and
confirmed files on disk, then exercised the installed operating model.

## Observed evidence

Fresh session ID: `01a0ceea-0831-7f81-8c74-1fb854e8dbe5`. CLI process exit: 0.

The initial probe discovered guidance but declined execution because its prompt
prohibited reading the brief even for hashing. The corrected fresh prompt allowed
internal integrity hashing only, retaining the prohibition on loading reference
text into model context. No project permission or verifier change was needed.

Completed tool commands in corrected probe:

- `/bin/bash -lc "pwd; rg --files -g AGENTS.md -g README.md -g PROJECT.md -g WORK.md -g TESTING.md -g verify-bootstrap.py -g '"'!docs/bootstrap/**'"' -g '"'!docs/reference/**'"'; if [ -n \""'$CODEX_HOME" ]; then printf '"'CODEX_HOME=%s\\n' \""'$CODEX_HOME"; fi'` — exit 0.
- `/bin/bash -lc 'cat AGENTS.md README.md docs/PROJECT.md docs/WORK.md scripts/verify-bootstrap.py'` — exit 0.
- `/bin/bash -lc 'cat /home/rob/.codex/AGENTS.md docs/TESTING.md'` — exit 0.
- `/bin/bash -lc 'python scripts/verify-bootstrap.py'` — exit 0.

Canonical command output:

```text
PASS: documentation verification only; artifacts, index, source hash and sections 1-27.
Firmware build, device behavior and fresh-session discovery are not verified by this check.
```

The command trace contains no read of the canonical AAE document, proposal or
archived brief text into model context. The deterministic verifier hashes the
archived brief internally. No installer, SDK or device operation was executed.

## Fresh-session report

The read-only bootstrap probe passed.

- **Instructions identified:** machine guidance at [/home/rob/.codex/AGENTS.md](/home/rob/.codex/AGENTS.md) and project guidance at [AGENTS.md](/mnt/data_3tb/Projects3/esp-link/AGENTS.md). Both were supplied in session context and confirmed on disk; this is not an independent runtime loader trace.
- **Operating model:** Standard, moderate cost; local-first alarm clock, concise Markdown guidance and deterministic verification. Firmware is not implemented.
- **Current bounded task:** approved bootstrap only, per `docs/WORK.md`.

Exact command, executed from `/mnt/data_3tb/Projects3/esp-link`:

```sh
python scripts/verify-bootstrap.py
```

**Exit status: `0`**

```text
PASS: documentation verification only; artifacts, index, source hash and sections 1-27.
Firmware build, device behavior and fresh-session discovery are not verified by this check.
```

The verifier checks artifact presence, index links, archived brief integrity and section coverage. It does not prove semantic correctness, secret hygiene, firmware compilation or hardware behavior. This session separately demonstrates guidance discovery and command execution.

No files were modified, software installed, devices or network services accessed, or installer executed. Excluded documents were not loaded; the verifier read brief bytes internally for hashing only.

**Next:** record this evidence and reconcile the remaining bootstrap checklist. The next project phase is hardware readiness, not yet started or authorized by this probe.
## Local validation

- Documentation verifier passed in the installing session.
- Temporary-copy fault checks rejected a broken index link, modified source bytes
  and missing section 27.
- AGENTS.md matches the approved draft exactly.
- Preserved installer SHA-256: `8033f0bc484e863ad274645db2e90e3ac7dd17c35c53de4589653850140344bf`.
- Source-brief fingerprint matches the supplied local bytes.
- Staged whitespace review passed, with original source bytes preserved verbatim.
- No remote configured; no firmware or physical-device validation performed.
