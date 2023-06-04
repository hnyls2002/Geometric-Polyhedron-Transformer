/* poc.cpp A complete C to polyhedra to C compiler */

#include <clan/clan.h>
#include <cloog/cloog.h>
#include <osl/osl.h>

#include <cstdlib>
#include <iostream>
#include <vector>

#include "utility.h"

using namespace std;

int split(osl_scop_p scop, std::vector<int> statementID, unsigned int depth);

/**
 * reorder function:
 * Reorders the statements in the loop
 * scop: the SCoP to be transformed
 * statementID: the statement scattering ID on AST
 * neworder: the new order of the statements
 * return status
 */
// int reorder(osl_scop_p scop, std::vector<int> statementID, std::vector<int>
// neworder) ;

/**
 * interchange function:
 * On each statement which belongs to the node, the loops that match the
 * depth_1-th and the depth_2 are interchanged
 * given the inner loop
 * scop: the SCoP to be transformed
 * statementID: the statement scattering ID on AST
 * depth_1, depth_2: >= 1
 * pretty: 1 or 0 : whether update the scatnames
 * return status
 */
// int interchange(osl_scop_p scop, std::vector<int> statementID,
//                 unsigned int depth_1, unsigned int depth_2, int pretty);

/**
 * fuse function:
 * Fuse loop with the first loop after
 * scop: the SCoP to be transformed
 * statementID: the statement scattering ID on AST
 * return status
 */
// int fuse(osl_scop_p scop, std::vector<int> statementID);

/**
 * skew function
 * Transform the iteration domain so that the loop at depth depends on the
 * loop iterator at depth_other: in all occurrences, the loop iterator i
 * of the former loop is replaced by (i + coeff*j) where j is the loop iterator
 * of the latter loop.  Adjusts the loop boundaries accordingly.
 * Skewing the loop by its own iterator, i.e. depth == depth_outer, is invalid
 * scop: the SCoP to be transformed
 * statementID: the statement scattering ID on AST
 * depth: 1-based depth of the output loop to modify
 * depth_other: 1-based depth of the loop iterator to add
 * coeff: the coefficient to multiply the dimension by
 * return status
 */
// int skew(osl_scop_p scop, std::vector<int> statementID, unsigned int depth,
//          unsigned int depth_other, int coeff);

/**
 * tile function:
 * Do tiling on the loop at depth with size, the outer loop is at depth_outer
 * scop: the SCoP to be transformed
 * statementID: the statement scattering ID on AST
 * depth: tiling on loop at depth
 * depth_outer: outer loop depth
 * size: tiling size
 * return status
 */
// int tile(osl_scop_p scop, std::vector<int> statementID, unsigned int depth,
//          unsigned int depth_outer, unsigned int size);

// BONUS
/** unroll function
 * Unroll a loop
 * scop: the SCoP to be transformed
 * statementID: the statement scattering ID on AST
 * factor: unroll factor
 * return status
 */
// int unroll(osl_scop_p scop, std::vector<int> statementID, unsigned int
// factor);

// int reorder(osl_scop_p scop, std::vector<int> statementID, std::vector<int>
// neworder) ; int interchange(osl_scop_p scop,
//                 std::vector<int> statementID,
//                 unsigned int depth_1, unsigned int depth_2,
//                 int pretty) ;
// int fuse(osl_scop_p scop, std::vector<int> statementID) ;
// int skew(osl_scop_p scop,
//          std::vector<int> statementID,
//          unsigned int depth,
//          unsigned int depth_other,
//          int coeff) ;
// int tile(osl_scop_p scop,
//          std::vector<int> statementID, unsigned int depth, unsigned int
//          depth_outer, unsigned int size) ;
// int unroll(osl_scop_p scop, std::vector<int> statementID, unsigned int
// factor) ;

/* Use the Clan library to convert a SCoP from C to OpenScop */
osl_scop_p read_scop_from_c(FILE* input, char* input_name) {
    clan_options_p clanoptions;
    osl_scop_p scop;
    clanoptions = clan_options_malloc();
    clanoptions->precision = OSL_PRECISION_MP;
    CLAN_strdup(clanoptions->name, input_name);
    scop = clan_scop_extract(input, clanoptions);
    clan_options_free(clanoptions);
    return scop;
}

/* Use the CLooG library to output a SCoP from OpenScop to C */
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

string get_trans(osl_generic_p extension) {
    string arg;
    for (auto p = extension; p != NULL; p = p->next) {
        if (strcmp(p->interface->URI, "clay") == 0) {
            arg = ((char**)p->data)[0];
        }
    }
    return arg;
}

void transformation(osl_scop_p scop) {
    string arg = get_trans(scop->extension);
    // some preprocessing
    arg.erase(0, arg.find_first_not_of(" "));
    arg.erase(arg.find(';'), arg.length());

    if (arg == "split([0,1], 1)") {  // split loop
        std::vector<int> statementID;
        statementID.push_back(0);
        statementID.push_back(1);
        split(scop, statementID, 1);
    } else if (arg == "split([0,0], 1") {  // split nothing before
        std::vector<int> statementID;
        statementID.push_back(0);
        statementID.push_back(0);
        split(scop, statementID, 1);
    } else if (arg == "split([0,2], 1)") {  // split statement 1
        std::vector<int> statementID;
        statementID.push_back(0);
        statementID.push_back(2);
        split(scop, statementID, 1);
    } else if (arg == "split([0,1,1], 1)") {  // split statement 2
        std::vector<int> statementID;
        statementID.push_back(0);
        statementID.push_back(1);
        statementID.push_back(1);
        split(scop, statementID, 1);
    } else if (arg == "split([0,1,1], 2)") {  // split statement 3
        std::vector<int> statementID;
        statementID.push_back(0);
        statementID.push_back(1);
        statementID.push_back(1);
        split(scop, statementID, 2);
    }

    // TODO : Do the loop transformations on SCoP here
}

int main(int argc, char* argv[]) {
    osl_scop_p scop;
    FILE* input;
    if ((argc < 2) || (argc > 2)) {
        fprintf(stderr, "usage: %s file.c\n", argv[0]);
        exit(0);
    }
    if (argc == 1)
        input = stdin;
    else
        input = fopen(argv[1], "r");
    if (input == NULL) {
        fprintf(stderr, "cannot open input file\n");
        exit(0);
    }
    scop = read_scop_from_c(input, argv[1]);

    // original scop
    FILE* original = fopen("./testspace/original.scop", "w");
    osl_scop_print(original, scop);

    // do the transformations
    transformation(scop);

    // transformed scop
    FILE* transformed = fopen("./testspace/transformed.scop", "w");
    osl_scop_print(transformed, scop);

    // transformed scop to c
    FILE* output = fopen("./testspace/output.c", "w");
    print_scop_to_c(output, scop);

    osl_scop_free(scop);
    fclose(input);
    return 0;
}

int split(osl_scop_p scop, std::vector<int> statementID, unsigned int depth) {
    statement_shift(scop->statement, statementID, depth);
    return 0;
}
