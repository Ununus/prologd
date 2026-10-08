#pragma once

#include <iosfwd>

namespace prologd_cli {

// A line-delimited JSON session, the front-end protocol of `prologd --protocol
// json`.  Requests arrive on `in`, one JSON object per line; events go out on
// `out`, one JSON object per line:
//
// | Request | Meaning |
// |---------|---------|
// | `{"cmd":"load","path":"prog.pld"}` | run a file |
// | `{"cmd":"load","text":"...","name":"buffer"}` | run program text |
// | `{"cmd":"query","text":"?goal."}` | run one statement |
// | `{"cmd":"input","text":"..."}` | answer the pending `input_request` |
// | `{"cmd":"quit"}` | end the session |
//
// Events: `ready`, `output`, `result`, `input_request`, `error`, `loaded`, `bye`.
// The engine of this implementation draws nothing (its graphics predicates are
// no-ops), does not step and does not format, so the requests that carry those
// features — `step`, `format`, `graphics`, `interrupt` — and the `trace`,
// `screen`, `limits` and `vars` fields are accepted and ignored, and their events
// are never emitted.
//
// This mirrors `prologd2`'s protocol (`src/cli/protocol.cpp`), but the engine here
// speaks CP1251: text leaving it is converted to UTF-8 and text coming in is
// converted back, so the front-end always sees UTF-8.
//
// Returns the process exit code: 0 clean, 2 a load failure, 3 a query failure.
int run_json_session(std::istream &in, std::ostream &out);

}  // namespace prologd_cli
