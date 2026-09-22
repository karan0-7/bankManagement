// test_scale.cpp
//
// Standalone scale + correctness test harness for the Bank System at Scale
// project, written against the FRD's "Data Volume and Scale" (section 5) and
// "Performance Requirements and Benchmarking" (section 6) requirements.
//
// This file does NOT modify bank.cpp, bank.h, account.h, account.cpp,
// user.h, or user.cpp. It links against the real Account/User classes and
// drives them directly (bypassing the interactive std::cin prompts in
// addAccount()/addUser()/bank.cpp), so every test here exercises the actual
// production code.
//
// Build (from the bankManagement directory):
//   c++ -std=c++17 -O2 test_scale.cpp account/account.cpp user/user.cpp -o test_scale
//
// Run:
//   ./test_scale            // default: 1,000,000 operations per test
//   ./test_scale 100000     // override the scale for a faster run
//
// Each test_* function is also independently callable if you only want to
// exercise one capability (see main() below).

#include "./account/account.h"
#include "./user/user.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>

using Clock = std::chrono::steady_clock;

struct TestReport
{
    int passed = 0;
    int failed = 0;

    void check(bool condition, const std::string &description)
    {
        if (condition)
        {
            ++passed;
            std::cout << "  [PASS] " << description << '\n';
        }
        else
        {
            ++failed;
            std::cout << "  [FAIL] " << description << '\n';
        }
    }

    void summary(const std::string &suiteName) const
    {
        std::cout << suiteName << ": " << passed << " passed, " << failed << " failed\n\n";
    }
};

static double elapsedMs(Clock::time_point start, Clock::time_point end)
{
    return std::chrono::duration<double, std::milli>(end - start).count();
}

static void reportTiming(const std::string &label, double ms, long operations)
{
    std::cout << "  [TIMING] " << label << ": " << std::fixed << std::setprecision(2)
               << ms << " ms for " << operations << " ops ("
               << (operations / (ms / 1000.0)) << " ops/sec)\n";
}

// ---------------------------------------------------------------------------
// FRD 5: at least 100k concurrently existing accounts; here run at 1M.
// FRD 6: measure and report account-creation time.
// FRD 1 & 7: every issued account id must be unique.
// ---------------------------------------------------------------------------
std::unordered_map<int, Account> test_createAccountsAtScale(long count)
{
    std::cout << "== test_createAccountsAtScale(" << count << ") ==\n";
    TestReport report;
    std::unordered_map<int, Account> accounts;
    accounts.reserve(static_cast<size_t>(count));

    auto start = Clock::now();
    for (long i = 0; i < count; ++i)
    {
        int startingBalance = 100 + static_cast<int>(i % 1000);
        Account account{startingBalance, "savings"};
        accounts.insert({account.getId(), account});
    }
    auto end = Clock::now();

    reportTiming("Account creation (unordered_map<int, Account>)", elapsedMs(start, end), count);

    report.check(static_cast<long>(accounts.size()) == count,
                 "created account count matches requested scale (no id collisions on insert)");

    report.summary("test_createAccountsAtScale");
    return accounts;
}

// ---------------------------------------------------------------------------
// FRD 5 & 6: same scale/timing expectations, applied to User.
// ---------------------------------------------------------------------------
std::unordered_map<int, User> test_createUsersAtScale(long count)
{
    std::cout << "== test_createUsersAtScale(" << count << ") ==\n";
    TestReport report;
    std::unordered_map<int, User> users;
    users.reserve(static_cast<size_t>(count));

    auto start = Clock::now();
    for (long i = 0; i < count; ++i)
    {
        User user{"user_" + std::to_string(i)};
        users.insert({user.getId(), user});
    }
    auto end = Clock::now();

    reportTiming("User creation (unordered_map<int, User>)", elapsedMs(start, end), count);

    report.check(static_cast<long>(users.size()) == count,
                 "created user count matches requested scale (no id collisions on insert)");

    report.summary("test_createUsersAtScale");
    return users;
}

// ---------------------------------------------------------------------------
// FRD 1: deposits increase balance by exactly the given amount, and an
// invalid (zero/negative) deposit must be rejected.
// FRD 6: measure and report batch deposit time.
// ---------------------------------------------------------------------------
void test_depositsAtScale(std::unordered_map<int, Account> &accounts, long count)
{
    std::cout << "== test_depositsAtScale(" << count << ") ==\n";
    TestReport report;

    std::mt19937 rng{42};
    std::uniform_int_distribution<int> accountPicker(1, static_cast<int>(accounts.size()));
    std::uniform_int_distribution<int> amountPicker(1, 500);

    long long balanceBefore = 0;
    for (auto &entry : accounts)
        balanceBefore += entry.second.getBalance();

    auto start = Clock::now();
    long long totalDeposited = 0;
    for (long i = 0; i < count; ++i)
    {
        int id = accountPicker(rng);
        int amount = amountPicker(rng);
        accounts.at(id).deposit(amount);
        totalDeposited += amount;
    }
    auto end = Clock::now();

    reportTiming("Deposits", elapsedMs(start, end), count);

    long long balanceAfter = 0;
    for (auto &entry : accounts)
        balanceAfter += entry.second.getBalance();

    report.check(balanceAfter == balanceBefore + totalDeposited,
                 "sum of all account balances increased by exactly the sum of deposited amounts");

    // FRD 1: "Rejection of any deposit ... request that is invalid (zero or negative amount)"
    int sampleId = accounts.begin()->first;
    int balanceBeforeInvalid = accounts.at(sampleId).getBalance();
    accounts.at(sampleId).deposit(-500);
    int balanceAfterInvalid = accounts.at(sampleId).getBalance();
    report.check(balanceAfterInvalid == balanceBeforeInvalid,
                 "deposit() rejects a negative amount and leaves the balance unchanged");

    report.summary("test_depositsAtScale");
}

// ---------------------------------------------------------------------------
// FRD 1 & 7: withdrawals decrease balance, an over-limit withdrawal must be
// rejected, and a balance may never become negative.
// FRD 6: measure and report batch withdrawal time.
// ---------------------------------------------------------------------------
void test_withdrawalsAtScale(std::unordered_map<int, Account> &accounts, long count)
{
    std::cout << "== test_withdrawalsAtScale(" << count << ") ==\n";
    TestReport report;

    std::mt19937 rng{7};
    std::uniform_int_distribution<int> accountPicker(1, static_cast<int>(accounts.size()));

    auto start = Clock::now();
    for (long i = 0; i < count; ++i)
    {
        int id = accountPicker(rng);
        Account &account = accounts.at(id);
        int amount = std::min(50, std::max(1, account.getBalance() / 4 + 1));
        account.widthdrawal(amount);
    }
    auto end = Clock::now();

    reportTiming("Withdrawals", elapsedMs(start, end), count);

    bool anyNegative = false;
    for (auto &entry : accounts)
        if (entry.second.getBalance() < 0)
            anyNegative = true;

    report.check(!anyNegative,
                 "no account balance is negative after in-bounds withdrawals");

    // FRD 1 & 7: "Withdrawals ... provided sufficient funds are available" /
    // "No account balance may ever become negative as a result of a normal operation"
    int sampleId = accounts.begin()->first;
    Account &sample = accounts.at(sampleId);
    int currentBalance = sample.getBalance();
    sample.widthdrawal(currentBalance + 1000);
    report.check(sample.getBalance() >= 0,
                 "widthdrawal() rejects an amount greater than the balance instead of going negative");

    report.summary("test_withdrawalsAtScale");
}

// ---------------------------------------------------------------------------
// FRD 3: a transfer is a withdrawal from source + deposit into destination,
// and a successful transfer must actually move funds between the two real
// stored accounts (not local copies), while a transfer whose withdrawal
// fails must leave the destination completely unaffected.
// FRD 6: measure and report batch transfer time.
//
// This reproduces the exact statements bank.cpp::transferFunds() executes
// against the accounts map (now via Account& references), so it is testing
// the real transfer mechanics as implemented today, not an idealized
// version of them.
// ---------------------------------------------------------------------------
void test_transfersAtScale(std::unordered_map<int, Account> &accounts, long count)
{
    std::cout << "== test_transfersAtScale(" << count << ") ==\n";
    TestReport report;

    std::mt19937 rng{99};
    int n = static_cast<int>(accounts.size());
    std::uniform_int_distribution<int> accountPicker(1, n);
    const int transferAmount = 10;

    // Track what a correct, atomic transfer (withdraw only if affordable,
    // then deposit -- FRD 3) would produce, so a mismatch below pinpoints a
    // real deviation instead of just checking totals, which can balance out
    // by coincidence even when a bug is present.
    std::vector<long long> expectedBalance(n + 1, 0);
    for (auto &entry : accounts)
        expectedBalance[entry.first] = entry.second.getBalance();

    auto start = Clock::now();
    long performed = 0;
    for (long i = 0; i < count; ++i)
    {
        int sourceId = accountPicker(rng);
        int destId = accountPicker(rng);
        if (sourceId == destId)
            continue;

        // Same statements as bank.cpp::transferFunds():
        Account &sourceAccount = accounts.at(sourceId);
        Account &destinationAccount = accounts.at(destId);
        sourceAccount.widthdrawal(transferAmount);
        destinationAccount.deposit(transferAmount);
        ++performed;

        if (expectedBalance[sourceId] >= transferAmount)
        {
            expectedBalance[sourceId] -= transferAmount;
            expectedBalance[destId] += transferAmount;
        }
        // else: a correct implementation rejects the withdrawal and leaves
        // both accounts untouched -- expectedBalance intentionally does not
        // change here so the check below can catch it if the real code
        // credits the destination anyway.
    }
    auto end = Clock::now();

    reportTiming("Transfers (bank.cpp transferFunds() semantics)", elapsedMs(start, end), performed);

    bool perAccountMatches = true;
    int firstMismatchId = -1;
    for (auto &entry : accounts)
    {
        if (entry.second.getBalance() != expectedBalance[entry.first])
        {
            perAccountMatches = false;
            if (firstMismatchId == -1)
                firstMismatchId = entry.first;
        }
    }

    report.check(perAccountMatches,
                 "every account's balance matches what an atomic withdraw-then-deposit transfer "
                 "would produce -- transferFunds() mutates the real stored accounts via "
                 "references, with no destination credited when its source withdrawal is rejected");
    if (!perAccountMatches)
        std::cout << "    e.g. first mismatch at account id " << firstMismatchId << '\n';

    report.summary("test_transfersAtScale");
}

// ---------------------------------------------------------------------------
// FRD 3: "If the withdrawal from the source account fails for any reason
// (insufficient funds, invalid amount, invalid account), the transfer must
// not proceed, and the destination account must be left completely
// unaffected."
//
// The bulk test above relies on random pairing to occasionally hit this
// case; this test isolates it deterministically against a controlled pair
// of accounts so the outcome doesn't depend on chance.
// ---------------------------------------------------------------------------
void test_transferInsufficientFunds()
{
    std::cout << "== test_transferInsufficientFunds ==\n";
    TestReport report;

    std::unordered_map<int, Account> localAccounts;
    Account poorSource{5, "checking"};
    Account destination{100, "checking"};
    int sourceId = poorSource.getId();
    int destId = destination.getId();
    localAccounts.insert({sourceId, poorSource});
    localAccounts.insert({destId, destination});

    const int transferAmount = 50; // larger than poorSource's balance of 5

    // Same statements as bank.cpp::transferFunds():
    Account &sourceAccount = localAccounts.at(sourceId);
    Account &destinationAccount = localAccounts.at(destId);
    sourceAccount.widthdrawal(transferAmount);
    destinationAccount.deposit(transferAmount);

    report.check(localAccounts.at(sourceId).getBalance() == 5,
                 "source account balance is untouched when it cannot afford the transfer");
    report.check(localAccounts.at(destId).getBalance() == 100,
                 "destination account is left completely unaffected when the source "
                 "withdrawal is rejected for insufficient funds");

    report.summary("test_transferInsufficientFunds");
}

// ---------------------------------------------------------------------------
// FRD 4: account lookup must distinguish a hit from a miss safely.
// ---------------------------------------------------------------------------
void test_accountLookup(std::unordered_map<int, Account> &accounts)
{
    std::cout << "== test_accountLookup ==\n";
    TestReport report;

    int existingId = accounts.begin()->first;
    bool foundExisting = accounts.find(existingId) != accounts.end();
    report.check(foundExisting, "lookup of an existing account id succeeds");

    int missingId = -1;
    bool foundMissing = accounts.find(missingId) != accounts.end();
    report.check(!foundMissing,
                 "lookup of a non-existent account id (-1) is safely reported as not-found via find()");

    bool threw = false;
    try
    {
        accounts.at(missingId);
    }
    catch (const std::out_of_range &)
    {
        threw = true;
    }
    report.check(threw,
                 "accessing a non-existent account id through at() throws rather than risking invalid access");

    report.summary("test_accountLookup");
}

// ---------------------------------------------------------------------------
// FRD 2: every balance-changing operation must be recorded as a transaction,
// retrievable as a complete history from the account itself.
// ---------------------------------------------------------------------------
void test_transactionHistory(std::unordered_map<int, Account> &accounts)
{
    std::cout << "== test_transactionHistory ==\n";
    TestReport report;

    int id = accounts.begin()->first;
    Account &account = accounts.at(id);
    size_t before = account.getTransactions().size();

    account.deposit(25);
    account.widthdrawal(10);

    size_t after = account.getTransactions().size();
    report.check(after == before + 2,
                 "each deposit/withdrawal appends exactly one transaction to the account's history");

    report.summary("test_transactionHistory");
}

// ---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    long scale = 1000000;
    if (argc > 1)
        scale = std::stol(argv[1]);

    std::cout << "Bank System at Scale -- test harness (scale = " << scale << ")\n\n";

    auto accounts = test_createAccountsAtScale(scale);
    auto users = test_createUsersAtScale(scale);

    test_depositsAtScale(accounts, scale);
    test_withdrawalsAtScale(accounts, scale);
    test_transfersAtScale(accounts, scale / 2);
    test_transferInsufficientFunds();
    test_accountLookup(accounts);
    test_transactionHistory(accounts);

    return 0;
}
