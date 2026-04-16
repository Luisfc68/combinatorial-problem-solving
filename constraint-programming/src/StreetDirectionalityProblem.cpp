#include <cstdlib>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

using namespace std;
using namespace Gecode;

enum Direction {
    NONE,
    FORWARD,
    BACKWARD
};

vector<vector<int>> floyd(vector<vector<int>> F) {
    int n = F.size();
    for (int x = 0; x < n; ++x) assert(F[x][x] == 0);

    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (F[i][k] != -1 && F[k][j] != -1) {
                    if (F[i][j] != -1) {
                        F[i][j] = min(F[i][j], F[i][k] + F[k][j]);
                    }
                    else {
                        F[i][j] = F[i][k] + F[k][j];
                    }
                }
            }
        }
    }

    return F;
};

class RespectLimitPropagator : public Propagator {
private:
    const vector<vector<int>> distances;
    const vector<vector<int>> limits;
    vector<pair<int, int>> streets;

protected:
    ViewArray<Int::IntView> removals;

public:
    RespectLimitPropagator(
        Home home,
        ViewArray<Int::IntView> removals,
        const vector<vector<int>> distances,
        const vector<vector<int>> limits
    ) : Propagator(home), distances(distances), limits(limits), removals(removals) {
        int n = distances.size();
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i < j && distances[i][j] == distances[j][i] && distances[i][j] != -1) {
                    streets.push_back(make_pair(i, j));
                }
            }
        }
        removals.subscribe(home, *this, Int::PC_INT_VAL); // TODO documentar por que PC_INT_VAL
    }

    RespectLimitPropagator(
        Space& home,
        RespectLimitPropagator& propagator
    ) : Propagator(home, propagator),
        distances(propagator.distances),
        limits(propagator.limits),
        streets(propagator.streets)
    {
        removals.update(home, propagator.removals);
    }

    static ExecStatus post(
        Home home,
        ViewArray<Int::IntView> removals,
        const vector<vector<int>>& distances,
        const vector<vector<int>>& limits
    ) {
        int n = distances.size();
        for (int i = 0; i < n; ++i)
            if (distances[i][i] != 0) return ES_FAILED;

        auto costs = floyd(distances);
        bool breaksLimit, bidirectionalAsymmetric;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                breaksLimit = costs[i][j] > limits[i][j];
                bidirectionalAsymmetric = distances[i][j] > 0 && distances[j][i] > 0 && distances[i][j] != distances[j][i];
                if (breaksLimit || bidirectionalAsymmetric) return ES_FAILED;
            }
        }


        // TODO mejorar segun el manual 316
        // revisar limits y costs (que la diagonal es 0 y que limite es mas grande)
        (void) new (home) RespectLimitPropagator(home, removals, distances, limits);
        return ES_OK;
    }

    virtual size_t dispose(Space& home) override {
        removals.cancel(home, *this, Int::PC_INT_VAL);
        (void)Propagator::dispose(home);
        return sizeof(*this);
    }

    virtual Propagator* copy(Space& home) override {
        return new (home) RespectLimitPropagator(home, *this);
    }

    virtual PropCost cost(const Space&, const ModEventDelta&) const override {
        return PropCost::cubic(PropCost::HI, (unsigned int)distances.size()); // TODO justificar HI
    }

    virtual void reschedule(Space& home) override {
        removals.reschedule(home, *this, Int::PC_INT_VAL);
    }

    virtual ExecStatus propagate(Space& home, const ModEventDelta&) override {
        vector<vector<int>> updatedDistances(distances);
        int first, second;
        for (int i = 0; i < streets.size(); ++i) {
            if (removals[i].assigned()) {
                first = streets[i].first;
                second = streets[i].second;
                if (removals[i].val() == FORWARD) updatedDistances[first][second] = -1;
                else if (removals[i].val() == BACKWARD) updatedDistances[second][first] = -1;
            }
        }

        auto updatedCosts = floyd(updatedDistances);

        if (exceedsLimit(updatedCosts)) {
            return ES_FAILED;
        }

        GECODE_ES_CHECK(pruneCurrentSolution(home, updatedDistances));

        for (int i = 0; i < removals.size(); ++i)
            if (!removals[i].assigned()) return ES_FIX;

        return home.ES_SUBSUMED(*this);
    }

private:
    ExecStatus pruneCurrentSolution(Home home, vector<vector<int>>& currentSolution) {
        for (int i = 0; i < removals.size(); ++i) {
            if (!removals[i].assigned()) {
                GECODE_ES_CHECK(pruneStreet(home, currentSolution, i));
            }
        }
        return ES_OK;
    }

    ExecStatus pruneStreet(Home home, vector<vector<int>> &currentSolution, int index) {
        const vector<Direction> pruneOptions = {FORWARD, BACKWARD};
        int first, second;
        Direction option;
        vector<vector<int>> solutionToCheck;
        for (int o = 0; o < pruneOptions.size(); ++o) {
            option = pruneOptions[o];
            solutionToCheck = currentSolution;
            first = streets[index].first;
            second = streets[index].second;
            if (option == FORWARD)  solutionToCheck[first][second] = -1;
            if (option == BACKWARD) solutionToCheck[second][first] = -1;
            auto costs = floyd(solutionToCheck);
            if (exceedsLimit(costs)) {
                GECODE_ME_CHECK(removals[index].nq(home, option));
            }
        }
        return ES_OK;
    }

    bool exceedsLimit(vector<vector<int>>& costMatrix) {
        int n = costMatrix.size();
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                // second part is that it became unrecheable
                if (costMatrix[i][j] > limits[i][j] || (costMatrix[i][j] == -1 && limits[i][j] != -1))
                    return true;
            }
        }
        return false;
    }
};

void respectLimit(
    Home home,
    const IntVarArgs& removals,
    const vector<vector<int>>& costs,
    const vector<vector<int>>& limits
) {
    GECODE_POST;
    ViewArray<Int::IntView> removalsView(home, removals);
    GECODE_ES_FAIL(RespectLimitPropagator::post(home, removalsView, costs, limits));
}

class StreetDirectionality : public Space {

private:
    int numberOfCrossings;
    int threshold;
    vector<vector<int>> costMatrix;
    vector<vector<int>> limitMatrix;
    vector<pair<int, int>> twoWayStreets;

    IntVarArray twoWayRemovals;
    IntVar numberOfRemovals;

public:
    StreetDirectionality(
        int numberOfCrossings,
        int threshold,
        vector<vector<int>> timeMatrix
    ) :
        numberOfCrossings(numberOfCrossings),
        threshold(threshold),
        costMatrix(timeMatrix),
        limitMatrix(numberOfCrossings, vector<int>(numberOfCrossings))
    {
        int value;
        limitMatrix = floyd(timeMatrix);
        for (int i = 0; i < numberOfCrossings; ++i) {
            for (int j = 0; j < numberOfCrossings; ++j) {
                if (i < j && timeMatrix[i][j] == timeMatrix[j][i] && timeMatrix[i][j] != -1) {
                    twoWayStreets.push_back(make_pair(i, j));
                }
                value = limitMatrix[i][j];
                // limits are truncated so they are inclusive, e.g. in the example threshold is 38 and
                // so limit would be 3*1.38=4.14, this matrix will have 4 which is ok
                limitMatrix[i][j] = value == -1 ? value : value + (threshold * value) / 100;
            }
        }
        twoWayRemovals = IntVarArray(*this, twoWayStreets.size(), IntSet{ NONE, FORWARD, BACKWARD });
        numberOfRemovals = IntVar(*this, 0, twoWayStreets.size());

        respectLimit(*this, twoWayRemovals, costMatrix, limitMatrix);
        count(*this, twoWayRemovals, IntSet{ FORWARD, BACKWARD }, IRT_EQ, numberOfRemovals);
        // max para no empezar buscando por 0 que seria no hacer nada
        branch(*this, twoWayRemovals, INT_VAR_NONE(), INT_VAL_MAX());
    }


    StreetDirectionality(StreetDirectionality& s) : Space(s) {
        twoWayRemovals.update(*this, s.twoWayRemovals);
        numberOfRemovals.update(*this, s.numberOfRemovals);
        twoWayStreets = s.twoWayStreets;
        numberOfCrossings = s.numberOfCrossings;
        threshold = s.threshold;
        costMatrix = s.costMatrix;
        limitMatrix = s.limitMatrix;
    }

    virtual Space* copy() {
        return new StreetDirectionality(*this);
    }

    virtual void constrain(const Space& _s) {
        const StreetDirectionality& s = static_cast<const StreetDirectionality&>(_s);

        int value = s.numberOfRemovals.val();
        rel(*this, numberOfRemovals > value);
    }

    int optimalValue() {
        return numberOfRemovals.val();
    }

    void print() const {
        cout << numberOfCrossings << endl;
        for (int i = 0; i < numberOfCrossings; ++i) {
            for (int j = 0; j < numberOfCrossings; ++j)
                cout << costMatrix[i][j] << " ";
            cout << endl;
        }
        cout << threshold << endl;
        auto totalRemoved = 0;
        for (int i = 0; i < twoWayStreets.size(); ++i) {
            // the result if the opposite of what we remove (the remaining orientation)
            if (twoWayRemovals[i].val() == FORWARD) {
                cout << twoWayStreets[i].second << " " << twoWayStreets[i].first << endl;
                totalRemoved++;
            }
            else if (twoWayRemovals[i].val() == BACKWARD) {
                cout << twoWayStreets[i].first << " " << twoWayStreets[i].second << endl;
                totalRemoved++;
            }
        }
        cout << totalRemoved << endl;
    }
};


int main(int argc, char* argv[]) {
    int numberOfCrossings, threshold;
    vector<vector<int>> timeMatrix;
    string crossingTime;

    cin >> numberOfCrossings;

    timeMatrix = vector<vector<int>>(numberOfCrossings, vector<int>(numberOfCrossings));
    for (int i = 0; i < numberOfCrossings; ++i) {
        for (int j = 0; j < numberOfCrossings; ++j) {
            cin >> crossingTime;
            timeMatrix[i][j] = stoi(crossingTime);
        }
    }

    cin >> threshold;

    StreetDirectionality* m = new StreetDirectionality(numberOfCrossings, threshold, timeMatrix);
    BAB<StreetDirectionality> e(m);
    delete m;
    StreetDirectionality* best = NULL;

    while (StreetDirectionality* s = e.next()) {
        delete best;
        best = s;
    }

    if (best != NULL) {
        best->print();
        delete best;
    }
}
