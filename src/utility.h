#ifndef UTILITY_H
#define UTILITY_H

#include <clan/clan.h>
#include <cloog/cloog.h>
#include <osl/osl.h>

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

// display the statement for debug
void display_statement(osl_statement_p statement) {
    cerr << statement->extension->interface->sprint(statement->extension->data)
         << endl;
}

// find the k-th element's row
// row in the relation matrix may not be ordered
// but column is ordered naturally
int find_row(osl_relation_p realation, int k) {
    for (int i = 0; i < realation->nb_rows; ++i) {
        // start with e/i : so k + 1
        if (!osl_int_zero(realation->precision, realation->m[i][k + 1])) {
            return i;
        }
    }
    return -1;
}

// extract the statementID from the scattering
vector<int> get_statementID(osl_relation_p scattering) {
    vector<int> statementID;

    // get the last element of the row : multiplication for constant 1
    int constant_pos = scattering->nb_columns - 1;

    // only 0, 2, 4, ... are the statementID
    for (int i = 0; i < scattering->nb_output_dims; i += 2) {
        int row = find_row(scattering, i);
        int value = osl_int_get_si(scattering->precision,
                                   scattering->m[row][constant_pos]);
        statementID.push_back(value);
    }

    return statementID;
}

// find the different position of two vectors and return the difference
int id_compare(vector<int> id0, vector<int> id1, int& d_pos) {
    // do not worry about the lengths not match : must have difference
    int len = min(id0.size(), id1.size());
    for (int i = 0; i < len; ++i) {
        if (id0[i] != id1[i]) {
            d_pos = i;
            return id0[i] - id1[i];
        }
    }
    d_pos = len;
    return 0;
}

// shift all the statements after the statementID at depth
void statement_shift(osl_statement_p statement, vector<int> id0,
                     unsigned int depth) {
    int col = (depth - 1) * 2;
    for (; statement != NULL; statement = statement->next) {
        auto id = get_statementID(statement->scattering);
        // display_statement(statement);
        // for (auto x : id) {
        //     cerr << x << " ";
        // }
        // cerr << endl;
        int d_pos, diff = id_compare(id0, id, d_pos);
        if (diff < 0 && d_pos + 1 >= depth &&
            col < statement->scattering->nb_output_dims) {
            int row = find_row(statement->scattering, col);
            int constant_pos = statement->scattering->nb_columns - 1;
            osl_int_increment(statement->scattering->precision,
                              &statement->scattering->m[row][constant_pos],
                              statement->scattering->m[row][constant_pos]);
        }
    }
}

// change the k-th id of the statement to x
// the output scattering index should be 2 * k
void statement_id_modify(osl_statement_p statement, int k, int x) {
    int row = find_row(statement->scattering, 2 * k);
    int constant_pos = statement->scattering->nb_columns - 1;
    // display_statement(statement);
    // cerr << "row: " << row << endl;
    // cerr << "constant_pos: " << constant_pos << endl;
    // cerr << "x: " << x << endl;
    osl_int_set_si(statement->scattering->precision,
                   &statement->scattering->m[row][constant_pos], x);
}

// add x to the k-th id of the statement
void statement_id_add(osl_statement_p statement, int k, int x) {
    int row = find_row(statement->scattering, 2 * k);
    int constant_pos = statement->scattering->nb_columns - 1;
    osl_int_add_si(statement->scattering->precision,
                   &statement->scattering->m[row][constant_pos],
                   statement->scattering->m[row][constant_pos], x);
}

bool in_loop(vector<int> loop_id, vector<int> statement_id) {
    int d_pos;
    id_compare(loop_id, statement_id, d_pos);
    return d_pos == loop_id.size();
}

// check if the given id is a loop
// the given loop id length should be shorter
bool check_is_loop(osl_scop_p scop, vector<int> loop_id) {
    for (auto statement = scop->statement; statement != NULL;
         statement = statement->next) {
        auto id = get_statementID(statement->scattering);
        if (in_loop(loop_id, id)) {
            // display_statement(statement);
            return loop_id.size() * 2 - 1 <
                   statement->scattering->nb_output_dims;
        }
    }
    return false;
}

// find the next loop's id (next to the given loop_id, and the same level)
vector<int> get_next_loop(osl_scop_p scop, vector<int> loop_id) {
    vector<int> next_loop_id = loop_id;
    next_loop_id[loop_id.size() - 1] += 1;
    if (check_is_loop(scop, next_loop_id)) {
        return next_loop_id;
    }
    return vector<int>();
}

// find the max id matching the loop-id prefix
vector<int> find_max_in_loop(osl_scop_p scop, vector<int> loop_id) {
    vector<int> max_id;
    for (auto statement = scop->statement; statement != NULL;
         statement = statement->next) {
        auto statement_id = get_statementID(statement->scattering);
        if (in_loop(loop_id, statement_id)) {
            if (max_id.empty()) {
                max_id = statement_id;
            } else {
                int d_pos;
                if (id_compare(statement_id, max_id, d_pos) > 0 &&
                    d_pos == loop_id.size()) {
                    max_id = statement_id;
                }
            }
        }
    }
    if (max_id.empty()) {
        cerr << "not a loop in find_max_in_loop()" << endl;
        exit(-1);
    }
    return max_id;
}

bool check_name_exist(osl_scatnames_p scat, char* str) {
    char** ptr = scat->names->string;
    while (*ptr != NULL) {
        if (strcmp(*ptr, str) == 0) {
            return true;
        }
        ++ptr;
    }
    return false;
}

int stripmine(osl_scop_p scop, vector<int> loop_id, unsigned int depth,
              unsigned int size) {
    int col = (depth - 1) * 2;
    for (auto statement = scop->statement; statement != NULL;
         statement = statement->next) {
        auto scattering = statement->scattering;
        auto statement_id = get_statementID(scattering);
        auto precision = scattering->precision;
        if (in_loop(loop_id, statement_id)) {
            int row = find_row(scattering, col);

            // insert new lines and columns
            // cerr << "col : " << col << endl;
            osl_relation_insert_blank_column(scattering, col + 1);
            osl_relation_insert_blank_column(scattering, col + 1);

            osl_relation_insert_blank_row(scattering, row);
            osl_relation_insert_blank_row(scattering, row);
            osl_relation_insert_blank_row(scattering, row);

            osl_int_set_si(precision, &scattering->m[row + 0][col + 1], -1);
            osl_int_set_si(precision, &scattering->m[row + 1][col + 2], -size);
            osl_int_set_si(precision, &scattering->m[row + 2][col + 2], size);
            osl_int_set_si(precision,
                           &scattering->m[row + 2][scattering->nb_columns - 1],
                           size - 1);

            // inquality
            osl_int_set_si(precision, &scattering->m[row + 1][0], 1);
            osl_int_set_si(precision, &scattering->m[row + 2][0], 1);

            // dependences
            osl_int_set_si(precision, &scattering->m[row + 1][col + 4], 1);
            osl_int_set_si(precision, &scattering->m[row + 2][col + 4], -1);

            scattering->nb_output_dims += 2;

            // reorder
            int row_next = find_row(scattering, col + 2);
            osl_int_assign(precision,
                           &scattering->m[row][scattering->nb_columns - 1],
                           scattering->m[row_next][scattering->nb_columns - 1]);
            osl_int_set_si(precision,
                           &scattering->m[row_next][scattering->nb_columns - 1],
                           0);
        }
    }

    osl_scatnames_p scat =
        (osl_scatnames_p)osl_generic_lookup(scop->extension, OSL_URI_SCATNAMES);
    osl_strings_p names = scat->names;

    char buffer[128];
    char* new_var_iter;
    char* new_var_scat;

    for (int i = 0;; ++i) {
        sprintf(buffer, "__%s%s%d", names->string[col + 1],
                names->string[col + 1], i);
        if (check_name_exist(scat, buffer)) {
            continue;
        }
        break;
    }
    OSL_strdup(new_var_iter, buffer);

    for (int i = 0;; ++i) {
        sprintf(buffer, "__b%d", i);
        if (check_name_exist(scat, buffer)) {
            continue;
        }
        break;
    }
    OSL_strdup(new_var_scat, buffer);

    // cerr << "new_var_iter: " << new_var_iter << endl;
    // cerr << "new_var_scat: " << new_var_scat << endl;

    // insert two names
    int nb_strings = osl_strings_size(names) + 2;
    osl_strings_p new_names = osl_strings_malloc();
    new_names->string = (char**)malloc(sizeof(char**) * (nb_strings + 1));

    for (int i = 0; i < col; ++i) new_names->string[i] = names->string[i];

    new_names->string[col + 0] = new_var_scat;
    new_names->string[col + 1] = new_var_iter;

    for (int i = col + 2; i < nb_strings; ++i)
        new_names->string[i] = names->string[i - 2];

    new_names->string[nb_strings] = NULL;

    free(names->string);
    free(names);
    scat->names = new_names;

    return 0;
}

#endif