// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}
// -------------------------------------- registry

TEST(reg_empty_table) {
    ToolRegistry r;
    CHECK_EQ(r.size(), std::size_t{0});
    CHECK(r.bucket_count() >= 1 && r.bucket_count() <= 64);
    CHECK_EQ(r.max_probe_length(), std::size_t{0});
    CHECK(r.find("anything") == nullptr);
}

TEST(reg_null_rejected) {
    ToolRegistry r;
    CHECK(!r.add(nullptr));
    CHECK_EQ(r.size(), std::size_t{0});
    REQUIRE(r.add(std::make_unique<NamedTool>("a")));
    CHECK(!r.add(std::unique_ptr<Tool>()));
    CHECK_EQ(r.size(), std::size_t{1});
    CHECK(r.find("b") == nullptr);
    CHECK_EQ(r.size(), std::size_t{1});
}

TEST(reg_for_each_visits_each_once) {
    ToolRegistry r;
    const std::vector<std::string> names = {"read_file", "write_file", "list_dir",
                                            "calculator", "echo", "x"};
    for (const auto& n : names) REQUIRE(r.add(std::make_unique<NamedTool>(n)));

    std::vector<std::string> seen;
    r.for_each([&](const Tool& t) { seen.push_back(t.name()); });
    CHECK_EQ(seen.size(), names.size());
    for (const auto& n : names) {
        int count = 0;
        for (const auto& s : seen) if (s == n) ++count;
        CHECK_EQ(count, 1);
    }
}

TEST(reg_hash_order_sensitive) {
    CHECK(ToolRegistry::hash("ab") != ToolRegistry::hash("ba"));
    CHECK(ToolRegistry::hash("tool_1234") != ToolRegistry::hash("tool_1243"));
}

TEST(reg_10000_tools) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("tool_0");
    Tool* first_ptr = first.get();
    REQUIRE(r.add(std::move(first)));
    Tool* found_first = r.find("tool_0");
    CHECK(found_first == first_ptr);

    bool load_ok = r.load_factor() <= 1.0;
    for (int i = 1; i < 10000; ++i) {
        REQUIRE(r.add(std::make_unique<NamedTool>("tool_" + std::to_string(i))));
        if (r.load_factor() > 1.0) load_ok = false;
    }
    CHECK(load_ok);
    CHECK_EQ(r.size(), std::size_t{10000});

    bool all_found = true;
    for (int i = 0; i < 10000; ++i) {
        const std::string n = "tool_" + std::to_string(i);
        Tool* t = r.find(n);
        if (t == nullptr || t->name() != n) all_found = false;
    }
    CHECK(all_found);
    CHECK(r.find("tool_0") == found_first);
    CHECK(r.max_probe_length() <= 64);

    std::printf("    FNV-1a: bucket_count=%zu load_factor=%.4f max_probe_length=%zu\n",
                r.bucket_count(), r.load_factor(), r.max_probe_length());
}

TEST(reg_alternative_hash) {
    ToolRegistry r;
    for (int i = 0; i < 10000; ++i)
        REQUIRE(r.add(std::make_unique<NamedTool>("tool_" + std::to_string(i))));
    const std::size_t buckets = r.bucket_count();

    std::vector<int> counts(buckets, 0);
    for (int i = 0; i < 10000; ++i) {
        const std::string n = "tool_" + std::to_string(i);
        std::size_t weak = static_cast<unsigned char>(n[0]) + n.size();
        ++counts[weak % buckets];
    }
    int longest = 0;
    for (int c : counts) if (c > longest) longest = c;

    std::printf("    FNV-1a longest chain=%zu | weak s[0]+s.size() longest chain=%d (buckets=%zu)\n",
                r.max_probe_length(), longest, buckets);
    CHECK(static_cast<std::size_t>(longest) > r.max_probe_length());
}

// -------------------------------------------------- workspace

namespace {
bool accepts(const Workspace& ws, const std::string& in, const fs::path& expected) {
    fs::path out;
    std::string reason;
    return ws.resolve(in, &out, &reason) && out == expected;
}
bool rejects(const Workspace& ws, const std::string& in) {
    fs::path out;
    std::string reason;
    return !ws.resolve(in, &out, &reason) && !reason.empty();
}
}  // namespace

TEST(ws_accepts_normal_paths) {
    TempDir d;
    Workspace ws(d.path());
    const fs::path root = ws.root();
    CHECK(accepts(ws, "notes/todo.txt", root / "notes" / "todo.txt"));
    CHECK(accepts(ws, "./notes//todo.txt", root / "notes" / "todo.txt"));
    CHECK(accepts(ws, ".", root));
    CHECK(accepts(ws, "a/../b", root / "b"));
    CHECK(accepts(ws, "a/..", root));
    CHECK(accepts(ws, "new/dir/file.txt", root / "new" / "dir" / "file.txt"));
    CHECK(accepts(ws, "my..notes.txt", root / "my..notes.txt"));
    CHECK(!fs::exists(root / "new"));   // resolve must not create anything
}

TEST(ws_rejects_empty_and_nul) {
    TempDir d;
    Workspace ws(d.path());
    CHECK(rejects(ws, ""));
    CHECK(rejects(ws, std::string("notes") + '\0' + "/x"));
}

TEST(ws_rejects_absolute) {
    TempDir d;
    Workspace ws(d.path());
    CHECK(rejects(ws, "/etc/passwd"));
    CHECK(rejects(ws, (ws.root() / "notes").string()));   // absolute even if inside root
}

TEST(ws_rejects_dotdot) {
    TempDir d;
    Workspace ws(d.path());
    CHECK(rejects(ws, ".."));
    CHECK(rejects(ws, "../etc/passwd"));
    CHECK(rejects(ws, "a/../../b"));
}

TEST(ws_rejects_escape_and_return) {
    TempDir d;
    fs::create_directories(d.path() / "ws" / "notes");
    Workspace ws(d.path() / "ws");
    CHECK(rejects(ws, "notes/../../ws/notes/todo.txt"));
}

// -------------------------------- calculator

namespace {
bool evals_to(const std::string& expr, double expected) {
    double v = 0;
    std::string err;
    return eval(expr, &v, &err) && v == expected;
}
}  // namespace

TEST(calc_left_to_right) {
    CHECK(evals_to("10 - 4 - 3", 3));
    CHECK(evals_to("100 / 10 / 5", 2));
}

TEST(calc_parentheses) {
    CHECK(evals_to("(2 + 3) * 4", 20));
    CHECK(evals_to("2 * (3 + 4) - 5", 9));
    CHECK(evals_to("((1 + 2) * (3 + 4))", 21));
}

TEST(calc_negatives_and_decimals) {
    CHECK(evals_to("-3 * -3", 9));
    CHECK(evals_to("7 / 2", 3.5));
    CHECK(evals_to("1.5 + 2.25", 3.75));
    CHECK(evals_to("2 * -3 + 10", 4));
}

TEST(calc_division_by_zero_in_parens) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / (2 - 2)", &v, &err));
    CHECK(err == "division by zero");
}

TEST(calc_long_and_nested) {
    std::string sum = "1";
    for (int i = 0; i < 20000; ++i) sum += "+1";
    CHECK(evals_to(sum, 20001));

    std::string nested(5000, '(');
    nested += "7";
    nested += std::string(5000, ')');
    CHECK(evals_to(nested, 7));
}

int main(int argc, char** argv) { return mt::run_all(argc, argv); }
