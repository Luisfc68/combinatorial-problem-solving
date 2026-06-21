#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <stdlib.h>
#include <set>
#include <cmath>

#define V +

using namespace std;

template <typename T>
using matrix = vector<vector<T>>;

ofstream cnf;
ifstream sol;

typedef string literal;
typedef string clause;

int numberOfCrossings;
int n_vars;
int n_clauses;
matrix<int> ids;

literal FALSE, TRUE;

matrix<int> floyd(matrix<int> F)
{
    int n = F.size();
    for (int x = 0; x < n; ++x)
        assert(F[x][x] == 0);

    for (int k = 0; k < n; ++k)
    {
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                if (F[i][k] != -1 && F[k][j] != -1)
                {
                    if (F[i][j] != -1)
                    {
                        F[i][j] = min(F[i][j], F[i][k] + F[k][j]);
                    }
                    else
                    {
                        F[i][j] = F[i][k] + F[k][j];
                    }
                }
            }
        }
    }

    return F;
}

literal operator-(const literal &lit)
{
    if (lit[0] == '-')
        return lit.substr(1);
    else
        return "-" + lit;
}

literal to_literal(int v)
{
    return to_string(v) + " ";
}

int add_var()
{
    return ++n_vars;
}

void add_clause(const clause &c)
{
    cnf << c << "0" << endl;
    ++n_clauses;
}

void add_amo(const vector<literal> &z)
{
    int n = z.size();
    if (n <= 1)
        return;

    int m = ceil(log2(n));
    vector<literal> y = vector<literal>(m);

    for (int j = 0; j < m; ++j)
    {
        y[j] = (to_literal(add_var()));
    }

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < m; ++j)
        {
            bool bit_is_one = (i >> j) & 1;
            if (bit_is_one)
            {
                add_clause(-z[i] V y[j]);
            }
            else
            {
                add_clause(-z[i] V - y[j]);
            }
        }
    }
}

bool is_arc(const matrix<int> &g, int i, int j)
{
    return i != j && g[i][j] != -1;
}

int get_flow_index(int s, int t, int k, int l)
{
    return ((s * numberOfCrossings + t) * numberOfCrossings + k) * numberOfCrossings + l;
}

literal flow_as_literal(const vector<int> &flows, int s, int t, int k, int l)
{
    return to_literal(flows[get_flow_index(s, t, k, l)]);
}

bool has_flow(const vector<int> &flows, int s, int t, int k, int l)
{
    return flows[get_flow_index(s, t, k, l)] != 0;
}

void full_adder(literal a, literal b, literal c, literal &sum, literal &carry_output)
{
    sum = to_literal(add_var());
    carry_output = to_literal(add_var());
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            for (int k = 0; k < 2; ++k)
            {
                int pattern = i + j + k;
                int pattern_sum = pattern & 1;
                int pattern_carry = pattern >> 1;
                clause ante = (i ? -a : a)V(j ? -b : b) V(k ? -c : c);
                add_clause(ante V(pattern_sum ? sum : -sum));
                add_clause(ante V(pattern_carry ? carry_output : -carry_output));
            }
        }
    }
}

void comparator(vector<literal> &w, int i, int j)
{
    literal x1 = w[i];
    literal x2 = w[j];
    literal y1 = to_literal(add_var());
    literal y2 = to_literal(add_var());
    add_clause(-y1 V x1 V x2);
    add_clause(y1 V - x1);
    add_clause(y1 V - x2);
    add_clause(y2 V - x1 V - x2);
    add_clause(-y2 V x1);
    add_clause(-y2 V x2);
    w[i] = y1;
    w[j] = y2;
}

void oe_merge(vector<literal> &lits, int low, int n, int r)
{
    int m = r * 2;
    if (m < n)
    {
        oe_merge(lits, low, n, m);
        oe_merge(lits, low + r, n, m);
        for (int i = low + r; i + r < low + n; i += m)
            comparator(lits, i, i + r);
    }
    else
    {
        comparator(lits, low, low + r);
    }
}

void oe_sort(vector<literal> &lits, int low, int n)
{
    if (n > 1)
    {
        int m = n / 2;
        oe_sort(lits, low, m);
        oe_sort(lits, low + m, m);
        oe_merge(lits, low, n, 1);
    }
}

vector<literal> sorting_network(vector<literal> in)
{
    int n = 1;
    while (n < in.size())
        n <<= 1;
    while (in.size() < n)
        in.push_back(FALSE);
    oe_sort(in, 0, n);
    return in;
}

vector<literal> add_numbers(const vector<literal> &n1, const vector<literal> &n2)
{
    vector<literal> result;
    literal carry = FALSE;
    int w = max(n1.size(), n2.size());
    for (int i = 0; i < w; ++i)
    {
        literal a = i < n1.size() ? n1[i] : FALSE;
        literal b = i < n2.size() ? n2[i] : FALSE;
        literal sum, carry_output;
        full_adder(a, b, carry, sum, carry_output);
        result.push_back(sum);
        carry = carry_output;
    }
    result.push_back(carry);
    return result;
}

vector<literal> weighted(int w, literal l)
{
    vector<literal> num;
    while (w > 0)
    {
        num.push_back((w & 1) ? l : FALSE);
        w >>= 1;
    }
    return num;
}

void add_less_or_equal_clause(vector<literal> distance, long long max_distance)
{
    long long sum_bits = 0;
    long long tmp = max_distance;

    while (tmp > 0)
    {
        ++sum_bits;
        tmp >>= 1;
    }

    if (sum_bits == 0)
        sum_bits = 1;

    // force all bits above the max_distance width to be 0
    for (int i = sum_bits; i < distance.size(); ++i)
        add_clause(-distance[i]);

    if (distance.size() > sum_bits)
        distance.resize(sum_bits);

    while ((long long)distance.size() < sum_bits)
        distance.push_back(FALSE);

    long long complement = ((1LL << sum_bits) - 1LL) - max_distance;

    vector<literal> complement_literals;
    for (int b = 0; b < sum_bits; ++b)
        complement_literals.push_back(((complement >> b) & 1) ? TRUE : FALSE);

    vector<literal> result = add_numbers(distance, complement_literals);

    add_clause(-result[sum_bits]);
}

void write_CNF(
    const matrix<int> &timeMatrix,
    const matrix<int> &distanceMatrix,
    const vector<pair<int, int>> &twoWayCrossings,
    vector<int> &flows,
    int threshold,
    int forced_streets)
{
    // we need to reset this on every search
    n_vars = 0;
    n_clauses = 0;
    ids = matrix<int>(numberOfCrossings, vector<int>(numberOfCrossings));
    FALSE = to_literal(add_var());
    TRUE = -FALSE;
    add_clause(-FALSE);

    // reachable ordered pairs, the fact they are ordered helps for the binary search later
    vector<pair<int, int>> reachablePairs;
    vector<int> converted = vector<int>(twoWayCrossings.size());
    for (int i = 0; i < numberOfCrossings; ++i)
    {
        for (int j = 0; j < numberOfCrossings; ++j)
        {
            ids[i][j] = add_var();
        }
    }

    for (int s = 0; s < numberOfCrossings; ++s)
    {
        for (int u = 0; u < numberOfCrossings; ++u)
        {
            if (s != u && distanceMatrix[s][u] != -1)
            {
                reachablePairs.push_back({s, u});
                for (int k = 0; k < numberOfCrossings; ++k)
                {
                    for (int l = 0; l < numberOfCrossings; ++l)
                    {
                        if (is_arc(timeMatrix, k, l) && distanceMatrix[s][k] != -1 && distanceMatrix[l][u] != -1)
                        {
                            flows[get_flow_index(s, u, k, l)] = add_var();
                        }
                    }
                }
            }
        }
    }
    for (int i = 0; i < twoWayCrossings.size(); ++i)
        converted[i] = add_var();

    // maintain unidirectional streets (C1 in report)
    for (int i = 0; i < numberOfCrossings; ++i)
    {
        for (int j = 0; j < numberOfCrossings; ++j)
        {
            if (is_arc(timeMatrix, i, j) && timeMatrix[j][i] == -1)
            {
                add_clause(to_literal(ids[i][j]));
            }
        }
    }

    // do not remove both sides (C3 in report)
    pair<int, int> s;
    for (int i = 0; i < twoWayCrossings.size(); ++i)
    {
        s = twoWayCrossings[i];
        add_clause(to_literal(ids[s.first][s.second]) V to_literal(ids[s.second][s.first]));
    }

    for (auto &pair : reachablePairs)
    {
        int s = pair.first;
        int t = pair.second;

        // edges used in the path must be one of the existing edges (C4 in report)
        for (int k = 0; k < numberOfCrossings; ++k)
        {
            for (int l = 0; l < numberOfCrossings; ++l)
            {
                if (is_arc(timeMatrix, k, l) && timeMatrix[l][k] != -1 && has_flow(flows, s, t, k, l))
                {
                    add_clause(-flow_as_literal(flows, s, t, k, l) V to_literal(ids[k][l]));
                }
            }
        }

        // respect the flow from s to t in each pair of reachable vertices (C5 in report)
        for (int v = 0; v < numberOfCrossings; ++v)
        {
            vector<literal> out, in;
            for (int l = 0; l < numberOfCrossings; ++l)
            {
                if (is_arc(timeMatrix, v, l) && has_flow(flows, s, t, v, l))
                {
                    out.push_back(flow_as_literal(flows, s, t, v, l));
                }
            }

            for (int k = 0; k < numberOfCrossings; ++k)
            {
                if (is_arc(timeMatrix, k, v) && has_flow(flows, s, t, k, v))
                {
                    in.push_back(flow_as_literal(flows, s, t, k, v));
                }
            }

            if (v == s)
            {
                for (auto &li : in)
                    add_clause(-li);
                clause c;
                for (auto &li : out)
                    c = c V li;
                add_clause(c);
                add_amo(out);
            }
            else if (v == t)
            {
                for (auto &li : out)
                    add_clause(-li);
                clause c;
                for (auto &li : in)
                    c = c V li;
                add_clause(c);
                add_amo(in);
            }
            else
            {
                add_amo(out);
                add_amo(in);
                clause orOut;
                for (auto &lit : out)
                    orOut = orOut V lit;
                clause orIn;
                for (auto &lit : in)
                    orIn = orIn V lit;
                for (auto &lit : in)
                    add_clause(-lit V orOut); // in used -> some out used
                for (auto &lit : out)
                    add_clause(-lit V orIn); // out used -> some in used
            }
        }

        // C6: sum of used arc times <= B[s][t]
        vector<vector<literal>> terms;
        for (int k = 0; k < numberOfCrossings; ++k)
            for (int l = 0; l < numberOfCrossings; ++l)
                if (is_arc(timeMatrix, k, l) && has_flow(flows, s, t, k, l))
                    terms.push_back(weighted(timeMatrix[k][l], flow_as_literal(flows, s, t, k, l)));

        while (terms.size() > 1)
        {
            vector<vector<literal>> next;
            for (size_t i = 0; i + 1 < terms.size(); i += 2)
                next.push_back(add_numbers(terms[i], terms[i + 1]));
            if (terms.size() % 2)
                next.push_back(terms.back());
            terms = next;
        }
        vector<literal> sum = terms.empty() ? vector<literal>{FALSE} : terms[0];
        long long max_distance = distanceMatrix[s][t] + threshold * distanceMatrix[s][t] / 100; // integer division, as in ILP
        add_less_or_equal_clause(sum, max_distance);
    }

    for (int k = 0; k < twoWayCrossings.size(); ++k)
    {
        int i = twoWayCrossings[k].first;
        int j = twoWayCrossings[k].second;
        add_clause(-to_literal(converted[k]) V - to_literal(ids[i][j]) V - to_literal(ids[j][i]));
    }
    if (forced_streets >= 1)
    {
        vector<literal> converted_literal;
        for (int id : converted)
            converted_literal.push_back(to_literal(id));
        vector<literal> y = sorting_network(converted_literal);
        add_clause(y[forced_streets - 1]); // at least k (forced_streets) of them true
    }

    cnf << "p cnf " << n_vars << " " << n_clauses << endl;
}

bool run_kissat(set<int> &true_lits)
{
    true_lits.clear();
    system("tac tmp.rev | kissat | grep -E -v \"^c\" | tail --lines=+2 | cut --delimiter=' ' --field=1 --complement > tmp.out");
    sol.open("tmp.out");
    int lit;
    while (sol >> lit)
    {
        if (lit > 0)
            true_lits.insert(lit);
    }
    sol.close();
    return !true_lits.empty();
}

bool feasible(
    const matrix<int> &timeMatrix,
    const matrix<int> &distanceMatrix,
    const vector<pair<int, int>> &twoWayCrossings,
    vector<int> &flows,
    int threshold,
    int forced_streets,
    set<int> &model)
{
    cnf.open("tmp.rev");
    write_CNF(timeMatrix, distanceMatrix, twoWayCrossings, flows, threshold, forced_streets);
    cnf.close();
    return run_kissat(model);
}

int main(int argc, char **argv)
{
    int threshold;
    matrix<int> timeMatrix, distanceMatrix;
    string crossingTime;
    vector<pair<int, int>> twoWayCrossings;
    vector<int> flows;

    cin >> numberOfCrossings;
    timeMatrix = matrix<int>(numberOfCrossings, vector<int>(numberOfCrossings));
    for (int i = 0; i < numberOfCrossings; ++i)
    {
        for (int j = 0; j < numberOfCrossings; ++j)
        {
            cin >> crossingTime;
            timeMatrix[i][j] = stoi(crossingTime);
        }
    }
    cin >> threshold;

    distanceMatrix = floyd(timeMatrix);
    flows = vector<int>(pow(numberOfCrossings, 4));

    for (int i = 0; i < numberOfCrossings; ++i)
    {
        for (int j = i + 1; j < numberOfCrossings; ++j)
        {
            if (timeMatrix[i][j] != -1 && timeMatrix[j][i] != -1)
                twoWayCrossings.push_back({i, j});
        }
    }

    // we do binary search to find the optimal value
    set<int> solution, best_solution;
    int low = 0;
    int high = twoWayCrossings.size();
    int best = 0;
    bool is_feasible;
    feasible(timeMatrix, distanceMatrix, twoWayCrossings, flows, threshold, 0, best_solution);
    while (low <= high)
    {
        int mid = (low + high) / 2;
        is_feasible = feasible(timeMatrix, distanceMatrix, twoWayCrossings, flows, threshold, mid, solution);
        if (is_feasible)
        {
            best = mid;
            best_solution = solution;
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    cout << numberOfCrossings << endl;
    for (int i = 0; i < numberOfCrossings; ++i)
    {
        for (int j = 0; j < numberOfCrossings; ++j)
            cout << timeMatrix[i][j] << " ";
        cout << endl;
    }
    cout << threshold << endl;

    int converted = 0;
    for (auto &s : twoWayCrossings)
    {
        int i = s.first, j = s.second;
        bool keepIJ = best_solution.count(ids[i][j]);
        bool keepJI = best_solution.count(ids[j][i]);
        if (keepIJ && !keepJI)
        {
            cout << i << " " << j << endl;
            ++converted;
        }
        else if (!keepIJ && keepJI)
        {
            cout << j << " " << i << endl;
            ++converted;
        }
    }
    cout << converted << endl;
}
