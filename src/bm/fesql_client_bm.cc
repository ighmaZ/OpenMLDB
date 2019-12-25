/* Compile with:
 *
 * cc multi_threaded_inserts.c -lmysqlclient -pthread -o mti
 */

#include "llvm/Support/TargetSelect.h"
#include "benchmark/benchmark.h"
#include "bm/fesql_client_bm_case.h"
#include "glog/logging.h"

using namespace ::llvm;  // NOLINT
namespace fesql {
namespace bm {

static void BM_SIMPLE_QUERY(benchmark::State &state) {  // NOLINT
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    int64_t record_size = state.range(0);
    SIMPLE_CASE1_QUERY(&state, BENCHMARK, record_size);
}



static void BM_WINDOW_CASE1_QUERY(benchmark::State &state) {  // NOLINT
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    int64_t record_size = state.range(0);
    ::fesql::bm::WINDOW_CASE1_QUERY(&state, BENCHMARK, false, record_size);
}
static void BM_WINDOW_CASE1_BATCH_QUERY(benchmark::State &state) {  // NOLINT
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    int64_t record_size = state.range(0);
    ::fesql::bm::WINDOW_CASE1_QUERY(&state, BENCHMARK, true, record_size);
}

// BENCHMARK(BM_SIMPLE_INSERT);
// BENCHMARK(BM_INSERT_WITH_INDEX);
BENCHMARK(BM_SIMPLE_QUERY)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_WINDOW_CASE1_QUERY)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_WINDOW_CASE1_BATCH_QUERY)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
// BENCHMARK(BM_INSERT_SINGLE_THREAD);
}  // namespace bm
};  // namespace fesql

BENCHMARK_MAIN();
