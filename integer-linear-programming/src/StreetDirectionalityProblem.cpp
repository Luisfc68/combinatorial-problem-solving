#include <ilcplex/ilocplex.h>
#include <vector>
ILOSTLBEGIN

using namespace std;

template <typename T>
using matrix = vector<vector<T>>;

matrix<int> floyd(matrix<int> F) {
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

IloBoolVar cell(IloBoolVarArray matrix, int n, int i, int j) { return matrix[i * n + j]; }

void add_flow_constraint(
    IloModel model,
    IloEnv env,
    matrix<IloBoolVarArray> paths,
    int numberOfCrossings,
    int s,
    int t
) {
    // respect the flow from s to t in each pair of reachable vertices (C5 in report)
    int balanceResult;
    for (int v = 0; v < numberOfCrossings; ++v) {
        IloExpr balance(env);
        // flow going out of v
        for (int u = 0; u < numberOfCrossings; ++u)
            balance += cell(paths[s][t], numberOfCrossings, v, u);
        // flow going into v
        for (int u = 0; u < numberOfCrossings; ++u)
            balance -= cell(paths[s][t], numberOfCrossings, u, v);

        // if is the origin the flow is 1 because it starts there,
        // if is the destination the flow is -1 because it ends there,
        // the rest must have 0 because the same comes in and out
        if (v == s) {
            balanceResult = 1;
        }
        else if (v == t) {
            balanceResult = -1;
        }
        else {
            balanceResult = 0;
        }
        model.add(balance == balanceResult);
        balance.end();
    }
}

void add_threshold_constraint(
    IloModel model,
    IloEnv env,
    matrix<IloBoolVarArray> paths,
    IloBoolVarArray stays,
    matrix<int> distanceMatrix,
    matrix<int> timeMatrix,
    int threshold,
    int numberOfCrossings,
    int s,
    int t
) {
    IloBoolVarArray pairPath = paths[s][t];
    IloExpr pathValueExpr = IloExpr(env);
    for (int k = 0; k < numberOfCrossings; ++k) {
        for (int l = 0; l < numberOfCrossings; ++l) {
            // edges used in the path must be one of the existing edges (C4 in report)
            model.add(cell(pairPath, numberOfCrossings, k, l) <= cell(stays, numberOfCrossings, k, l));
            pathValueExpr += cell(pairPath, numberOfCrossings, k, l) * timeMatrix[k][l];

        }
    }
    // respect the threshold (C6 in report)
    model.add(pathValueExpr <= (distanceMatrix[s][t] + (threshold * distanceMatrix[s][t]) / 100));
    pathValueExpr.end();
}



int main()
{
    int numberOfCrossings, threshold;
    matrix<int> timeMatrix, distanceMatrix;
    string crossingTime;

    cin >> numberOfCrossings;
    timeMatrix = matrix<int>(numberOfCrossings, vector<int>(numberOfCrossings));
    for (int i = 0; i < numberOfCrossings; ++i) {
        for (int j = 0; j < numberOfCrossings; ++j) {
            cin >> crossingTime;
            timeMatrix[i][j] = stoi(crossingTime);
        }
    }
    cin >> threshold;

    distanceMatrix = floyd(timeMatrix);

    IloEnv         env;
    IloModel model(env);

    IloBoolVarArray stays = IloBoolVarArray(env, numberOfCrossings * numberOfCrossings);
    matrix<IloBoolVarArray> paths = matrix<IloBoolVarArray>(numberOfCrossings, vector<IloBoolVarArray>(numberOfCrossings));

    for (int i = 0; i < numberOfCrossings; ++i) {
        for (int j = 0; j < numberOfCrossings; ++j) {
            // maintain unidirectional streets (C1 in report)
            if (timeMatrix[i][j] != -1 && timeMatrix[j][i] == -1) {
                model.add(cell(stays, numberOfCrossings, i, j) == 1);
            }
            else if (timeMatrix[j][i] != -1 && timeMatrix[i][j] == -1) {
                model.add(cell(stays, numberOfCrossings, j, i) == 1);
            }

            if (i < j && timeMatrix[i][j] != -1 && timeMatrix[j][i] != -1) {
                // do not remove both sides (C3 in report)
                model.add(cell(stays, numberOfCrossings, i, j) + cell(stays, numberOfCrossings, j, i) >= 1);
            }

            // each pair has an array to determine its path
            if (i != j && distanceMatrix[i][j] != -1)
                paths[i][j] = IloBoolVarArray(env, numberOfCrossings * numberOfCrossings);
            // if crossing is disable in the original matrix it stays disabled (C2 in report)
            if (timeMatrix[i][j] == -1) model.add(cell(stays, numberOfCrossings, i, j) == 0);
        }
    }

    for (int s = 0; s < numberOfCrossings; ++s) {
        for (int t = 0; t < numberOfCrossings; ++t) {
            if (s != t && distanceMatrix[s][t] != -1) {
                add_flow_constraint(model, env, paths, numberOfCrossings, s, t);
                add_threshold_constraint(model, env, paths, stays, distanceMatrix, timeMatrix, threshold, numberOfCrossings, s, t);
            }
        }
    }

    // objective function: maximize the number of removed crossings (the more streets in 0 then the sum is going to be bigger)
    IloExpr expr(env);
    for (int i = 0; i < numberOfCrossings; ++i) {
        for (int j = i + 1; j < numberOfCrossings; ++j) {
            if (timeMatrix[i][j] != -1 && timeMatrix[j][i] != -1) {
                expr += 2 - cell(stays, numberOfCrossings, i, j) - cell(stays, numberOfCrossings, j, i);
            }
        }
    }
    IloObjective obj = IloMaximize(env, expr);
    model.add(obj);
    expr.end();

    IloCplex cplex(model);
    cplex.setOut(env.getNullStream());
    auto res = cplex.solve();
    assert(res == IloTrue);

    cout << numberOfCrossings << endl;
    for (int i = 0; i < numberOfCrossings; ++i) {
        for (int j = 0; j < numberOfCrossings; ++j) {
            cout << timeMatrix[i][j] << " ";
        }
        cout << endl;
    }
    cout << threshold << endl;

    for (int i = 0; i < numberOfCrossings; ++i) {
        for (int j = i + 1; j < numberOfCrossings; ++j) {
            if (timeMatrix[i][j] != -1 && timeMatrix[j][i] != -1) {
                bool keepIJ = cplex.getValue(cell(stays, numberOfCrossings, i, j)) > 0;
                bool keepJI = cplex.getValue(cell(stays, numberOfCrossings, j, i)) > 0;
                if (keepIJ && !keepJI) cout << i << " " << j << endl;
                else if (!keepIJ && keepJI) cout << j << " " << i << endl;
            }
        }
    }

    cout << cplex.getObjValue() << endl;
    env.end();
}
