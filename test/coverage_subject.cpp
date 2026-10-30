// Kept separate from test code so the report can select only production logic.
int coverage_subject(int value) {
    if (value > 0) return 10;
    return -10;
}

bool coverage_condition(bool a, bool b) {
    return a && b;
}
