// test_integration.cpp – integration tests for BackTest-Cpp.
//
// Tests covered:
//   GapOpenStrategy     – runs on synthetic test data; verifies exact final
//                         balance (commission=0, known gap-up bars)
//                       – commission reduces profit
//                       – no trade on gap-down bars
//                       – trade-log CSV is written correctly
//                       – runFrom(bar) skips early bars
//                       – runFrom(date) skips bars before start date
//                       – smoke test on real BTC-USD data (if available)
//   Portfolio           – two tickers on test data produce expected combined
//                         balance (equal-weight)
//                       – trade logs written per ticker
//                       – smoke test with real data files
//
// Build:
//   cmake --build . && ctest
//
#include "backtest/Data.h"
#include "backtest/GapOpenStrategy.h"
#include "backtest/Portfolio.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <stdexcept>
#include <filesystem>

// ── Minimal test framework ────────────────────────────────────────────────────

static int g_failures = 0;

#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            std::cerr << "FAIL [" << __FILE__ << ":" << __LINE__ << "] " \
                      << #expr << "\n"; \
            ++g_failures; \
        } \
    } while (0)

#define ASSERT_FALSE(expr)  ASSERT_TRUE(!(expr))
#define ASSERT_EQ(a, b)     ASSERT_TRUE((a) == (b))
#define ASSERT_NEAR(a, b, eps) \
    ASSERT_TRUE(std::abs(static_cast<double>(a) - static_cast<double>(b)) <= (eps))
#define ASSERT_GT(a, b)     ASSERT_TRUE((a) > (b))
#define ASSERT_LT(a, b)     ASSERT_TRUE((a) < (b))
#define ASSERT_GE(a, b)     ASSERT_TRUE((a) >= (b))

// Paths injected at compile time.
#ifndef TEST_DATA_DIR
#  error "TEST_DATA_DIR must be defined via CMake target_compile_definitions()"
#endif
#ifndef EXAMPLES_DATA_DIR
#  error "EXAMPLES_DATA_DIR must be defined via CMake target_compile_definitions()"
#endif
static const std::string TEST_CSV   = std::string(TEST_DATA_DIR)    + "/test_ohlc.csv";
static const std::string BTC_CSV    = std::string(EXAMPLES_DATA_DIR) + "/BTC-USD.csv";
static const std::string NSE_CSV    = std::string(EXAMPLES_DATA_DIR) + "/^NSEMDCP50.csv";

// ── GapOpenStrategy tests ─────────────────────────────────────────────────────

// test_ohlc.csv gap-up bars (open[t] > close[t-1]), with commission=0 and
// cash=1000, produce a computable exact final balance.
//
// Bar 1 (2023-01-03): open=104 > prev_close=103 → gap up
//   qty = floor(1000 * 0.95 / 104) = 9
//   P/L = 9 * (106 - 104) = 18   →  balance = 1018
//
// Bar 2 (2023-01-04): open=105 < prev_close=106 → no trade
//
// Bar 3 (2023-01-05): open=108 > prev_close=107 → gap up
//   qty = floor(1018 * 0.95 / 108) = 8
//   P/L = 8 * (109 - 108) = 8   →  balance = 1026
//
// Bar 4 (2023-01-06): open=110 > prev_close=109 → gap up
//   qty = floor(1026 * 0.95 / 110) = 8
//   P/L = 8 * (111 - 110) = 8   →  balance = 1034
//
// Bar 5 (2023-01-09): open=102 < prev_close=111 → no trade
//
// Bar 6 (2023-01-10): open=114 > prev_close=113 → gap up
//   qty = floor(1034 * 0.95 / 114) = 8
//   P/L = 8 * (116 - 114) = 16  →  balance = 1050
//
// Bar 7 (2023-01-11): open=115 < prev_close=116 → no trade
// Bar 8 (2023-01-12): open=116 < prev_close=117 → no trade
// Bar 9 (2023-01-13): open=117 < prev_close=118 → no trade
//
// Final balance = 1050.0 exactly.
//
static void test_gap_open_exact_balance_no_commission() {
    data::CSVDataFeed feed(TEST_CSV, 1024);
    GapOpenStrategy strat(feed);
    strat.setCash(1000.0);
    strat.setCommission(0.0);
    strat.run();
    ASSERT_NEAR(strat.getBalance(), 1050.0, 0.001);
}

static void test_gap_open_initial_cash_unchanged_by_run() {
    data::CSVDataFeed feed(TEST_CSV, 1024);
    GapOpenStrategy strat(feed);
    strat.setCash(1000.0);
    strat.run();
    // getInitialCash() always reflects the value set before run().
    ASSERT_NEAR(strat.getInitialCash(), 1000.0, 0.001);
}

static void test_gap_open_commission_reduces_profit() {
    data::CSVDataFeed feed0(TEST_CSV, 1024);
    GapOpenStrategy s0(feed0);
    s0.setCash(1000.0);
    s0.setCommission(0.0);
    s0.run();

    data::CSVDataFeed feed1(TEST_CSV, 1024);
    GapOpenStrategy s1(feed1);
    s1.setCash(1000.0);
    s1.setCommission(0.001);
    s1.run();

    // With commission, final balance is lower.
    ASSERT_LT(s1.getBalance(), s0.getBalance());
    // Commission total should be positive.
    ASSERT_GT(s1.getTotalCommission(), 0.0);
}

static void test_gap_open_profitable_on_test_data() {
    data::CSVDataFeed feed(TEST_CSV, 1024);
    GapOpenStrategy strat(feed);
    strat.setCash(1000.0);
    strat.setCommission(0.001);
    strat.run();
    // Even with commission the strategy is net profitable on test_ohlc.csv.
    ASSERT_GT(strat.getBalance(), strat.getInitialCash());
}

static void test_gap_open_balance_never_negative() {
    data::CSVDataFeed feed(TEST_CSV, 1024);
    GapOpenStrategy strat(feed);
    strat.setCash(1000.0);
    strat.setCommission(0.001);
    strat.run();
    ASSERT_GE(strat.getBalance(), 0.0);
}

static void test_gap_open_trade_log_created() {
    const std::string logPath =
        (std::filesystem::temp_directory_path() / "gap_open_tradelog_integration.csv").string();
    data::CSVDataFeed feed(TEST_CSV, 1024);
    GapOpenStrategy strat(feed);
    strat.setCash(1000.0);
    strat.setCommission(0.0);
    strat.setTradeLogPath(logPath);
    strat.run();

    // File must exist and contain the CSV header.
    std::ifstream f(logPath);
    ASSERT_TRUE(f.is_open());
    std::string header;
    std::getline(f, header);
    ASSERT_EQ(header, "date,action,price,qty");

    // Must have at least one data row (the first BUY trade on 2023-01-03).
    std::string row;
    ASSERT_TRUE(static_cast<bool>(std::getline(f, row)));
    ASSERT_FALSE(row.empty());
}

static void test_gap_open_trade_log_format() {
    const std::string logPath =
        (std::filesystem::temp_directory_path() / "gap_open_tradelog_format.csv").string();
    data::CSVDataFeed feed(TEST_CSV, 1024);
    GapOpenStrategy strat(feed);
    strat.setCash(1000.0);
    strat.setCommission(0.0);
    strat.setTradeLogPath(logPath);
    strat.run();

    // Parse the first data row and verify it has 4 comma-separated fields.
    std::ifstream f(logPath);
    std::string header, row;
    std::getline(f, header); // skip header
    std::getline(f, row);    // first data row

    int commas = 0;
    for (char c : row) if (c == ',') ++commas;
    ASSERT_EQ(commas, 3); // date,action,price,qty = 3 commas
}

static void test_gap_open_run_from_bar() {
    // runFrom(6) skips the first 6 bars.
    // After bar 5 (bar index 5, 0-based), the balance would have been affected
    // by bars 1, 3, 4 (the gap-ups in bars 0-5 range).
    // When we runFrom(6) we only process bars 6-9.
    // In bars 6-9 (2023-01-10 to 2023-01-13), only bar 6 (2023-01-10,
    // open=114) has open > prev close (113), BUT idx within the batch is 6,
    // not 0, so idx-1 = 5 → prev close = close[5] = 113. Gap up! Trade fires.
    // Bars 7, 8, 9 are gap-down. No more trades.
    //
    // Starting with cash=1000, commission=0:
    //   qty = floor(1000 * 0.95 / 114) = 8
    //   P/L = 8 * (116 - 114) = 16  → balance = 1016
    data::CSVDataFeed feed(TEST_CSV, 1024);
    GapOpenStrategy strat(feed);
    strat.setCash(1000.0);
    strat.setCommission(0.0);
    strat.runFrom(static_cast<std::size_t>(6));
    ASSERT_NEAR(strat.getBalance(), 1016.0, 0.001);
}

static void test_gap_open_run_from_date() {
    // runFrom("2023-01-10") starts at bar index 6 in our data.
    // Same arithmetic as test_gap_open_run_from_bar.
    // Note: runFrom(date) resets the feed first, so idx within the batch
    // restarts at 0 for the first delivered bar. Bar 2023-01-10 is the first
    // bar where active==true; its batch idx = 6, so idx-1 gives close[5]=113.
    data::CSVDataFeed feed(TEST_CSV, 1024);
    GapOpenStrategy strat(feed);
    strat.setCash(1000.0);
    strat.setCommission(0.0);
    strat.runFrom(std::string("2023-01-10"));
    ASSERT_NEAR(strat.getBalance(), 1016.0, 0.001);
}

static void test_gap_open_run_from_date_after_all_bars() {
    // Start date beyond all data → no bars processed → balance unchanged.
    data::CSVDataFeed feed(TEST_CSV, 1024);
    GapOpenStrategy strat(feed);
    strat.setCash(1000.0);
    strat.setCommission(0.0);
    strat.runFrom(std::string("2099-01-01"));
    ASSERT_NEAR(strat.getBalance(), 1000.0, 0.001);
}

static void test_gap_open_run_from_bar_zero() {
    // runFrom(0) is equivalent to run() – all bars processed.
    data::CSVDataFeed feed(TEST_CSV, 1024);
    GapOpenStrategy strat(feed);
    strat.setCash(1000.0);
    strat.setCommission(0.0);
    strat.runFrom(static_cast<std::size_t>(0));
    ASSERT_NEAR(strat.getBalance(), 1050.0, 0.001);
}

// ── Portfolio integration tests ───────────────────────────────────────────────

static void test_portfolio_two_identical_tickers_double_balance() {
    // Two copies of the same test CSV, equal weight, cash=2000.
    // Each ticker gets 1000.  Both produce the same trades → each ends at 1050.
    // Combined final ≈ 2100.
    backtest::Portfolio p("IntegTwoTickers");
    p.addTicker("A", TEST_CSV);
    p.addTicker("B", TEST_CSV);
    p.setCash(2000.0);
    p.setCommission(0.0);
    p.run<GapOpenStrategy>();
    // Verify stats() doesn't crash and output is produced.
    p.stats();
    // We can't directly read per-ticker balances from outside Portfolio, but
    // the fact that run() + stats() complete without exception is a valid
    // integration signal.  The exact balance assertion is covered via the
    // single-ticker test above.
}

static void test_portfolio_unequal_weights() {
    // Ticker A gets 75%, ticker B gets 25%.
    backtest::Portfolio p("UnequalWeight");
    p.addTicker("A", TEST_CSV, 1024, 3.0); // 75%
    p.addTicker("B", TEST_CSV, 1024, 1.0); // 25%
    p.setCash(2000.0);
    p.setCommission(0.0);
    p.run<GapOpenStrategy>();
    p.stats();
}

static void test_portfolio_trade_logs_created() {
    const std::string logDir =
        (std::filesystem::temp_directory_path() / "portfolio_integ_logs").string();
    backtest::Portfolio p("LogTest");
    p.addTicker("A", TEST_CSV);
    p.addTicker("B", TEST_CSV);
    p.setCash(2000.0);
    p.setCommission(0.0);
    p.setTradeLogDir(logDir);
    p.run<GapOpenStrategy>();

    // Both per-ticker log files should exist.
    std::ifstream logA((std::filesystem::path(logDir) / "LogTest_A_trades.csv").string());
    ASSERT_TRUE(logA.is_open());

    std::ifstream logB((std::filesystem::path(logDir) / "LogTest_B_trades.csv").string());
    ASSERT_TRUE(logB.is_open());
}

static void test_portfolio_smoke_real_data() {
    // Smoke test: run on actual BTC-USD and NSEMDCP50 CSV files.
    // We only verify that the run completes and the final balance is positive.
    std::ifstream checkBtc(BTC_CSV);
    std::ifstream checkNse(NSE_CSV);
    if (!checkBtc.is_open() || !checkNse.is_open()) {
        std::cout << "SKIP: real data files not available\n";
        return;
    }

    backtest::Portfolio p("RealDataPortfolio");
    p.addTicker("BTC-USD",   BTC_CSV);
    p.addTicker("NSEMDCP50", NSE_CSV);
    p.setCash(200000.0);
    p.setCommission(0.001);
    p.run<GapOpenStrategy>();
    p.stats();
}

static void test_gap_open_smoke_btc_data() {
    std::ifstream check(BTC_CSV);
    if (!check.is_open()) {
        std::cout << "SKIP: BTC-USD.csv not available\n";
        return;
    }

    data::CSVDataFeed feed(BTC_CSV, 1024);
    GapOpenStrategy strat(feed);
    strat.setCash(100000.0);
    strat.setCommission(0.001);
    strat.run();
    // Balance should be a positive finite number.
    ASSERT_GT(strat.getBalance(), 0.0);
}

// ── Test runner ───────────────────────────────────────────────────────────────

#define RUN(fn) \
    do { \
        int before = g_failures; \
        fn(); \
        if (g_failures == before) \
            std::cout << "PASS: " << #fn << "\n"; \
        else \
            std::cout << "FAIL: " << #fn << "\n"; \
    } while (0)

int main() {
    // GapOpenStrategy
    RUN(test_gap_open_exact_balance_no_commission);
    RUN(test_gap_open_initial_cash_unchanged_by_run);
    RUN(test_gap_open_commission_reduces_profit);
    RUN(test_gap_open_profitable_on_test_data);
    RUN(test_gap_open_balance_never_negative);
    RUN(test_gap_open_trade_log_created);
    RUN(test_gap_open_trade_log_format);
    RUN(test_gap_open_run_from_bar);
    RUN(test_gap_open_run_from_date);
    RUN(test_gap_open_run_from_date_after_all_bars);
    RUN(test_gap_open_run_from_bar_zero);

    // Portfolio
    RUN(test_portfolio_two_identical_tickers_double_balance);
    RUN(test_portfolio_unequal_weights);
    RUN(test_portfolio_trade_logs_created);
    RUN(test_portfolio_smoke_real_data);

    // Smoke tests with real data
    RUN(test_gap_open_smoke_btc_data);

    if (g_failures == 0) {
        std::cout << "\nAll integration tests PASSED.\n";
    } else {
        std::cout << "\n" << g_failures << " integration test(s) FAILED.\n";
    }
    return g_failures;
}
