/*
read scop from file and print to c code
*/

#include <bits/stdc++.h>
#include <cloog/cloog.h>
#include <osl/osl.h>

using namespace std;

void print_scop_to_c(FILE* output, osl_scop_p scop) {
    CloogState* state;
    CloogOptions* options;
    CloogInput* input;
    struct clast_stmt* clast;
    state = cloog_state_malloc();
    options = cloog_options_malloc(state);
    options->openscop = 1;
    cloog_options_copy_from_osl_scop(scop, options);
    input = cloog_input_from_osl_scop(options->state, scop);
    clast = cloog_clast_create_from_input(input, options);
    clast_pprint(output, clast, 0, options);
    cloog_clast_free(clast);
    options->scop = NULL;  // don’t free the scop
    cloog_options_free(options);
    cloog_state_free(state);  // the input is freed inside
}

int main(int argc, char* argv[]) {
    FILE *scop_src, *target;

    if (argc == 1)
        scop_src = fopen("scop.txt", "r");
    else if (argc == 2) {
        scop_src = fopen(argv[1], "r");
        target = fopen("target.c", "w");
    } else if (argc == 3) {
        scop_src = fopen(argv[1], "r");
        target = fopen(argv[2], "w");
    } else {
        cout << "Usage: ./codegen <scop_src> <target>" << endl;
        return 1;
    }

    osl_scop_p scop = osl_scop_read(scop_src);

    print_scop_to_c(target, scop);

    return 0;
}