#pragma once

#include "zeroerr/assert.h"
#ifdef ZEROERR_ENABLE_BENCHMARK
#include "zeroerr/benchmark.h"
#endif
#ifdef ZEROERR_ENABLE_FUZZTEST
#include "zeroerr/fuzztest.h"
#endif
#ifdef ZEROERR_ENABLE_TABLE
#include "zeroerr/table.h"
#endif

#include "zeroerr/color.h"
#include "zeroerr/dbg.h"
#include "zeroerr/log.h"
#include "zeroerr/print.h"
#include "zeroerr/unittest.h"
#ifdef ZEROERR_ENABLE_STD_ADAPTER
#include "zeroerr/std_adapter.h"
#endif
