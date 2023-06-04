/* poc.cpp A complete C to polyhedra to C compiler */

#include <clan/clan.h>
#include <cloog/cloog.h>
#include <osl/osl.h>

#include <cstdlib>
#include <iostream>
#include <vector>

#include "parser.h"
#include "utility.h"

using namespace std;

int split(osl_scop_p scop, std::vector<int> statementID, unsigned int depth);

int reorder(osl_scop_p scop, std::vector<int> loopID,
            std::vector<int> neworder);

int interchange(osl_scop_p scop, std::vector<int> loopID, unsigned int depth_1,
                unsigned int depth_2, int pretty);

int fuse(osl_scop_p scop, std::vector<int> loopID);

int skew(osl_scop_p scop, std::vector<int> loopID, unsigned int depth,
         unsigned int depth_other, int coeff);

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

void transformation(osl_scop_p scop) {
    vector<BaseArg*> args;
    int func = parser(scop, args);
    switch (func) {
        case SPLIT: {
            auto arg0 = (VectorArg*)args[0];
            auto arg1 = (SingleIntArg*)args[1];
            arg0->display();
            arg1->display();
            split(scop, arg0->arg, arg1->arg);
            break;
        }
        case REORDER: {
            auto arg0 = (VectorArg*)args[0];
            auto arg1 = (VectorArg*)args[1];
            arg0->display();
            arg1->display();
            reorder(scop, arg0->arg, arg1->arg);
            break;
        }
        case INTERCHANGE: {
            auto arg0 = (VectorArg*)args[0];
            auto arg1 = (SingleIntArg*)args[1];
            auto arg2 = (SingleIntArg*)args[2];
            auto arg3 = (SingleIntArg*)args[3];
            arg0->display();
            arg1->display();
            arg2->display();
            arg3->display();
            interchange(scop, arg0->arg, arg1->arg, arg2->arg, arg3->arg);
            break;
        }
        case FUSE: {
            auto arg0 = (VectorArg*)args[0];
            arg0->display();
            fuse(scop, arg0->arg);
            break;
        }
        case SKEW: {
            auto arg0 = (VectorArg*)args[0];
            auto arg1 = (SingleIntArg*)args[1];
            auto arg2 = (SingleIntArg*)args[2];
            auto arg3 = (SingleIntArg*)args[3];
            arg0->display();
            arg1->display();
            arg2->display();
            arg3->display();
            skew(scop, arg0->arg, arg1->arg, arg2->arg, arg3->arg);
            break;
        }
        default:
            cerr << "Unknown transformation: " << func << endl;
            break;
    }
    for (auto ptr : args) {
        if (ptr->type == 0)
            delete (SingleIntArg*)ptr;
        else
            delete (VectorArg*)ptr;
    }
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

// the loopID represents a unique loop (or a node in the AST)
// if the loopID is empty, then the whole scop is reordered
int reorder(osl_scop_p scop, std::vector<int> loopID,
            std::vector<int> neworder) {
    for (auto statement = scop->statement; statement != NULL;
         statement = statement->next) {
        auto id1 = get_statementID(statement->scattering);
        // check if the statement is in the loop
        if (in_loop(loopID, id1)) {
            // the position to be modified
            int index = loopID.size();
            // old order
            int old_order = id1[index];
            // new order
            int new_order = neworder[old_order];
            statement_id_modify(statement, index, new_order);
        }
    }
    return 0;
}

// just swap the depth_1 loop column and depth_2 loop column
int interchange(osl_scop_p scop, std::vector<int> loopID, unsigned int depth_1,
                unsigned int depth_2, int pretty) {
    if (depth_1 == depth_2) return 0;
    // get the index in scattering matrix
    int idx1 = depth_1 * 2 - 1;
    int idx2 = depth_2 * 2 - 1;
    for (auto statement = scop->statement; statement != NULL;
         statement = statement->next) {
        auto id1 = get_statementID(statement->scattering);
        if (in_loop(loopID, id1)) {
            // display_statement(statement);
            // swap the column idx1 and idx2
            for (int row = 0; row < statement->scattering->nb_rows; row++) {
                osl_int_swap(statement->scattering->precision,
                             &statement->scattering->m[row][idx1 + 1],
                             &statement->scattering->m[row][idx2 + 1]);
            }
        }
    }

    if (pretty) {
        osl_strings_p names = ((osl_scatnames_p)osl_generic_lookup(
                                   scop->extension, OSL_URI_SCATNAMES))
                                  ->names;
        swap(names->string[idx1], names->string[idx2]);
    }

    return 0;
}

// fuse means to merge two loops into one
int fuse(osl_scop_p scop, std::vector<int> loopID) {
    // check whether the loopID is valid
    if (!check_is_loop(scop, loopID)) return -1;

    vector<int> next_loop_id = get_next_loop(scop, loopID);
    // if there is no next loop to fuse
    if (next_loop_id.empty()) return -1;

    int fuse_val = loopID.back();
    auto max_id = find_max_in_loop(scop, loopID);
    int base_val = max_id[loopID.size()];

    for (auto statement = scop->statement; statement != NULL;
         statement = statement->next) {
        auto precision = statement->scattering->precision;
        auto statement_id = get_statementID(statement->scattering);
        if (in_loop(next_loop_id, statement_id)) {
            int constant_pos = statement->scattering->nb_columns - 1;
            // change the level
            statement_id_modify(statement, loopID.size() - 1, fuse_val);

            // cat the statement behind the max id
            statement_id_add(statement, loopID.size(), base_val + 1);
        }
    }

    return 0;
}

int skew(osl_scop_p scop, std::vector<int> loopID, unsigned int depth,
         unsigned int depth_other, int coeff) {
    int idx1 = depth * 2 - 1;
    int idx2 = depth_other * 2 - 1;

    for (auto statement = scop->statement; statement != NULL;
         statement = statement->next) {
        auto scattering = statement->scattering;
        auto precision = scattering->precision;
        auto statement_id = get_statementID(scattering);

        if (in_loop(loopID, statement_id)) {
            // every row should be transformed !!!
            for (int row = 0; row < scattering->nb_rows; ++row) {
                int t = osl_int_get_si(0, scattering->m[row][idx1 + 1]);
                t = t * coeff;
                osl_int_add_si(precision, &scattering->m[row][idx2 + 1],
                               scattering->m[row][idx2 + 1], t);
            }
        }
    }

    return 0;
}